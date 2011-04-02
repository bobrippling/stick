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

namespace Global
{
	UDPSocket  *sock;
	Stick     **sticks, *stick_me;
	int         nsticks = 0;
	bool        redraw = true;
}
