#include "pico/stdlib.h"

#include "display.h"
#include "display_internal.h"
#include "hardware_config.h"
#include "st7735_driver.h"

void init_display_commands() {

	// Software reset first
	send_command(ST7735_SWRESET);
	sleep_ms(150);

	// Wake up display
	send_command(ST7735_SLPOUT);
	sleep_ms(120);

	// Set pixel format to RGB565 (16-bit color)
	send_command(ST7735_COLMOD);
	send_data_byte(0x05);  // 0x05 = 16-bit/pixel (RGB565)

	// Set screen orientation (optional, depends on your physical setup)
	send_command(ST7735_MADCTL);
	send_data_byte(0x60);  // 0x00 = normal, or try 0xC0 (90°), 0x60 (180°), 0xA0 (270°) for rotation

	// Set frame rate
	send_command(ST7735_FRMCTR1);
	send_data_byte(0x01);  // RTNA
	send_data_byte(0x2C);  // FPA
	send_data_byte(0x2D);  // BPA

	// Normal display mode
	send_command(ST7735_NORON);

	// Turn on display
	send_command(ST7735_DISPON);
	sleep_ms(100);
}

// Sets rectangular drawing window from (x1,y1) to (x2,y2)
// Each 16-bit coord split into high/low bytes for SPI
// Flow: CASET (X range) → RASET (Y range) → RAMWR (ready for pixel data)
void set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {

	send_command(ST7735_CASET);
	send_data_byte(x1 >> 8);
	send_data_byte(x1 & 0xFF);
	send_data_byte(x2 >> 8);
	send_data_byte(x2 & 0xFF);

	send_command(ST7735_RASET);
	send_data_byte(y1 >> 8);
	send_data_byte(y1 & 0xFF);
	send_data_byte(y2 >> 8);
	send_data_byte(y2 & 0xFF);

	send_command(ST7735_RAMWR);
}

void display_toggle_test() {
	// Turn display off
	send_command(ST7735_DISPOFF);
	sleep_ms(1000);

	// Turn display on
	send_command(ST7735_DISPON);
	sleep_ms(1000);

	// Repeat 3 times
	for (int i = 0; i < 3; i++) {
		send_command(ST7735_DISPOFF);
		sleep_ms(500);
		send_command(ST7735_DISPON);
		sleep_ms(500);
	}
}
