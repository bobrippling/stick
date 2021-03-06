#ifndef CONFIG_H
#define CONFIG_H

#define CONF_PORT "2848"
#define CONF_MAX_STICKS  128
#define CONF_MAX_BULLETS 1024

#define CONF_NPLATFORMS  4
#define CONF_PLAT_MAX_W  500
#define CONF_PLAT_MIN_W  100
#define CONF_PLAT_H       25

#define CONF_SCALE_SPEED 0.5

#define CONF_WIDTH  800
#define CONF_HEIGHT 600

#define CONF_BULLET_LEN   20
#define CONF_BULLET_DELAY 200
#define CONF_BULLET_SPEED 20.0f

#define CONF_FEET_H           10
#define CONF_STICK_JUMP_DELAY 50
#define CONF_JUMP_VECTOR (Vector::VEC_XCOMP_YCOMP, 0.0f, -20.0f)

#define CONF_GRAVITY (Vector::VEC_XCOMP_YCOMP, 0.0f, 2.0f)

#endif
