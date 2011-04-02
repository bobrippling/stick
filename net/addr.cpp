#include <cstring>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "addr.h"

#ifdef _WIN32
static int inet_aton(const char *address, struct in_addr *sock)
{
	int s;
	s = inet_addr(address);
	if(s == 1)
		return false;
	sock->s_addr = s;
	return true;
}
#endif

bool lookup(const char *host, const char *port, struct sockaddr *addr)
{
	struct addrinfo *res = NULL;

	if(getaddrinfo(host, port, NULL, &res))
		return false;

	memcpy(addr, res->ai_addr, sizeof addr);

	freeaddrinfo(res);
	return true;
}

SockAddr::SockAddr(const SockAddr &a)
	: _addr(a._addr), _valid(a._valid)
{
}

SockAddr::SockAddr(const struct sockaddr_in *a)
	: _addr(), _valid(false)
{
	if(a){
		memcpy(&_addr, a, sizeof _addr);
		_valid = true;
	}
}

SockAddr::SockAddr(const char *ip, const char *port)
	: _addr(), _valid(false)
{
	if(ip)
		_valid = lookup(ip, port, reinterpret_cast<struct sockaddr *>(&_addr));
}


const struct sockaddr_in *SockAddr::getaddr() const
{
	if(_valid)
		return &_addr;
	return NULL;
}

const struct sockaddr *SockAddr::getgenericaddr() const
{
	if(_valid)
		return reinterpret_cast<const struct sockaddr *>(&_addr);
	return NULL;
}

const char *SockAddr::c_str()
{
	if(_valid)
		return str(&_addr);
	return NULL;
}

const char *SockAddr::str(const struct sockaddr *addr, socklen_t len)
{
	static char buffer[INET6_ADDRSTRLEN];
	const bool ip6 = len == sizeof(struct sockaddr_in);
	const void *ptr;

	if(ip6)
		ptr = &reinterpret_cast<const struct sockaddr_in6 *>(addr)->sin6_addr;
	else
		ptr = &reinterpret_cast<const struct sockaddr_in  *>(addr)->sin_addr;

	return inet_ntop(ip6 ? AF_INET6 : AF_INET, ptr, buffer, sizeof buffer);
}

#define QUICK(type) \
const char *SockAddr::str(const struct type *addr) \
{ return str(reinterpret_cast<const struct sockaddr *>(addr), sizeof *addr); }
QUICK(sockaddr_in)
QUICK(sockaddr_in6)
#undef QUICK


int SockAddr::port() const
{
	if(_valid)
		return ntohs(_addr.sin_port);
	return -1;
}

bool SockAddr::valid() const
{
	return _valid;
}

bool SockAddr::equals(const SockAddr *a) const
{
	if(!a || !a->_valid)
		return !_valid; // if we're not valid also, they're equal

	return memcmp(&a->_addr, &_addr, sizeof _addr) == 0;
}

// --------------------


Addressable::Addressable(const SockAddr &addr)
	: _addr(new SockAddr(addr))
{
}

Addressable::Addressable(const struct sockaddr_in &addr)
	: _addr(new SockAddr(&addr))
{
}

Addressable::Addressable()
	: _addr(NULL)
{
}

Addressable::~Addressable()
{
	if(_addr)
		delete _addr;
}

const SockAddr *Addressable::getsockaddr() const
{
	return _addr;
}

bool Addressable::addr_equals(const SockAddr *a) const
{
	if(!a && !_addr)
		return true;
	else if(!_addr || !a)
		return false;

	return _addr->equals(a);
}
