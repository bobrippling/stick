#ifndef STICK_H
#define STICK_H

class Stick : public Addressable, public Drawable, public Position
{
	private:
		char *_name;

	public:
		Stick(const struct sockaddr_in *addr, const char *name);
		virtual void draw(SDL_Surface *) const;
};

#endif
