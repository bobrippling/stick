#include <SDL/SDL.h>

#include "config.h"

#include "net/headers.h"

#include "net/addr.h"
#include "net/udp_socket.h"
#include "gfx.h"
#include "2d.h"
#include "obj.h"
#include "stick.h"
#include "bullet.h"
#include "util.h"
#include "files.h"

#include "global.h"

namespace Global
{
	UDPSocket  *sock;

	Stick  **sticks, *stick_me;
	Bullet **bullets;

	int   nsticks = 0, nbullets = 0;
	bool  redraw = true;

	struct keys keys;
}
