#include "pico/stdlib.h"

#include "st7735_driver.h"
#include "display.h"
#include "framebuffer.h"

void init() {
	clear(0x0000);
}
void clear(uint16_t color) {
	for (size_t i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++)
	{
		Framebuffer::framebuffer[i] = color;
	}

}
void fill_framebuffer(uint16_t* framebuffer, uint16_t buffer_size, uint16_t color) {
	for (size_t i = 0; i < buffer_size; i++)
	{
		framebuffer[i] = color;
		framebuffer[i] = (color << 8) | (color >> 8);
	}
}

void set_pixel(uint16_t x, uint16_t y, uint16_t color) {
	if(x >= 128 || y >= 160) return;
	framebuffer[y * DISPLAY_WIDTH + x] = color;
}

void framebuffer_test() {
	set_window(0, 0, 127, 159);
	uint16_t buffer_size = DISPLAY_HEIGHT * DISPLAY_WIDTH;
	uint16_t* framebuffer = new uint16_t[buffer_size];
	fill_framebuffer(framebuffer, buffer_size, 0xF800);
	send_data((uint8_t*)framebuffer, buffer_size * 2);
}

void color_test() {
	set_window(0, 0, 127, 159);  // Full screen

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
