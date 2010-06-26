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

Socket::Socket():
	fd(socket(DOMAIN, TYPE, PROTOCOL)), state(IDLE), addr(), lerr(NULL)
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

Socket::Socket(const Socket& s):
	fd(s.fd), state(s.state), addr(), lerr(s.lerr)
{
	memset(&addr, '\0', sizeof addr);
}

Socket::Socket(int sock, struct sockaddr_in *ad, enum State ste):
	fd(sock), state(ste), addr(), lerr(NULL)
{
	memcpy(&addr, ad, sizeof addr);
}

void Socket::reinit(int sock, struct sockaddr_in *ad, enum State ste)
{
	if(state == CONNECTED && shutdown(fd, SHUT_RDWR) == -1)
		throw ERR_SHUTDOWN;

	if(close(fd) == -1)
		throw ERR_CLOSE;


	fd = sock;
	if(ad)
		memcpy(&addr, ad, sizeof addr);
	else
		memset(&addr, '\0', sizeof addr);

	state = ste;
	lerr = NULL;
}

Socket& Socket::operator=(const Socket& s)
{
	fd = s.fd;
	state = s.state;
	lerr = s.lerr;
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
	int newfd = socket(DOMAIN, TYPE, PROTOCOL);
	if(newfd == -1 || !setblocking(false))
		throw ERR_INIT;

	reinit(newfd, NULL, IDLE);
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

bool Socket::senddata(char c)
{
	return senddata((const void *)&c, (size_t)sizeof(char));
}

bool Socket::senddata(const char *data)
{
	return senddata((const void *)data, strlen(data));
}

Socket& Socket::operator<<(char c)
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

bool Socket::recvdata(std::string& data)
{
#define BUFSIZE 512
	char buf[BUFSIZE];
	bool win;

	if((win = recvdata(buf, BUFSIZE)))
		data = buf;

	return win;
#undef BUFSIZE
}

bool Socket::recvdata(char *buf, int len)
{
	int ret;

	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = recv(fd, buf, len, MSG_NOSIGNAL /* prevent SIGPIPE etc */);
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

enum Socket::State Socket::getstate()
{
	if(state == CONNECTING){
		// check if we're connected yet
		if(::connect(fd, (sockaddr *) &addr, sizeof addr) == -1){
			if(errno != EINPROGRESS && errno != EALREADY){
				if(errno)
					lerr = strerror(errno);
				else
					lerr = "timed out";
				state = IDLE;
			}
#if DEBUG
			else
				std::cerr << "socket::getstate(): errno: " << errno << std::endl;
#endif
		}else
			state = CONNECTED;
	}
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

Socket *Socket::accept()
{
	int newfd;
	struct sockaddr_in ad;
	socklen_t siz = sizeof ad;

	if(state != LISTENING)
		throw ERR_NOT_LISTENING;

	newfd = ::accept(fd, (struct sockaddr *) &ad, &siz);

	if(newfd == -1){
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			lerr = ERR_COULDNT_ACCEPT;
			return NULL;
		}
		lerr = NULL;
		return NULL;
	}else{
		return new Socket(newfd, &ad);
	}
}

bool Socket::accept(Socket& s)
{
	int newfd;
	struct sockaddr_in ad;
	socklen_t siz = sizeof ad;

	if(state != LISTENING)
		throw ERR_NOT_LISTENING;

	newfd = ::accept(fd, (struct sockaddr *) &ad, &siz);

	if(newfd == -1){
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			lerr = ERR_COULDNT_ACCEPT;
			return false;
		}
		lerr = NULL;
		return false;
	}else{
		s.reinit(newfd, &ad);
		return true;
	}
}
