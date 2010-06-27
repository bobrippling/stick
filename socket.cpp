#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "socket.h"

#define DOMAIN AF_INET /* AF_INET6 */
#define TYPE SOCK_STREAM
#define PROTOCOL 0
#define LISTEN_BACKLOG 10
#define USE_IPv6 0

#define ERR_PREFIX          errno ? strerror(errno) :
#define ERR_INIT            ERR_PREFIX "Couldn't create socket/set non blocking"
#define ERR_CLOSE           ERR_PREFIX "Couldn't close socket"
#define ERR_SHUTDOWN        ERR_PREFIX "Couldn't shutdown socket"

#define ERR_NTOP            ERR_PREFIX "Couldn't convert IP socket address"
#define ERR_SEND            ERR_PREFIX "Couldn't send data"

#define ERR_NOT_CONNECTED   ERR_PREFIX "Not connected"
#define ERR_NOT_IDLE        ERR_PREFIX "Not in idle state"

#define ERR_TIMED_OUT       ERR_PREFIX "Timed out"
#define ERR_NOT_LISTENING   ERR_PREFIX "Not listening"

#define ERR_COULDNT_BIND    ERR_PREFIX "Couldn't bind port"
#define ERR_COULDNT_ACCEPT  ERR_PREFIX "Couldn't accept connection"

#define TEST_IDLE() do{ \
		if(state != IDLE){ \
			lerr = ERR_NOT_IDLE; \
			return false; \
		} \
	}while(0)

#define DEBUG 1

#if DEBUG
#include <iostream>
#endif

Socket::Socket(
				Socket& (connreq)(),
				void (*disconnected)(),
				void (*received)(),
				void (*error)()
		):
	fd(socket(DOMAIN, TYPE, PROTOCOL)), addr(),
	lerr(NULL), connrequestf(connreq), disconnectedf(disconnected),
	receivedf(received), errorf(error), state(IDLE)
{
	if(fd == -1 || !setblocking(false))
		throw ERR_INIT;

#if defined(SO_NOSIGPIPE)
	{
		int val = 1;
		// ignore sigpipe - FreeBSD
		setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
	}
#endif

	memset(&addr, '\0', sizeof addr);
}


Socket& Socket::operator=(const Socket& s)
{
	// FIXME
	std::cerr << s.fd;
	return *this;
}

bool Socket::setblocking(bool block) const
{
	int flags;

	if((flags = fcntl(fd, F_GETFL, 0)) == -1)
		flags = 0;

	if(block)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	return fcntl(fd, F_SETFL, flags) != -1;
}


Socket::~Socket()
{
	cleanup();
	shutdown(fd, SHUT_RDWR);
	close(fd);
	fd = -1;
}

const char *Socket::remoteaddr()
{
	// FIXME
	static char ip[16];
	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

	if(!inet_ntop(AF_INET, &addr, ip, 16)){
		lerr = ERR_NTOP;
		return NULL;
	}
	return ip;
}

// data i/o -----------------------------------------

void Socket::cleanup()
{
	newsocket(socket(DOMAIN, TYPE, PROTOCOL));
}

void Socket::newsocket(int newfd)
{
	if(newfd == -1 || !setblocking(false))
		throw ERR_INIT;

	if(state == CONNECTED)
		shutdown(fd, SHUT_RDWR);
	close(fd);
	fd = newfd;
}

void Socket::disconnect()
{
	if(state != IDLE)
		cleanup();
	lerr = NULL;
}

bool Socket::senddata(const void *p, size_t siz)
{
	int ret;

	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

	//win = sendto(fd, p, siz, 0, NULL, 0) != -1;
	//                                addr, addrlen

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = send(fd, p, siz, MSG_NOSIGNAL /* prevent SIGPIPE etc */);
#elif defined(SO_NOSIGPIPE)
	ret = send(fd, p, siz, 0);
#else
	// TODO
	// not bsd nor linux
	// neither, nor:
	// http://krokisplace.blogspot.com/2010/02/suppressing-sigpipe-in-library.html
#error unknown OS
#endif

	if(ret == -1 || errno == EPIPE){
		if(errno == ECONNRESET)
			// disconnected
			lerr = ERR_NOT_CONNECTED;
		else
			lerr = ERR_SEND;

		cleanup();
		return false;
	}

	return true;
}

