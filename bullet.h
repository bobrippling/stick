#ifndef BULLET_H
#define BULLET_H

class Bullet : public Drawable, public Mover
{
	private:
		float _offset_x, _offset_y;

		void calc_dim();

	public:
		Bullet(float x, float y, float speed, float heading);
		virtual void draw(SDL_Surface *) const;
};


#endif
