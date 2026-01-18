
#include "pico/stdlib.h"
#include "stdio.h"
#include "cstring"

#include "st7735_driver.h"
#include "display.h"
#include "framebuffer.h"

// Endian helpers

void swap_endian(uint16_t* buffer) {

	size_t number_of_pixels = DISPLAY_HEIGHT * DISPLAY_WIDTH;
	for (size_t i = 0; i < number_of_pixels; i++)
	{
		buffer[i] = __builtin_bswap16(buffer[i]);
	}
}

void Framebuffer::init() {

	fill_with_color(0x0000);
	send_to_display();
}
void Framebuffer::swap_buffers() {

	uint16_t* buffer = front_buffer;
	front_buffer = back_buffer;
	back_buffer = buffer;
}
void Framebuffer::send_to_display() {

	swap_endian(front_buffer);
	set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
	uint16_t buffer_size = DISPLAY_WIDTH * DISPLAY_HEIGHT;
	send_data((uint8_t*)front_buffer, buffer_size * 2);
	swap_endian(front_buffer);
}

void Framebuffer::set_pixel(uint16_t x, uint16_t y, uint16_t color) {

	if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
	back_buffer[y * DISPLAY_WIDTH + x] = color;
}
void Framebuffer::fill_with_color(uint16_t color) {

	for (size_t i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
		back_buffer[i] = color;
	}
}

void Framebuffer::draw_line(uint16_t x, uint16_t y, uint16_t width, uint16_t color) {

	for (size_t i = 0; i < width; i++) {
		back_buffer[y * DISPLAY_WIDTH + x + i] = color;
	}
}
void Framebuffer::draw_rectangle(uint16_t y, uint16_t height, uint16_t x, uint16_t width, uint16_t color) {


	if(y > SCREEN_HEIGHT) {
		printf("[ERROR] starting raw out of bound");
		return;
	}
	if(height > SCREEN_HEIGHT - y) {
		printf("[ERROR] number of raws out of bound");
		return;
	}

	size_t end_raw_y = y + height;
	for (size_t i = y; i < end_raw_y; i++) {
		draw_line(x, i, width, color);
	}
}
void Framebuffer::draw_rectangle_memset(uint16_t y, uint16_t height, uint16_t x, uint16_t width, uint16_t color) {

	if(y > SCREEN_HEIGHT) {
		printf("[ERROR] starting raw out of bound");
		return;
	}
	if(height > SCREEN_HEIGHT - y) {
		printf("[ERROR] number of raws out of bound");
		return;
	}

	uint16_t *line = &back_buffer[y * DISPLAY_WIDTH + x];
	for (size_t i = 0; i < width; i++) {
		line[i] = color;
	}

	size_t line_len = width * sizeof(uint16_t);
	uint16_t *dest = line + DISPLAY_WIDTH;
	size_t end_raw_y = y + height;
	for (size_t i = 1; i < end_raw_y; i++) {
		memcpy(dest, line, line_len);
		dest += DISPLAY_WIDTH;
	}
}

