#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

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
#define ERR_SELECT          ERR_PREFIX "Socket poll error"

#define ERR_SEND            ERR_PREFIX "Couldn't send data"
#define ERR_RECV            ERR_PREFIX "Socket receive error"

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

/*
 * http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_15.html
 */

Socket::Socket(int bs,
				Socket& (connreq)(),
				void (*disconnected)(),
				void (*received)(void *, size_t),
				void (*error)()
		):
	fd(socket(DOMAIN, TYPE, PROTOCOL)), addr(),
	lerr(NULL), buffer(new char[bs]), buffersize(bs),
	connrequestf(connreq), disconnectedf(disconnected),
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


Socket::Socket(const Socket& s):
	fd(), addr(), lerr(), buffer(NULL), buffersize(-1),
	connrequestf(), disconnectedf(), receivedf(),
	errorf(), state()
{
	*this = s;
}

Socket& Socket::operator=(const Socket& s)
{
	fd = s.fd;

	memcpy(&addr, &s.addr, sizeof addr);

	lerr = s.lerr;

	buffer = new char[buffersize = s.buffersize];

	connrequestf = s.connrequestf;
	disconnectedf = s.disconnectedf;
	receivedf = s.receivedf;
	errorf = s.errorf;

	state = s.state;

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
	delete[] buffer;
}

const char *Socket::remoteaddr()
{
	static char ip[16];

	// FIXME

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
#if DEBUG
	else
		std::cerr << "sent \"" << (const char *)p << "\"\n";
#endif

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

// ------------------------------------------------

const char *Socket::getstatestr() const
{
	switch(state){
		case CONNECTED:  return "connected";
		case LISTENING:  return "listening";
		case CONNECTING: return "connecting";
		case IDLE:       return "idle";
	}
	return NULL;
}

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
	 *
	 * returns true on state change
	 */

	switch(state){
		case CONNECTED:
			return checkconn();

		case LISTENING:
			return accept();

		case CONNECTING:
			return connectedyet();

		case IDLE:
			return false;
	}
	return true;
}

bool Socket::connectedyet()
{
	struct pollfd fds;
	// check if we're connected yet

	// FIXME FIXME FIXME
	fds.fd = fd;
	fds.events = POLLOUT;

	switch(poll(&fds, 1, 1 /*1ms*/) <= 0){
		case -1:
			if(errno)
				lerr = strerror(errno);
			else
				lerr = ERR_TIMED_OUT;
#if DEBUG
			std::cerr << "Socket::connectedyet(): connect() error: "
				<< lerr << " (" << errno << ")\n";
#endif
			cleanup();
			state = IDLE;
			return false;

		case 0:
			// not connected
			return false;

		default:
			if(errno){
				std::cerr << "Socket::connectedyet(): errno: " << errno
									<< ": " << strerror(errno) << std::endl;
				return false;
			}

			if((fds.revents & POLLOUT) == POLLOUT){
#if DEBUG
				std::cerr << "Socket::connectedyet(): connected\n";
#endif
				state = CONNECTED;
				return true;
			}
	}

	return false;
}

bool Socket::accept()
{
	int newfd;
	struct sockaddr_in ad;
	socklen_t siz = sizeof ad;

	if(!connrequestf)
		return false;

	newfd = ::accept(fd, (struct sockaddr *) &ad, &siz);

	if(newfd == -1){
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			lerr = ERR_COULDNT_ACCEPT;
		else
			lerr = NULL;

	}else /*if(connrequestf)*/ {
		Socket& s(connrequestf());

		s.newsocket(newfd);

		s.state = CONNECTED;
		memcpy(&s.addr, &ad, sizeof ad);

		return true;
	}
	return false;
}

bool Socket::checkconn()
{
	struct timeval tv = { 0, 50000 };
	fd_set fds;
	int ret;

	if(state != CONNECTED)
		throw ERR_NOT_CONNECTED;

	FD_SET(fd, &fds);

	// use poll()?
	switch(select(1, &fds, NULL, NULL, &tv)){
		case -1:
			lerr = ERR_SELECT;
			return false;

		case 0:
			// timeout
			return false;
	}

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = recv(fd, buffer, buffersize, MSG_NOSIGNAL /* prevent SIGPIPE */);
#elif defined(SO_NOSIGPIPE)
	ret = recv(fd, buffer, buffersize, 0);
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

	switch(recv(fd, buffer, buffersize, 0)){
		case -1:
			if(errno == EWOULDBLOCK || errno == EAGAIN)
				return false;
			cleanup();
			state = IDLE;
			lerr = ERR_RECV;
			return true;

		case 0:
			// disco
			cleanup();
			state = IDLE;
			lerr = NULL;
			return true;
	}

#if DEBUG
	std::cerr << "got data: \"" << buffer << "\"\n";
#endif

	// got data
	receivedf(buffer, buffersize);
	return true;
}
