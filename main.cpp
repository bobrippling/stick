#include <iostream>
#include <cstring>
#include <cstdio>

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "socket.h"

#define CONF_PORT 2848
#define BUF_SIZE  256

static void usage(const char *);
void loop();

Socket& connreq();
void disconn();
void receive(void *, size_t);
void error();

Socket server(BUF_SIZE, &connreq, &disconn, &receive, &error);
Socket client(BUF_SIZE, NULL, &disconn, &receive, &error);
Socket *connsock;


static void usage(const char *n)
{
	std::cerr << "Usage: " << n << " [OPTIONS] host\n";
	std::cerr << " -l: listen\n";
	exit(1);
}

int main(int argc, const char **argv)
{
#define USAGE() usage(*argv)
	bool host = 0;
	const char *ip = NULL;
	int port = CONF_PORT;

	for(int i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-l"))
			host = 1;
		else if(!strcmp(argv[i], "-p"))
			if(++i < argc)
				port = atoi(argv[i]);
			else{
				std::cerr << "need port\n";
				USAGE();
			}
		else if(!strcmp(argv[i], "--help"))
			USAGE();
		else if(!host)
			ip = argv[i];
		else
			USAGE();

	// sanity
	if(host && ip)
		USAGE();

	if(!host && !ip){
		std::cerr << "need host\n";
		USAGE();
	}


	try{
		if(host){
			server.listen(CONF_PORT);
			connsock = NULL;
		}else
			(connsock = &client)->connect(ip, port);
		loop();
	}catch(const char *s){
		std::cerr << "Caught exception! " << s << std::endl;
		return 1;
	}

	return 0;
}

Socket& connreq()
{
	std::cout << "connreq()\n";
	return *(connsock = &client);
}

void disconn()
{
	std::cout << "disconn()\n";
}

void receive(void *d, size_t l)
{
	std::cout << "receive(): " << (char *)d << "(" << l << ")" << std::endl;
}

void error()
{
	std::cout << "error(): " <<
		(connsock ? connsock->lasterr() : server.lasterr())
		<< std::endl;
}

#define WAIT() \
	do{ \
		struct timeval tv; \
		tv.tv_sec  = 1; \
		tv.tv_usec = 0; \
		select(0, NULL, NULL, NULL, &tv); \
	}while(0)

void loop()
{
	do{
		WAIT();


		if(connsock){
			std::cout << "(client) ";
			if(connsock->getstate() == Socket::CONNECTED){
				if(connsock->senddata("hi there", 9))
					std::cout << "sent message\n";
				else
					std::cerr << "connsock->senddata(): " << connsock->lasterr()
																	 << std::endl;
				break;
			}else
				std::cerr << "not connected (" << connsock->getstatestr()
					<< "), running events: (bool)" << connsock->runevents()
					<< std::endl;
		}else{
			// server - still waiting
			std::cout << "(server) ";
			if(server.runevents()){
				std::cerr << "server->runevents(): state: "
										 << server.getstatestr() << std::endl;
				if(connsock)
					std::cerr << "  connsock: "
									 << connsock->getstatestr() << std::endl;
			}
		}

		std::cout << "waiting... " << (connsock ? connsock->getstatestr() :
			server.getstatestr())
			<< std::endl;
	}while(1);


	while(connsock->getstate() == Socket::CONNECTED){
		std::cout << "connected (" << connsock->remoteaddr() <<
			"), waiting - " << connsock->getstatestr() << std::endl;

		if(connsock->runevents()){
			std::cout << "runevents() success, fin.\n";
			break;
		}
		WAIT();
	}

	connsock->disconnect();
}
