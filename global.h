#ifndef GLOBAL_H
#define GLOBAL_H

namespace Global
{
	extern UDPSocket  *sock;
	extern Stick     **sticks, *stick_me;
	extern int         nsticks;
	extern bool        redraw;
}

#endif