void Framebuffer::draw_sprite(uint16_t y, uint16_t height, uint16_t x, uint16_t width, const uint16_t* sprite) {

	if(y > SCREEN_HEIGHT) {
		printf("[ERROR] starting raw out of bound");
		return;
	}
	if(height > SCREEN_HEIGHT - y) {
		printf("[ERROR] number of raws out of bound");
		return;
	}

	for (size_t i = y; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			// if (sprite[(i - y) * width + j] == 0x1FF8) continue;
			// back_buffer[i * DISPLAY_WIDTH + x + j ] = sprite[(i - y) * width + j];

			uint16_t pixel = sprite[i * width + j];
			if (pixel == 0x1FF8) continue;  // Skip transparency
			back_buffer[(y + i) * DISPLAY_WIDTH + x + j] = pixel;
		}
	}
}
void Framebuffer::draw_sprite_alpha(uint16_t y, uint16_t height, uint16_t x, uint16_t width, const SpritePixel* sprite) {

	if(y > SCREEN_HEIGHT) {
		printf("[ERROR] starting raw out of bound");
		return;
	}
	if(height > SCREEN_HEIGHT - y) {
		printf("[ERROR] number of raws out of bound");
		return;
	}

	size_t end_y = y + height;
	for (size_t i = y; i < end_y; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			const SpritePixel& pixel = sprite[(i - y) * width + j];

			// Skip near-transparent pixels
			if (pixel.alpha < 10) continue;

			size_t buffer_index = i * DISPLAY_WIDTH + x + j;

			// Full opaque - just write directly (native little-endian)
			if (pixel.alpha == 255) {
				back_buffer[buffer_index] = pixel.color;
				continue;
			}

			// Alpha blending - native little-endian RGB565 (CPU natural format)
			// RGB565: [RRRRR GGGGGG BBBBB] as uint16_t

			// Extract sprite RGB (simple bit shifts!)
			uint8_t sr = (pixel.color >> 11) & 0x1F;  // R: bits 15-11
			uint8_t sg = (pixel.color >> 5) & 0x3F;   // G: bits 10-5
			uint8_t sb = pixel.color & 0x1F;          // B: bits 4-0

			// Extract background RGB (simple bit shifts!)
			uint16_t bg = back_buffer[buffer_index];
			uint8_t br = (bg >> 11) & 0x1F;
			uint8_t bg_g = (bg >> 5) & 0x3F;
			uint8_t bb = bg & 0x1F;

			// Blend: result = (sprite * alpha + bg * (255 - alpha)) / 255
			uint8_t inv_alpha = 255 - pixel.alpha;
			uint8_t r = (sr * pixel.alpha + br * inv_alpha) / 255;
			uint8_t g = (sg * pixel.alpha + bg_g * inv_alpha) / 255;
			uint8_t b = (sb * pixel.alpha + bb * inv_alpha) / 255;

			// Pack back to RGB565 (simple bit shifts!)
			back_buffer[buffer_index] = (r << 11) | (g << 5) | b;
		}
	}
}

void Framebuffer::draw_line_bresenham(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	if (x0 >= DISPLAY_WIDTH || x1 >= DISPLAY_WIDTH ||
	y0 >= DISPLAY_HEIGHT || y1 >= DISPLAY_HEIGHT) return;

	uint16_t dx = abs(x1 - x0);
	uint16_t dy = abs(y1 - y0);

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;

	if (dx >= dy) {
		// Shallow: x is fast axis
		int y = y0;
		int d = 2*dy - dx;
		for(int x = x0; x != x1; x += sx) {
			back_buffer[y * DISPLAY_WIDTH + x] = color;
			if(d > 0) {
				y += sy;
				d -= 2*dx;
			}
			d += 2*dy;
		}
	} else {
		// Steep: y is fast axis
		int x = x0;
		int d = 2*dx - dy;
		for(int y = y0; y != y1; y += sy)
		{
			back_buffer[y * DISPLAY_WIDTH + x] = color;
			if(d > 0) {
				x += sx;
				d -= 2*dy;
			}
			d += 2*dx;
		}
	}
	back_buffer[y1 * DISPLAY_WIDTH + x1] = color;
}

void Framebuffer::draw_diamond_outline(int center_x, int center_y, int width, int height, uint16_t color) {

	draw_line_bresenham(center_x - width, center_y, center_x, center_y + height, color);
	draw_line_bresenham(center_x - width, center_y, center_x, center_y - height, color);
	draw_line_bresenham(center_x + width, center_y, center_x, center_y + height, color);
	draw_line_bresenham(center_x + width, center_y, center_x, center_y - height, color);
}

void color_test_nobuffer() {
	set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);  // Full screen

	// Black
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0x00);
		send_data_byte(0x00);
	}
	sleep_ms(5000);

	// White
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0xFF);
		send_data_byte(0xFF);
	}
	sleep_ms(5000);

	// Gray
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0x84);
		send_data_byte(0x10);
	}
	sleep_ms(5000);

	// Red
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0xF8);
		send_data_byte(0x00);
	}
	sleep_ms(5000);

	// Green
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0x07);
		send_data_byte(0xE0);
	}
	sleep_ms(5000);

	// Blue
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0x00);
		send_data_byte(0x1F);
	}
	sleep_ms(5000);

	// Yellow
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0xFF);
		send_data_byte(0xE0);
	}
	sleep_ms(5000);

	// Magenta
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0xF8);
		send_data_byte(0x1F);
	}
	sleep_ms(5000);

	// Cyan
	for (int i = 0; i < 20480; i++) {
		send_data_byte(0x07);
		send_data_byte(0xFF);
	}

	sleep_ms(5000);
}
