#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "display.h"

namespace Framebuffer {
	static uint16_t framebuffer_0[DISPLAY_HEIGHT * DISPLAY_WIDTH];
	static uint16_t framebuffer_1[DISPLAY_HEIGHT * DISPLAY_WIDTH];

	static uint16_t* front_buffer = framebuffer_0;
	static uint16_t* back_buffer = framebuffer_1;

	void init();
	void swap_buffers();
	void set_pixel(uint16_t x, uint16_t y, uint16_t color);
	void send_to_display();

	void fill_with_color(uint16_t color);
	void draw_line(uint16_t x, uint16_t y, uint16_t line_len, uint16_t color);
	void draw_rectangle(uint16_t start_raw_y, uint16_t number_of_raws_y, uint16_t x, uint16_t line_len, uint16_t color);
	void draw_rectangle_memset(uint16_t start_raw_y, uint16_t number_of_raws_y, uint16_t x, uint16_t line_len, uint16_t color);
};

#endif
