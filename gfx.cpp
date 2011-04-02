#include <cstdarg>

#include <SDL/SDL.h>

#include "config.h"

#include "gfx.h"
#include "files.h"
#include "net/headers.h"
#include "net/addr.h"
#include "net/udp_socket.h"
#include "2d.h"
#include "obj.h"
#include "stick.h"
#include "bullet.h"
#include "global.h"

#define KEY_INITIAL_WAIT 100
#define KEY_REPEAT_WAIT  50

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

		return Draw::init();
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

				case SDL_MOUSEMOTION:
					break;

				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
					switch(event.button.button){
#define BUTTON(k, b) \
						case k: \
							b = event.type == SDL_MOUSEBUTTONDOWN; \
							break

						BUTTON(SDL_BUTTON_LEFT,  keys.button_left);
						BUTTON(SDL_BUTTON_RIGHT, keys.button_right);
#undef BUTTON
					}
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
					switch(event.key.keysym.sym){
#define KEY(k, v) case k: v = event.type == SDL_KEYDOWN; break
						KEY(SDLK_a, keys.left);
						KEY(SDLK_d, keys.right);
						KEY(SDLK_w, keys.up);
						KEY(SDLK_s, keys.down);

						default:
							break;
#undef KEY
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



#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <unistd.h>

#define FONT_NAME "fonts/arial.ttf"

namespace Draw
{
	static TTF_Font *font;

	bool init()
	{
		if(TTF_Init()){
			perror("TTF_Init()");
			return false;
		}

		atexit(TTF_Quit);

		if(access(FONT_NAME, R_OK)){
			perror(FONT_NAME);
			return false;
		}

		font = TTF_OpenFont(FONT_NAME, 12);
		if(!font){
			perror("TTF_OpenFont()");
			return false;
		}

		return true;
	}

	void term()
	{
		TTF_CloseFont(font);
		font = NULL;
	}

	void xyblit(SDL_Surface *dst, SDL_Surface *src, int x, int y)
	{
		SDL_Rect offset = { x, y, 0, 0};

		SDL_BlitSurface(src, NULL, dst, &offset);
	}

	void rect(SDL_Surface *screen, int x, int y, int len,
			short r, short g, short b, short a)
	{
		rect(screen, x, y, x + len, y + len, r, g, b, a);
	}

	void rect(SDL_Surface *screen, int x, int y, int x2, int y2,
			short r, short g, short b, short a)
	{
		// -----
		lineRGBA(screen, x, y, x2, y, r, g, b, a);
		// |
		lineRGBA(screen, x, y, x, y2, r, g, b, a);
		//      |
		lineRGBA(screen, x2, y, x2, y2, r, g, b, a);
		// ------
		lineRGBA(screen, x, y2, x2, y2, r, g, b, a);
	}

	void fillrect(SDL_Surface *screen, int x, int y, int w, int h,
			short r, short g, short b, short a)
	{
		SDL_Rect rect = { x, y, w, h };

		SDL_FillRect(screen, &rect, SDL_MapRGBA(screen->format, r, g, b, a));
	}

	void circle(SDL_Surface *screen, int x, int y, int rad, short r, short g, short b, short a)
	{
		circleRGBA(screen, x, y, rad, r, g, b, a);
	}

	void line(SDL_Surface *screen, int x, int y, int x2, int y2,
			short r, short g, short b, short a)
	{
		lineRGBA(screen, x, y, x2, y2, r, g, b, a);
	}

	void cross(SDL_Surface *screen, int x, int y, int w, int h,
			short r, short g, short b, short alpha)
	{
		line(screen, 0, y, w, y, r, g, b, alpha);
		line(screen, x, 0, x, h, r, g, b, alpha);
	}

	void text(SDL_Surface *screen, int x, int y,
			short r, short g, short b, short a, const char *fmt, ...)
	{
		// Note: needs TTF_Init(); to be called first
		SDL_Color col = { r, g, b, a };
		SDL_Surface *surf;
		char text[256];
		va_list l;

		va_start(l, fmt);
		vsnprintf(text, sizeof text, fmt, l);
		va_end(l);

		surf = TTF_RenderText_Solid(font, text, col);

		xyblit(screen, surf, x, y);

		SDL_FreeSurface(surf);
	}
}
