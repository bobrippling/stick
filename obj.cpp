#include <SDL/SDL.h>

#include "config.h"

#include "gfx.h"
#include "2d.h"
#include "obj.h"

Obj::Obj(SDL_Surface *img)
	: Mover(img->w, img->h), _img(img)
{
}

void Obj::draw(SDL_Surface *sur) const
{
	Draw::xyblit(sur, _img, _x, _y);
}
