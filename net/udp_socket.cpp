#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "addr.h"
#include "udp_socket.h"


UDPSocket::UDPSocket()
	:
	lerr(NULL), buffer(NULL),
	buffersize(0), fd(-1)
{
}

bool UDPSocket::init(const char *host, const char *port, int bs)
{
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(fd == -1)
		goto sockbail;

	buffer = new char[bs];

#if defined(SO_NOSIGPIPE)
	{
		int val = 1;
		// ignore sigpipe - FreeBSD
		setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
	}
#endif

	if(host){
		SockAddr addr(host, port);

		if(addr.valid()){
			if(connect(fd, addr.getgenericaddr(),
						sizeof(addr.getgenericaddr())) == -1)
				goto sockbail;
		}else
			goto sockbail;
	}

	return true;
sockbail:
	close(fd);
	return false;
}

bool UDPSocket::bind(const char *port)
{
	SockAddr local("127.0.0.1", port);
	if(local.valid())
		return ::bind(fd, local.getgenericaddr(), sizeof(struct sockaddr_in)) != -1;
	return false;
}

UDPSocket::~UDPSocket()
{
	close(fd);
	delete[] buffer;
	if(lerr)
		delete[] lerr;
}

const char *UDPSocket::remoteaddr()
{
	/* getpeername() */
	return NULL;
}

bool UDPSocket::senddata(const void *p, size_t siz, struct sockaddr *to, socklen_t len)
{
	int ret;

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = sendto(fd, p, siz, MSG_NOSIGNAL, to, len);
#elif defined(SO_NOSIGPIPE)
	ret = sendto(fd, p, siz, 0, to, len);
#else
	// not bsd nor linux
	// neither, nor:
	// http://krokisplace.blogspot.com/2010/02/suppressing-sigpipe-in-library.html
# error unknown OS
#endif

	if(ret == -1 || errno == EPIPE)
		return false;
	return true;
}

const char *UDPSocket::lasterr() const
{
	return lerr;
}

bool UDPSocket::recvdata(void *p, size_t siz, struct sockaddr *from, socklen_t *len)
{
	int ret;

#if defined(MSG_NOSIGNAL) && !defined(SO_NOSIGPIPE)
	ret = recvfrom(fd, p, siz, MSG_NOSIGNAL, from, len);
#elif defined(SO_NOSIGPIPE)
	ret = recvfrom(fd, p, siz, 0, from, len);
#else
# error unknown OS
#endif

	if(ret == -1)
		return false;
	return true;
}

bool UDPSocket::recvdata(void *p, size_t s)
{
	return recvdata(p, s, NULL, NULL);
}

bool UDPSocket::valid() const
{
	return fd != -1;
}
