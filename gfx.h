#ifndef GFX_H
#define GFX_H

class Drawable
{
	public:
		virtual void draw(SDL_Surface *) const = 0;
};

namespace GFX
{
	const int SCREEN_WIDTH  = CONF_WIDTH;
	const int SCREEN_HEIGHT = CONF_HEIGHT;
	const int SCREEN_BITS   = 32;

	bool init();
	bool events();
	void draw(bool first, ...); /* Drawable **, int n, Drawable **, int n, ... */
	void term();
}

namespace Draw
{
	bool init(void);
	void term(void);

	void xyblit(SDL_Surface *src, SDL_Surface *dst, int x, int y);

	void rect(SDL_Surface *, int x, int y, int len,
			short r, short g, short b, short alpha = 0xff);

	void rect(SDL_Surface *, int x, int y, int x2, int y2,
			short r, short g, short b, short alpha = 0xff);

	void fillrect(SDL_Surface *, int x, int y, int w, int h,
			short r, short g, short b, short alpha = 0xff);

	void circle(SDL_Surface *, int x, int y, int rad,
			short r, short g, short b, short alpha = 0xff);

	void line(SDL_Surface *, int x, int y, int x2, int y2,
			short r, short g, short b, short alpha = 0xff);

	void cross(SDL_Surface *, int x, int y, int w, int h,
			short r, short g, short b, short alpha = 0xff);

	void text(SDL_Surface *screen, int x, int y,
			short r, short g, short b, short a,
			const char *fmt, ...);
}

#endif
