#include <cstring>
#include <cstdio>
#include <ctime>

#include <SDL/SDL.h>

#include "config.h"

#include "net/headers.h"

#include "net/addr.h"
#include "net/udp_socket.h"
#include "gfx.h"
#include "2d.h"
#include "obj.h"
#include "plat.h"
#include "serial.h"
#include "stick.h"
#include "bullet.h"
#include "plat.h"
#include "util.h"
#include "files.h"
#include "global.h"

using namespace Global;

void handle_keys()
{
#define KEY_VEC(k, a, b) \
	if(k){ \
		Vector v(Vector::VEC_XCOMP_YCOMP, a * CONF_SCALE_SPEED, b * CONF_SCALE_SPEED); \
		stick_me->add_vector(v); \
		redraw = true; \
	}

	KEY_VEC(keys.up,     0.0f, -1.0f)
	KEY_VEC(keys.down,   0.0f, +1.0f)
	KEY_VEC(keys.left,  -1.0f,  0.0f)
	KEY_VEC(keys.right, +1.0f,  0.0f)
#undef KEY_VEC

	if(keys.jump)
		stick_me->jump();

#if 0
	printf("up:%d down:%d left:%d right:%d bleft:%d bright:%d\r",
		keys.up, keys.down, keys.left, keys.right,
		keys.button_left, keys.button_right);
#endif

	if(keys.button_left){
		long now = Util::mstime();

		if(stick_me->get_last_bullet() + CONF_BULLET_DELAY <= now){
			stick_me->set_last_bullet(now);

			// FIXME: apply vector shiz
			// FIXME: bound check
			for(int i = 0; i < nbullets; i++)
				if(!bullets[i]){
					bullets[i] = new Bullet(
							stick_me->get_x() + stick_me->get_w()/2,
							stick_me->get_y() + stick_me->get_h()/2,
							CONF_BULLET_SPEED, stick_me->get_facing());
					nbullets++;
					break;
				}
		}
	}
}

void physics()
{
	static const Vector vec_gravity CONF_GRAVITY;

	for(int i = 0; i < nsticks; i++){
		sticks[i]->apply_vector();
		sticks[i]->clip(0, 0, GFX::SCREEN_WIDTH, GFX::SCREEN_HEIGHT, Mover::CLIP_BOUNCE);

		if(!sticks[i]->on_platform()){
			sticks[i]->add_vector(vec_gravity);

			for(int j = 0; j < CONF_NPLATFORMS; j++)
				if(sticks[i]->touches(Stick::TOUCH_FEET, *platforms[j])){
					printf("stick %d -> platform %d\n", i, j);
					sticks[i]->set_cur_platform(j);
					sticks[i]->move_to_platform(*platforms[j]);
				}
		}
	}

	for(int i = 0; i < nbullets; i++){
		if(bullets[i]){
			bullets[i]->apply_vector();

			if(bullets[i]->clip(0, 0, GFX::SCREEN_WIDTH, GFX::SCREEN_HEIGHT, Mover::CLIP_NONE)){
				delete bullets[i];
				bullets[i] = NULL;
				nbullets--;
			}
		}
	}
}


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

void var_init()
{
	memset(&keys, 0, sizeof keys);

	sock = new UDPSocket();

	sticks = new Stick *[CONF_MAX_STICKS];
	memset(sticks, 0, CONF_MAX_STICKS * sizeof(sticks[0]));

	stick_me = sticks[0] = new Stick(NULL, "Tim", 0.0f, 0.0f, 0.0f, 0.0f);
	nsticks++;
	stick_me->set_x(GFX::SCREEN_WIDTH  / 2);
	stick_me->set_y(GFX::SCREEN_HEIGHT / 2);

	bullets = new Bullet *[CONF_MAX_BULLETS];
	memset(bullets, 0, CONF_MAX_BULLETS * sizeof(bullets[0]));

	// FIXME: platform[0] is the base platform
	platforms = new Platform *[CONF_NPLATFORMS];
	for(int i = 0; i < CONF_NPLATFORMS; i++)
		platforms[i] = new Platform(
			rand() % CONF_WIDTH,
			rand() % CONF_HEIGHT,
			rand() % (CONF_PLAT_MAX_W - CONF_PLAT_MIN_W) + CONF_PLAT_MIN_W,
			CONF_PLAT_H);
}

int main(int argc, const char **argv)
{
#define USAGE() usage(*argv)
	bool host = 0;
	const char *ip = NULL, *port = CONF_PORT;
	int ret = 0;

	srand(time(NULL));

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

	setbuf(stdout, NULL);

	if(!Files::init())
		return 1;

	var_init();


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
		handle_keys();

		if(!net())
			break;

		physics();

		if(true || redraw){ // FIXME
			redraw = false;
			GFX::draw(true,
					sticks, nsticks,
					bullets, nbullets,
					platforms, CONF_NPLATFORMS,
					NULL);
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
