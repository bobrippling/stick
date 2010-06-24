#include <iostream>
#include <cstring>
#include <cstdio>

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "socket.h"

#define CONF_PORT 2848

static void usage(const char *);
void connect(const char *, int);
void listen(int);


static void usage(const char *n)
{
	std::cerr << "Usage: " << n << " [OPTIONS] host\n";
	std::cerr << " -l: listen\n";
	exit(1);
}

void connect(const char *host, int port)
{
#define USEC 500000
	Socket s;

	if(!s.connect(host, port)){
		std::cerr << "couldn't connect to " << host << ": " << s.lasterr() << std::endl;
	}else{
		std::string in;

		std::cout << "connecting...\n";

		for(;;){
			struct timeval waittime;
			bool brk = false;

			waittime.tv_sec  = 1; // set it each time - the syscall changes the value
			waittime.tv_usec = USEC;

			switch(s.getstate()){
				case Socket::CONNECTED:
					std::cout << "connected\n";
					brk = true;
					break;
				case Socket::IDLE:
					std::cout << s.lasterr() << std::endl;
					brk = true;
				default:
					break;
			}
			if(brk)
				break;

			select(0, NULL, NULL, NULL, &waittime);
		}

		if(s.getstate() == Socket::CONNECTED){
			// TODO: branch to mainloop()
			for(;;){
				std::cout << "$ " << std::flush;
				if(!std::getline(std::cin, in)){
					std::cout << '\n';
					break;
				}
				if(!s.senddata(in) || !s.senddata('\n')){
					std::cerr << "Couldn't write to socket: " << s.lasterr() << std::endl;
					break;
				}
			}

			s.disconnect();
		}
	}
}

void listen(int port)
{
	Socket s;

	if(!s.listen(port)){
		std::cerr << "couldn't listen: " << s.lasterr() << std::endl;
		return;
	}

	std::cerr << "listening...\n";

	for(;;){
		Socket *client = s.accept();

		if(client){
			std::cout << "got connection from " << client->remoteaddr() << std::endl;

			for(;;){
				std::string in;

				std::cout << "$ " << std::flush;

				if(!std::getline(std::cin, in)){
					std::cout << '\n';
					std::cin.clear();
					break;
				}
				if(!client->senddata(in) || !client->senddata('\n')){
					std::cerr << "Couldn't write to socket: " << client->lasterr() << std::endl;
					break;
				}
			}
			client->disconnect();
			delete client;
		}else if(s.lasterr()){
			std::cerr << s.lasterr() << std::endl;
			break;
		}else{
			struct timeval waittime;
			waittime.tv_sec = 1;
			waittime.tv_usec = USEC;

			select(0, NULL, NULL, NULL, &waittime);
		}
	}
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


	if(host)
		listen(port);
	else
		connect(ip, port);

	return 0;
}
