#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "display.h"

// Sprite pixel format for alpha blending
struct SpritePixel {
	uint16_t color;  // RGB565
	uint8_t alpha;   // 0-255
};

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
	void draw_sprite(uint16_t start_raw_y, uint16_t number_of_raws_y, uint16_t x, uint16_t line_len, const uint16_t* sprite);
	void draw_sprite_alpha(uint16_t start_raw_y, uint16_t number_of_raws_y, uint16_t x, uint16_t line_len, const SpritePixel* sprite);
	void draw_tile_32x32();
	void draw_line_bresenham(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

};


#endif
