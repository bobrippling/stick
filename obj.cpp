#include <SDL/SDL.h>

#include "config.h"

#include "gfx.h"
#include "2d.h"
#include "obj.h"

Obj::Obj(SDL_Surface *img, float x, float y, float speed, float heading)
	: Mover(x, y, speed, heading, img->w, img->h), _img(img)
{
}

void Obj::draw(SDL_Surface *sur) const
{
	Draw::xyblit(sur, _img, _x, _y);
}
