#include <cstdarg>

#include <SDL/SDL.h>

#include "gfx.h"
#include "files.h"
#include "net/headers.h"
#include "net/addr.h"
#include "net/udp_socket.h"
#include "2d.h"
#include "stick.h"
#include "global.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define SCREEN_BITS      32

#define KEY_INITIAL_WAIT 100
#define KEY_REPEAT_WAIT  50

#define SCALE_SPEED 0.1

namespace GFX
{
	SDL_Surface *screen;

	bool init()
	{
		if(SDL_Init(SDL_INIT_EVERYTHING) == -1){
			fprintf(stderr, "SDL_Init failed\n");
			return false;
		}

		if(!(screen = SDL_SetVideoMode(
						SCREEN_WIDTH, SCREEN_HEIGHT,
						SCREEN_BITS, SDL_SWSURFACE))){
			fprintf(stderr, "SDL_SetVideoMode failed\n");
			return false;
		}

		SDL_EnableKeyRepeat(KEY_INITIAL_WAIT, KEY_REPEAT_WAIT);
		SDL_WM_SetCaption("Timmy", NULL);

		return true;
	}


	bool events()
	{
#define CONTROL_PRESSED() (SDL_GetModState() & (KMOD_RCTRL | KMOD_LCTRL))
		using namespace Global;

		SDL_Event event;

		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					return false;

				case SDL_MOUSEBUTTONDOWN:
					break;

				case SDL_MOUSEMOTION:
					break;

				case SDL_MOUSEBUTTONUP:
					if(event.button.button == SDL_BUTTON_LEFT)
						printf("left mouse\n");
					break;

				case SDL_KEYDOWN:
					switch(event.key.keysym.sym){
#define KEY_VEC(k, a, b) \
						case k: \
						{ \
							Vector v(a * SCALE_SPEED, b * SCALE_SPEED); \
							stick_me->add_speed(v); \
							redraw = true; \
							break; \
						}

						KEY_VEC(SDLK_a, -1.0f,  0.0f)
						KEY_VEC(SDLK_d, +1.0f,  0.0f)
						KEY_VEC(SDLK_w,  0.0f, -1.0f)
						KEY_VEC(SDLK_s,  0.0f, +1.0f)

						default:
							break;
					}
					break;
			}
		}
		return true;
#undef CONTROL_PRESSED
	}


	void draw(bool first, ...)
	{
		va_list l;

		SDL_BlitSurface(Files::img_bg(), NULL, screen, NULL);

		if(first){
			Drawable **draws;
			int n;

			va_start(l, first);

			for(;;){
				draws = va_arg(l, Drawable **);
				if(!draws)
					break;

				n = va_arg(l, int);
				for(int i = 0; i < n; i++)
					draws[i]->draw(screen);
			}

			va_end(l);
		}

		SDL_Flip(screen);
	}


	void term()
	{
		SDL_Quit();
	}
}
