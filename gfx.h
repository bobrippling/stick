#ifndef GFX_H
#define GFX_H

class Drawable
{
	public:
		virtual void draw(SDL_Surface *) const = 0;
};

namespace GFX
{
	bool init();
	bool events();
	void draw(bool first, ...); /* Drawable **, int n, Drawable **, int n, ... */
	void term();
}

#endif
