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
#define ERR_NOT_CONNECTED   ERR_PREFIX "Not connected"
#define ERR_NOT_IDLE        ERR_PREFIX "Not in idle state"
#define ERR_NOT_LISTENING   ERR_PREFIX "Not listening"
#define ERR_COULDNT_BIND    ERR_PREFIX "Couldn't bind port"
#define ERR_COULDNT_ACCEPT  ERR_PREFIX "Couldn't accept connection"
#define ERR_NTOP            ERR_PREFIX "Couldn't convert IP socket address"
#define ERR_SEND            ERR_PREFIX "Couldn't send data"

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

Socket::Socket(int sock, struct sockaddr_in *ad):
	fd(sock), state(CONNECTED), addr(), lerr(NULL)
{
	memcpy(&addr, ad, sizeof addr);
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
	if(state != IDLE)
		cleanup();
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
	shutdown(fd, SHUT_RDWR);
	fd = -1;
	state = IDLE;

	lerr = NULL;
}

void Socket::disconnect()
{
	if(state != IDLE)
		cleanup();
	lerr = NULL;
}

inline bool Socket::senddata(void *p, size_t siz)
{
	bool win;

	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

	//win = sendto(fd, p, siz, 0, NULL, 0) != -1;
	//                                addr, addrlen

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	win = send(fd, p, siz, MSG_NOSIGNAL /* prevent SIGPIPE etc */);
#elif defined(SO_NOSIGPIPE)
	win = send(fd, p, siz, 0);
#else
	// not bsd nor linux
	// TODO
	// *bracing for Windows BS*
#error crabs

	// neither, nor windows:
	// http://krokisplace.blogspot.com/2010/02/suppressing-sigpipe-in-library.html
#endif

	if(!win || errno == EPIPE){
		lerr = ERR_SEND;
		state = IDLE;
		win = false; // in case errno && win
	}

	return win;
}

bool Socket::senddata(std::string& data)
{
	return senddata(data.c_str());
}

bool Socket::senddata(char c)
{
	return senddata(&c, sizeof(char));
}

bool Socket::senddata(const char *data)
{
	return senddata((void *)data, strlen(data));
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

Socket& Socket::operator<<(std::string& s)
{
	senddata(s);
	return *this;
}

bool Socket::recvdata(std::string& data) const
{
#define BUFSIZ 512
	char buf[BUFSIZ];
	bool win;

	if((win = recvdata(buf, BUFSIZ)))
		data = buf;

	return win;
#undef BUFSIZ
}

bool Socket::recvdata(char *buf, int len) const
{
	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

	// FIXME: use recv
	// check for EWOULDBLOCK/once connected remove O_BLOCK and use select() or poll()
	return recvfrom(fd, buf, len, 0, NULL, 0) != -1;
	/*
	 * read
	 * recv
	 * recvmsg <- udp
	 */
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
