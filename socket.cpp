#include <sys/socket.h>
#include <string.h>

#include "socket.h"

#define DOMAIN AF_INET /* AF_INET6 */
#define TYPE SOCK_STREAM
#define PROTOCOL 0
#define LISTEN_BACKLOG 10

Socket::Socket() : fd(socket(DOMAIN, TYPE, PROTOCOL)), state(IDLE)
{
	if(fd == -1)
		throw;
}

Socket::~Socket()
{
	if(state != IDLE)
		cleanup();
}

void Socket::cleanup()
{
	shutdown(fd, SHUT_RDWR);
	fd = -1;
}

bool Socket::senddata(char *data)
{
	if(state != CONNECTED)
		throw;

	return sendto(fd, data, strlen(data), 0, NULL, 0) != -1;
	//                                addr, addrlen
}

bool Socket::recvdata(char *buf, int len)
{
	if(state != CONNECTED)
		throw;

	return recvfrom(fd, buf, len, 0, NULL, 0) != -1;
}

enum Socket::State Socket::getstate()
{
	return state;
}
