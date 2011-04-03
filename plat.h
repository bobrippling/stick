#ifndef PLAT_H
#define PLAT_H

#include "common.h"

class Platform : public Drawable, public Box
{
	public:
		inline Platform(float x, float y, float w, float h)
			: Box(x, y, w, h)
		{
		}

		inline void draw(SDL_Surface *s) const
		{
			Draw::rect(s, (int)_x, (int)_y, (int)(_x+_w), (int)(_y+_h), 0, 0, 0);
		}
};

#endif
