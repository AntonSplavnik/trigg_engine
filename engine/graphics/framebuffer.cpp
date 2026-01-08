#include "pico/stdlib.h"
#include "stdio.h"
#include "cstring"

#include "st7735_driver.h"
#include "display.h"
#include "framebuffer.h"

void Framebuffer::init() {

	fill_with_color(0x0000);
	send_to_display();
}

void Framebuffer::swap_buffers() {

	uint16_t* buffer = front_buffer;
	front_buffer = back_buffer;
	back_buffer = buffer;
}

void Framebuffer::fill_with_color(uint16_t color) {

	uint16_t swapped_color = (color << 8) | (color >> 8);
	for (size_t i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
		back_buffer[i] = swapped_color;
	}
}

void Framebuffer::set_pixel(uint16_t x, uint16_t y, uint16_t color) {

	if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
	back_buffer[y * DISPLAY_WIDTH + x] = (color << 8) | (color >> 8);
}

void Framebuffer::draw_line(uint16_t x, uint16_t y, uint16_t width, uint16_t color) {
	uint16_t swapped_color = (color << 8) | (color >> 8);
	for (size_t i = 0; i < width; i++) {
		back_buffer[y * DISPLAY_WIDTH + x + i] = swapped_color;
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

	uint swapped_color = (color << 8) | (color >> 8);
	uint16_t *line = &back_buffer[y * DISPLAY_WIDTH + x];
	for (size_t i = 0; i < width; i++) {
		line[i] = swapped_color;
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

	size_t end_y = y + height;
	for (size_t i = y; i < end_y; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			if (sprite[(i - y) * width + j] == 0x1FF8) continue;
			back_buffer[i * DISPLAY_WIDTH + x + j ] = sprite[(i - y) * width + j];
			// uint16_t pixel = sprite[(i - y) * width + j];
			// if (pixel == 0x1FF8) continue;  // Skip transparency
			// back_buffer[i * DISPLAY_WIDTH + x + j] = (pixel << 8) | (pixel >> 8);
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

			// Full opaque - just write directly (with byte swap)
			if (pixel.alpha == 255) {
				back_buffer[buffer_index] = (pixel.color << 8) | (pixel.color >> 8);
				continue;
			}

			// Alpha blending required
			uint16_t bg_swapped = back_buffer[buffer_index];
			uint16_t bg = (bg_swapped << 8) | (bg_swapped >> 8);  // Un-swap background

			// Extract RGB components (sprite and background)
			uint8_t sr = (pixel.color >> 11) & 0x1F;
			uint8_t sg = (pixel.color >> 5) & 0x3F;
			uint8_t sb = pixel.color & 0x1F;

			uint8_t br = (bg >> 11) & 0x1F;
			uint8_t bg_g = (bg >> 5) & 0x3F;
			uint8_t bb = bg & 0x1F;

			// Blend: result = (sprite * alpha + bg * (255 - alpha)) / 255
			uint8_t inv_alpha = 255 - pixel.alpha;
			uint8_t r = (sr * pixel.alpha + br * inv_alpha) / 255;
			uint8_t g = (sg * pixel.alpha + bg_g * inv_alpha) / 255;
			uint8_t b = (sb * pixel.alpha + bb * inv_alpha) / 255;

			// Pack to RGB565 and swap bytes for display
			uint16_t blended = (r << 11) | (g << 5) | b;
			back_buffer[buffer_index] = (blended << 8) | (blended >> 8);
		}
	}
}

void Framebuffer::send_to_display() {

	set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
	uint16_t buffer_size = DISPLAY_WIDTH * DISPLAY_HEIGHT;
	send_data((uint8_t*)front_buffer, buffer_size * 2);
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
