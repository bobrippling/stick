#include <SDL/SDL.h>

#include "net/headers.h"

#include "net/addr.h"
#include "gfx.h"
#include "2d.h"
#include "stick.h"

Stick::Stick(const struct sockaddr_in *addr, const char *name)
	: Addressable(addr), _name(strdup(name))
{
}

void Stick::draw(SDL_Surface *sur) const
{
	printf("Stick[%s]->draw(%p)\n", _name, sur);
}
