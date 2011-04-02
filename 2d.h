#ifndef TWO_D_H
#define TWO_D_H

#include <cmath>

#define ACCESS(n) \
		inline float get##n() const {return n;} \
		inline void  set##n(float i){n = i;}


/*
 * ---> =   0 degrees
 * <--- = 180 degrees
 *   ^  =  90 degrees
 *   V  = 270 degrees
 */

class Vector
{
	private:
		float _speed, _heading;

	public:
		static inline float clipangle(register float a)
		{
			while(a > M_PI)
				a -= 2 * M_PI;
			while(a < -M_PI)
				a += 2 * M_PI;
			return a;
		}

		inline Vector() : _speed(0), _heading(0)
		{
		}

		inline Vector(float xc, float yc)
			: _speed(0), _heading(0)
		{
			_speed = sqrt(xc * xc + yc * yc);

			if(xc > 0)
				_heading = atan(yc / xc);
			else if(xc < 0)
				_heading = M_PI/2.0f + atan(yc / xc);
			else if(yc > 0)
				_heading = M_PI;
			else
				_heading = clipangle(3 * M_PI);
		}

		inline Vector(const Vector &v) : _speed(v._speed), _heading(v._heading)
		{
		}

		inline Vector &operator=(const Vector &v)
		{
			_speed       = v._speed;
			_heading     = v._heading;
			return *this;
		}

		inline ~Vector()
		{
		}

		ACCESS(_speed)
		ACCESS(_heading)
};


class Position
{
	private:
		float _x, _y;

	public:
		inline Position() : _x(0), _y(0)
		{
		}

		inline Position(const Position &p) : _x(p._x), _y(p._y)
		{
		}

		inline Position &operator=(const Position &p)
		{
			_x       = p._x;
			_y       = p._y;
			return *this;
		}

		inline ~Position()
		{
		}

		ACCESS(_x)
		ACCESS(_y)

		inline void apply_vector(Vector &v)
		{
			register float xc, yc;

			xc = v.get_speed() * cos(v.get_heading());
			yc = v.get_speed() * sin(v.get_heading());

			_x += xc;
			_y += yc;
		}
};
#endif
