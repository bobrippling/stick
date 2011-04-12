#ifndef GLOBAL_H
#define GLOBAL_H

namespace Global
{
	extern Stick     **sticks, *stick_me;
	extern Bullet    **bullets;
	extern Platform  **platforms;

	extern UDPSocket  *sock;

	extern int         nsticks;
	extern int         nbullets;
	extern bool        redraw;

	struct keys
	{
		bool left, right, up, down;
		bool button_left, button_right;
		bool jump;
		int x, y;
	};
	extern struct keys keys;
}

#endif
