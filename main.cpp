#include <iostream>
#include <cstring>
#include <cstdio>

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "socket.h"

#define CONF_PORT 2848

static void usage(const char *);

static void usage(const char *n)
{
	std::cerr << "Usage: " << n << " [OPTIONS] host\n";
	std::cerr << " -l: listen\n";
	exit(1);
}


int main(int argc, const char **argv)
{
#define USAGE() usage(*argv)
#define USEC 500000
	bool listen = 0;
	const char *host = NULL;
	int port = CONF_PORT;
	Socket s;

	for(int i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-l"))
			listen = 1;
		else if(!strcmp(argv[i], "-p"))
			if(++i < argc)
				port = atoi(argv[i]);
			else{
				std::cerr << "need port\n";
				USAGE();
			}
		else if(!host)
			host = argv[i];
		else
			USAGE();

	// sanity
	if(listen && host)
		USAGE();

	if(!listen && !host){
		std::cerr << "need host\n";
		USAGE();
	}


	if(listen){
		std::cerr << "listening...\n";
		if(!s.listen()){
			std::cerr << "couldn't listen: ";
			perror(NULL);
		}

	}else{
		if(!s.connect(host, port)){
			std::cerr << "couldn't connect to " << host << ": " << s.lasterr() << std::endl;
		}else{
			std::string in;

			std::cout << "connecting...\n";
			for(;;){
				struct timeval waittime;
				bool brk = false;

				waittime.tv_sec  = 1;
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
				for(;;){
					std::cout << "$ " << std::flush;
					if(!std::getline(std::cin, in)){
						std::cout << '\n';
						break;
					}
					s << in << '\n';
				}

				s.disconnect();
			}
		}
	}

	return 0;
#undef USAGE
}
