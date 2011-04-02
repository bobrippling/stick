#include <cstring>
#include <cstdio>

#include <SDL/SDL.h>

#include "net/headers.h"

#include "net/addr.h"
#include "net/udp_socket.h"
#include "gfx.h"
#include "2d.h"
#include "stick.h"
#include "util.h"
#include "files.h"
#include "global.h"

#define CONF_PORT "2848"
#define CONF_MAX_STICKS 128

using namespace Global;

bool net()
{
	return true;
}

void cleanup()
{
	delete sock;
	for(int i = 0; i < nsticks; i++)
		delete sticks[i];
	delete[] sticks;
}

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
	const char *ip = NULL, *port = CONF_PORT;
	int ret = 0;

	for(int i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-l"))
			host = 1;
		else if(!strcmp(argv[i], "-p"))
			if(++i < argc)
				port = argv[i];
			else{
				fputs("need port\n", stderr);;
				USAGE();
			}
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

	if(!Files::init())
		return 1;

	sticks = new Stick *[CONF_MAX_STICKS];
	sock   = new UDPSocket();

	for(int i = 0; i < CONF_MAX_STICKS; i++)
		sticks[i] = NULL;

	stick_me = sticks[0] = new Stick(NULL, "Tim");

	if(!sock->init(ip, ip ? port : 0)){
		perror("sock->init()");
		goto bail;
	}else if(host && !sock->bind(port)){
		perror("sock->bind()");
		goto bail;
	}

	if(!GFX::init())
		goto bail;

	// main loop
	for(;;){
		if(!GFX::events())
			break;

		if(!net())
			break;

		if(redraw){
			redraw = false;
			GFX::draw(true, sticks, nsticks, NULL);
		}

		Util::mssleep(30);
	}

fin:
	cleanup();
	return ret;
bail:
	ret = 1;
	goto fin;
}
