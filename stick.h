#ifndef STICK_H
#define STICK_H

#include "common.h"

class Stick : public Addressable, public Obj
{
	private:
		char *_name;

		float _facing;
		long _last_bullet;

	public:
		Stick(const struct sockaddr_in *addr, const char *name);
		virtual ~Stick();

		ACCESS(_facing)
		ACCESS(_last_bullet)
};

#endif
