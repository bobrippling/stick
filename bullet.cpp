#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "config.h"

#include "gfx.h"
#include "2d.h"
#include "bullet.h"

Bullet::Bullet(float x, float y, float speed, float heading)
	: Mover(x, y, speed, heading, 0, 0),
	_offset_x(), _offset_y()
{
	calc_dim();
}

void Bullet::draw(SDL_Surface *sur) const
{
	Draw::line(sur, _x, _y, _x + _offset_x, _y + _offset_y, 0, 0, 0);
}

void Bullet::calc_dim()
{
	_offset_x = CONF_BULLET_LEN * cosf(_heading);
	_offset_y = CONF_BULLET_LEN * sinf(_heading);
}
