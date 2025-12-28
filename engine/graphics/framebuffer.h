#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "display.h"

namespace Framebuffer {
	static uint16_t framebuffer[DISPLAY_HEIGHT * DISPLAY_WIDTH];

	void init();
	void clear();
	void set_pixel(uint16_t x, uint16_t y, uint16_t color);
	void send_to_display();
};

#endif