bool Socket::senddata(const std::string& data)
{
	return senddata(data.c_str());
}

bool Socket::senddata(const char c)
{
	return senddata(&c, (size_t)sizeof(char));
}

bool Socket::senddata(const char *data)
{
	return senddata((const void *)data, strlen(data));
}

Socket& Socket::operator<<(const char c)
{
	senddata(c);
	return *this;
}

Socket& Socket::operator<<(const char *s)
{
	senddata(s);
	return *this;
}

Socket& Socket::operator<<(const std::string& s)
{
	senddata(s);
	return *this;
}

bool Socket::recvdata(void *buf, size_t len)
{
	int ret;

	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = recv(fd, buf, len, MSG_NOSIGNAL /* prevent SIGPIPE */);
#elif defined(SO_NOSIGPIPE)
	ret = recv(fd, buf, len, 0);
#else
	// not bsd nor linux
#error unknown OS
#endif
	/*
	 * funcs:
	 * read
	 * recv
	 * recvmsg <- udp
	 */

	if(ret == 0)
		// disconnected
		cleanup();

	return ret > 0;
}

// ------------------------------------------------

enum Socket::State Socket::getstate() const
{
	return state;
}

const char *Socket::lasterr() const
{
	return lerr;
}

// socket instructions -----------------------------

bool Socket::connect(const char *host, int port)
{
	TEST_IDLE();

	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);

#if USE_IPv6
	if(!inet_pton(AF_INET, host, &addr.sin_addr))
		return false;
#else
	if(!inet_aton(host, &addr.sin_addr)){
		lerr = "couldn't lookup host";
		return false;
	}
#endif

	if(::connect(fd, (sockaddr *) &addr, sizeof addr) == -1)
		if(errno == EINPROGRESS){
			state = CONNECTING;
			return true;
		}
	lerr = strerror(errno);
	return false;
}

bool Socket::connect(const std::string& h, int port)
{
	return connect(h.c_str(), port);
}

bool Socket::listen(int port)
{
	TEST_IDLE();

	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);

	if(bind(fd, (struct sockaddr *) &addr, sizeof addr) == -1){
		lerr = ERR_COULDNT_BIND;
		return false;
	}

	if(::listen(fd, LISTEN_BACKLOG) == -1){
		lerr = strerror(errno);
		return false;
	}else{
		state = LISTENING;
		lerr = NULL;
		return true;
	}
}

bool Socket::runevents()
{
	/*
	 * if connected:
	 *   receive data/check disconnect
	 * elsif listening:
	 *   check for connections
	 * elsif connecting:
	 *   check connected
	 * elsif idle
	 *   break;
	 */

	switch(state){
		case LISTENING:
			return accept();

		case CONNECTED:
			// TODO;
			break;

		case CONNECTING:
			return checkconnected();

		case IDLE:
			break;
	}
	return true;
}

bool Socket::checkconnected()
{
	// check if we're connected yet
	if(::connect(fd, (sockaddr *) &addr, sizeof addr) == -1){
		if(errno == EINPROGRESS && errno == EALREADY)
			return true;

		if(errno)
			lerr = strerror(errno);
		else
			lerr = ERR_TIMED_OUT;

		state = IDLE;

		return false;
	}else{
		state = CONNECTED;
		return true;
	}
}

bool Socket::accept()
{
	int newfd;
	struct sockaddr_in ad;
	socklen_t siz = sizeof ad;

	newfd = ::accept(fd, (struct sockaddr *) &ad, &siz);

	if(newfd == -1){
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			lerr = ERR_COULDNT_ACCEPT;
		else
			lerr = NULL;

	}else if(connrequestf){
		Socket s(connrequestf());

		s.newsocket(newfd);
		memcpy(&s.addr, &ad, sizeof ad);
		return true;
	}
	return false;
}
