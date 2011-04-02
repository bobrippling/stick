#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "files.h"
#include "util.h"

namespace Files
{
#if 0
	static void transparentsurface(SDL_Surface *img)
	{
		uint32_t colourkey = SDL_MapRGB(img->format, 0x00, 0xFF, 0xFF);
		SDL_SetColorKey(img, SDL_SRCCOLORKEY, colourkey);
	}
#endif

	static SDL_Surface *loadimage(const char *path)
	{
		SDL_Surface *img;

		img = IMG_Load(path);
		if(!img)
			return NULL;

#ifdef OPTIMISE
		SDL_Surface *opt = SDL_DisplayFormat(img);
		SDL_FreeSurface(img);
		if(!opt)
			return NULL;

		transparentsurface(opt);
		return opt;
#else
		return img;
#endif
	}


	static SDL_Surface *bg, *stick;


	bool init()
	{
#define LOAD(v, p) \
		do \
			if(!(v = loadimage(p))){ \
				perror(p); \
				return false; \
			} \
		while(0)

		LOAD(bg,    "img/bg.bmp");
		LOAD(stick, "img/stick.bmp");

		return true;
#undef LOAD
	}

	void term()
	{
		SDL_FreeSurface(bg);
		SDL_FreeSurface(stick);
	}

#define GETTER(n) \
	SDL_Surface *img_##n() { return n; }

	GETTER(bg)
	GETTER(stick)
}
