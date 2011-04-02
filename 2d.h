#ifndef TWO_D_H
#define TWO_D_H

#include <cmath>
#include "common.h"

/*
 * ---> =   0 degrees
 * <--- = 180 degrees
 *   ^  =  90 degrees
 *   V  = 270 degrees
 */

static inline float clipangle(register float a)
{
	while(a > M_PI)
		a -= 2 * M_PI;
	while(a < -M_PI)
		a += 2 * M_PI;
	return a;
}


static inline void xc_yc_to_speed_heading(float xc, float yc, float &speed, float &heading)
{
	speed = sqrtf(xc * xc + yc * yc);

	if(xc > 0)
		heading = atan(yc / xc);
	else if(xc < 0)
		heading = M_PI + atan(yc / xc);
	else if(yc > 0)
		heading =  M_PI / 2;
	else
		heading = -M_PI / 2;
}

static inline void speed_heading_to_xc_yc(float speed, float heading, float &xc, float &yc)
{
	xc = speed * cosf(heading);
	yc = speed * sinf(heading);
}


class Vector
{
	protected:
		float _speed, _heading;

	public:
		inline Vector() : _speed(0), _heading(0)
		{
		}

		inline Vector(float xc, float yc) : _speed(), _heading()
		{
			xc_yc_to_speed_heading(xc, yc, _speed, _heading);
		}

		inline Vector(const Vector &v) : _speed(v._speed), _heading(v._heading)
		{
		}

		inline Vector &operator=(const Vector &v)
		{
			_speed   = v._speed;
			_heading = v._heading;
			return *this;
		}

		inline ~Vector()
		{
		}

#define VEC_CODE(fname, sign) \
		inline void fname(const Vector &v) \
		{ \
			float xc[2], yc[2]; \
			\
			speed_heading_to_xc_yc(     _speed,        _heading,   xc[0], yc[0]); \
			speed_heading_to_xc_yc(v.get_speed(), v.get_heading(), xc[1], yc[1]); \
			\
			xc_yc_to_speed_heading(xc[0] sign xc[1], yc[0] sign yc[1], _speed, _heading); \
		}

		VEC_CODE(add_vector, +)
		VEC_CODE(mul_vector, *)

		ACCESS(_speed)
		ACCESS(_heading)
};


class Position
{
	protected:
		float _x, _y;

	public:
		inline Position()
			: _x(0), _y(0)
		{
		}

		inline Position(float x, float y)
			: _x(x), _y(y)
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

		inline void apply_vector(const Vector &v)
		{
			float xc, yc;
			speed_heading_to_xc_yc(v.get_speed(), v.get_heading(), xc, yc);
			_x += xc;
			_y += yc;
		}
};

class Mover : public Vector, public Position
{
	protected:
		int _w, _h;

	public:
		inline Mover(int w, int h)
			: _w(w), _h(h)
		{
		}

		inline Mover(float x, float y, float speed, float heading,
				int w, int h)
			: Vector(speed, heading), Position(x, y),
			_w(w), _h(h)
		{
		}

		enum clip_method
		{
			CLIP_BOUNCE, CLIP_STOP, CLIP_NONE
		};

		inline bool clip_generic(float &var, float min, float max, enum clip_method m, const Vector &v_b, const Vector &v_s)
		{
			bool low;

			if((low = var < min) || var > max){
				switch(m){
					case CLIP_NONE:
						return true;

					case CLIP_BOUNCE:
						mul_vector(v_b);
						break;

					case CLIP_STOP:
						mul_vector(v_s);
						break;
				}

				if(low)
					while(var < min)
						var += 0.1;
				else
					while(var > max)
						var -= 0.1;

				return true;
			}
			return false;
		}

		inline bool clip(float x_min, float y_min, float x_max, float y_max, enum clip_method m)
		{
			static const Vector
				v_x_b(-1,  1),
				v_x_s( 0,  1),
				v_y_b( 1, -1),
				v_y_s( 1,  0);

			x_max -= _w;
			y_max -= _h;

			return clip_generic(_x, x_min, x_max, m, v_x_b, v_x_s) |
				clip_generic(_y, y_min, y_max, m, v_y_b, v_y_s);
		}
};

#endif
