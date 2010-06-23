#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "socket.h"

#define DOMAIN AF_INET /* AF_INET6 */
#define TYPE SOCK_STREAM
#define PROTOCOL 0
#define LISTEN_BACKLOG 10
#define USE_IPv6 0

#define DEBUG 1

#if DEBUG
#include <iostream>
#endif

Socket::Socket():
	fd(socket(DOMAIN, TYPE, PROTOCOL)), state(IDLE), hostaddr(), lerr(NULL)
{
	if(fd == -1 || !setblocking(false))
		throw;

	memset(&hostaddr, '\0', sizeof hostaddr);
}

Socket::Socket(const Socket& s):
	fd(s.fd), state(s.state), hostaddr(), lerr(s.lerr)
{
	memset(&hostaddr, '\0', sizeof hostaddr);
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

// data i/o -----------------------------------------

bool Socket::senddata(std::string& data) const
{
	return senddata(data.c_str());
}

bool Socket::senddata(char c) const
{
	if(state != CONNECTED)
		throw;

	return sendto(fd, &c, sizeof(char), 0, NULL, 0) != -1;
	//                                addr, addrlen
}

bool Socket::senddata(const char *data) const
{
	if(state != CONNECTED)
		throw;

	return sendto(fd, data, strlen(data), 0, NULL, 0) != -1;
	//                                addr, addrlen
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
		throw;

	return recvfrom(fd, buf, len, 0, NULL, 0) != -1;
}

enum Socket::State Socket::getstate()
{
	if(state == CONNECTING){
		// check if we're connected yet
		if(::connect(fd, (sockaddr *) &hostaddr, sizeof hostaddr) == -1){
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
	if(state != IDLE)
		return false;

	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port   = htons(port);

#if USE_IPv6
	if(!inet_pton(AF_INET, host, &hostaddr.sin_addr))
		return false;
#else
	if(!inet_aton(host, &hostaddr.sin_addr)){
		lerr = "couldn't lookup host";
		return false;
	}
#endif

	if(::connect(fd, (sockaddr *) &hostaddr, sizeof hostaddr) == -1)
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

void Socket::cleanup()
{
	shutdown(fd, SHUT_RDWR);
	fd = -1;
	state = IDLE;

	lerr = NULL;
}

bool Socket::listen()
{
	if(state != IDLE)
		return false;

	if(::listen(fd, LISTEN_BACKLOG) == -1){
		if(errno == EWOULDBLOCK){
			state = LISTENING;
			lerr = NULL;
			return true;
		}
	}
	lerr = strerror(errno);
	return false;
}

void Socket::disconnect()
{
	if(state != IDLE)
		cleanup();
	lerr = NULL;
}
