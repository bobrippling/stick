#include <SDL/SDL.h>

#include "config.h"

#include "net/headers.h"

#include "net/addr.h"
#include "gfx.h"
#include "2d.h"
#include "obj.h"
#include "plat.h"
#include "stick.h"
#include "files.h"
#include "util.h"

Stick::Stick(const struct sockaddr_in *addr, const char *name, float x, float y, float speed, float heading)
	: Addressable(addr),
	Obj(Files::img_stick(), x, y, speed, heading),
	_name(new char[1 + strlen(name)]),
	_facing(0),
	_last_bullet(Util::mstime() - CONF_BULLET_DELAY),
	_cur_platform(-1)
{
	strcpy(_name, name);
}

Stick::~Stick()
{
	delete[] _name;
}

bool Stick::on_platform() const
{
	return _cur_platform != -1;
}

void Stick::move_to_platform(Platform &p)
{
	_y = p.get_y();
	_speed = 0.0f;
}

bool Stick::touches(enum touch_type t, Box &b)
{
	switch(t){
		case TOUCH_ALL:
			return Mover::touches(b);

		case TOUCH_FEET:
			return Mover::touches(b, _x, _y + _h - CONF_FEET_H, _w, (float)CONF_FEET_H);
			// FIXME: test
	}
	return false;
}
