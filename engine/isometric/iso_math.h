#ifndef ISO_MATH_H
#define ISO_MATH_H

#include "pico/stdlib.h"
#include "fixed_point.h"

struct World_space {
	// z - up; y - north, x - east
	Fixed_q16 x = 0;
	Fixed_q16 y = 0;
	Fixed_q16 z = 0;
};


struct Screen_space {
	int x = 0;
	int y = 0;
};

struct Camera {
	int offset_x = 0;
	int offset_y = 0;
};

inline Screen_space world_to_screen(const World_space& world, const Camera& cam) {

	Fixed_q16 sx = world.x - world.y;
	Fixed_q16 sy = (world.x + world.y) / 2 - world.z;

	Screen_space screen;
	screen.x = sx.to_int() + cam.offset_x;
	screen.y = sy.to_int() + cam.offset_y;
	return screen;
}

// Common case - ground level
inline World_space screen_to_world(const Screen_space& screen, const Camera& cam) {

	Fixed_q16 sx = Fixed_q16(screen.x - cam.offset_x);
	Fixed_q16 sy = Fixed_q16(screen.y - cam.offset_y);

	World_space world;  // z is already 0 from default constructor
	world.x = sx / 2 + sy;
	world.y = sy - sx / 2;
	return world;
}

// Rare case - specific z level (elevated platforms, multi-floor buildings)
inline World_space screen_to_world(const Screen_space& screen, const Camera& cam,
Fixed_q16 z);


#endif
