#ifndef GLOBAL_H
#define GLOBAL_H

namespace Global
{
	extern UDPSocket  *sock;
	extern Stick     **sticks, *stick_me;
	extern Bullet    **bullets;
	extern int         nsticks;
	extern int         nbullets;
	extern bool        redraw;

	struct keys
	{
		bool left, right, up, down,
				 button_left, button_right;
		int x, y;
	};
	extern struct keys keys;
}

#endif
