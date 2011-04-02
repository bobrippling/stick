#ifndef FILES_H
#define FILES_H

namespace Files
{
	SDL_Surface *img_bg();

	bool init();
	void term();

#define GETTER(n) \
	SDL_Surface *img_##n();

	GETTER(bg)
	GETTER(stick)
	GETTER(bullet)

#undef GETTER
}

#endif
