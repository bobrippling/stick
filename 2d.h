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

static inline float findangle(float xc, float yc)
{
	if(xc > 0)
		return atanf(yc / xc);
	else if(xc < 0)
		return M_PI + atanf(yc / xc);
	else if(yc > 0)
		return  M_PI / 2;
	else
		return -M_PI / 2;
}

static inline float findangle(float x1, float y1, float x2, float y2)
{
	return findangle(x2 - x1, y2 - y1);
}

static inline void xc_yc_to_speed_heading(float xc, float yc, float &speed, float &heading)
{
	speed = sqrtf(xc * xc + yc * yc);
	heading = findangle(xc, yc);
}

static inline void speed_heading_to_xc_yc(float speed, float heading, float &xc, float &yc)
{
	xc = speed * cosf(heading);
	yc = speed * sinf(heading);
}

static inline bool touches(float x_min, float y_min, float x_max, float y_max,
		float _x, float _y, float _w, float _h)
{
	return x_min <= _x + _w &&
					_x    <= x_max   &&
					y_min <= _y + _h &&
					_y    <= y_max;
}

class Vector
{
	protected:
		float _speed, _heading;

	public:
		enum vector_init
		{
			VEC_SPEED_HEADING,
			VEC_XCOMP_YCOMP
		};

		inline Vector() : _speed(0), _heading(0)
		{
		}

		inline Vector(enum vector_init t, float a, float b)
			: _speed(), _heading()
		{
			switch(t){
				case VEC_SPEED_HEADING:
					_speed   = a;
					_heading = b;
					break;

				case VEC_XCOMP_YCOMP:
					xc_yc_to_speed_heading(a, b, _speed, _heading);
					break;
			}
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

		ACCESS(float, _speed)
		ACCESS(float, _heading)
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

		ACCESS(float, _x)
		ACCESS(float, _y)

		inline void apply_vector(const Vector &v)
		{
			float xc, yc;
			speed_heading_to_xc_yc(v.get_speed(), v.get_heading(), xc, yc);
			_x += xc;
			_y += yc;
		}
};

class Box : public Position
{
	protected:
		float _w, _h;

	public:
		inline Box(float x, float y, float w, float h)
			: Position(x, y), _w(w), _h(h)
		{
		}

		ACCESS(float, _w)
		ACCESS(float, _h)

		float get_x2() const { return _x + _w; }
		float get_y2() const { return _y + _h; }
};


static inline bool touches(Box &b,
		float _x, float _y, float _w, float _h)
{
	return touches(b.get_x(), b.get_y(), b.get_x2(), b.get_y2(),
			_x, _y, _w, _h);
}


class Mover : public Vector, public Box
{
	public:
		inline Mover(float x, float y, float speed, float heading, float w, float h)
			: Vector(VEC_SPEED_HEADING, speed, heading), Box(x, y, w, h)
		{
		}

		inline void apply_vector()
		{
			Position::apply_vector(*this);
		}

		enum clip_method
		{
			CLIP_BOUNCE, CLIP_STOP, CLIP_NONE
		};
		enum clip_type
		{
			CLIP_SIDE_LOW, CLIP_SIDE_HIGH, CLIP_SIDE_NONE
		};

		inline enum clip_type intersects(float var, float min, float max)
		{
			if(     var <  min) return CLIP_SIDE_LOW;
			else if(var >= max) return CLIP_SIDE_HIGH;
			else                return CLIP_SIDE_NONE;
		}

		inline bool outside(float x_min, float y_min, float x_max, float y_max)
		{
			if((x_max -= _w) < x_min)
				x_max = x_min;

			if((y_max -= _h) < y_min)
				y_max = y_min;

			return intersects(_x, x_min, x_max) != CLIP_SIDE_NONE ||
						 intersects(_y, y_min, y_max) != CLIP_SIDE_NONE;
		}

		inline bool touches(float x_min, float y_min, float x_max, float y_max)
		{
			return ::touches(x_min, y_min, x_max, y_max, _x, _w, _y, _h);
		}

		inline bool intersects(Box &b)
		{
			return !outside(b.get_x(), b.get_y(), b.get_x2(), b.get_y2());
		}

		inline bool touches(Box &b)
		{
			return touches(b.get_x(), b.get_y(), b.get_x2(), b.get_y2());
		}

		inline bool touches(Box &b, float x, float y, float w, float h)
		{
			return ::touches(b.get_x(), b.get_y(), b.get_x2(), b.get_y2(),
					x, y, w, h);
		}

		inline bool clip_and_adj(float &var, float min, float max,
				enum clip_method m, const Vector &v_b, const Vector &v_s)
		{
			switch(intersects(var, min, max)){
				case CLIP_SIDE_NONE:
					return false;

				case CLIP_SIDE_LOW:
					if(m == CLIP_NONE)
						return true;

					while(var < min)
						var += 0.1;
					break;

				case CLIP_SIDE_HIGH:
					if(m == CLIP_NONE)
						return true;

					while(var > max)
						var -= 0.1;
					break;
			}

			switch(m){
				case CLIP_NONE:
					// unreachable
					break;

				case CLIP_BOUNCE:
					mul_vector(v_b);
					break;

				case CLIP_STOP:
					mul_vector(v_s);
					break;
			}

			return true;
		}


		inline bool clip(float x_min, float y_min, float x_max, float y_max, enum clip_method m)
		{
			static const Vector
				v_x_b(VEC_XCOMP_YCOMP, -1,  1),
				v_x_s(VEC_XCOMP_YCOMP,  0,  1),
				v_y_b(VEC_XCOMP_YCOMP,  1, -1),
				v_y_s(VEC_XCOMP_YCOMP,  1,  0);

			x_max -= _w;
			y_max -= _h;

			return clip_and_adj(_x, x_min, x_max, m, v_x_b, v_x_s) |
						 clip_and_adj(_y, y_min, y_max, m, v_y_b, v_y_s);
		}
};

#endif
