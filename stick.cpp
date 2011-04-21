#include <SDL/SDL.h>

#include "config.h"

#include "net/headers.h"

#include "net/addr.h"
#include "gfx.h"
#include "2d.h"
#include "obj.h"
#include "plat.h"
#include "serial.h"
#include "stick.h"
#include "files.h"
#include "util.h"

Stick::Stick(const struct sockaddr_in *addr, const char *name, float x, float y, float speed, float heading)
	: Addressable(addr),
	Obj(Files::img_stick(), x, y, speed, heading),
	_name(new char[1 + strlen(name)]),
	_facing(0),
	_last_bullet(Util::mstime() - CONF_BULLET_DELAY),
	_cur_platform(-1), _last_jump(Util::mstime() - CONF_STICK_JUMP_DELAY)
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

void Stick::jump()
{
	static const Vector jmpvec CONF_JUMP_VECTOR;
	const long now = Util::mstime();

	if(on_platform() && _last_jump + CONF_STICK_JUMP_DELAY < now){
		_cur_platform = -1;
		_last_jump = now;
		Position::apply_vector(jmpvec);
	}
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

const char *Stick::serial(int id) const
{
	static char buffer[32];

	memcpy(buffer    , &id, sizeof id);

	memcpy(buffer + 4, &_x, sizeof _x);
	memcpy(buffer + 8, &_y, sizeof _y);

	memcpy(buffer + 12, &_speed,   sizeof _speed);
	memcpy(buffer + 16, &_heading, sizeof _heading);

	return buffer;
}
