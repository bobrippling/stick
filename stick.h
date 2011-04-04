#ifndef STICK_H
#define STICK_H

#include "common.h"

class Stick : public Addressable, public Obj
{
	private:
		char *_name;

		float _facing;
		long _last_bullet;

		int _cur_platform;

	public:
		enum touch_type
		{
			TOUCH_FEET, TOUCH_ALL
		};

		Stick(const struct sockaddr_in *addr, const char *name,
				float x, float y, float speed, float heading);
		virtual ~Stick();

		ACCESS(float, _facing)
		ACCESS(long,  _last_bullet)
		ACCESS(int,   _cur_platform)

		bool on_platform() const;
		void move_to_platform(Platform &p);

		bool touches(enum touch_type t, Box &b);
};

#endif
