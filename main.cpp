#include <iostream>
#include <cstring>
#include <cstdio>

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tcp_socket.h"

#define CONF_PORT 2848
#define BUF_SIZE  256

static void usage(const char *);

UDPSocket *server, *clients;

static void usage(const char *n)
{
	fprintf(stderr, "Usage: %s [OPTIONS] host\n", n);
	fputs(" -l:     listen\n", stderr);
	fputs(" -p num: port\n", stderr);
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
				fputs("need port\n", stderr);;
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
		fputs("need host\n", stderr);
		USAGE();
	}


	try{
		if(host){
			server = new UDPSocket(NULL, port);
	}catch(const char *s){
		fprintf(stderr, "Caught exception! %s\n", s);
		return 1;
	}

	return 0;
}
