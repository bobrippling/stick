#ifndef OBJ_H
#define OBJ_H

class Obj : public Drawable, public Mover
{
	private:
		SDL_Surface *_img;

	public:
		Obj(SDL_Surface *img, float x, float y, float speed, float heading);

		virtual void draw(SDL_Surface *) const;
};


#endif
