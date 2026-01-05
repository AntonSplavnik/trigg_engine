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
	for (size_t i = y; i < end_raw_y; i++) {
		memcpy(dest, line, line_len);
		dest += DISPLAY_WIDTH;
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
