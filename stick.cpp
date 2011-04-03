#include <SDL/SDL.h>

#include "config.h"

#include "net/headers.h"

#include "net/addr.h"
#include "gfx.h"
#include "2d.h"
#include "obj.h"
#include "stick.h"
#include "files.h"
#include "util.h"

Stick::Stick(const struct sockaddr_in *addr, const char *name)
	: Addressable(addr), Obj(Files::img_stick()),
	_name(new char[1 + strlen(name)]),
	_facing(0), _last_bullet(Util::mstime() - CONF_BULLET_DELAY)
{
	strcpy(_name, name);
}

Stick::~Stick()
{
	delete[] _name;
}
