#include "pico/stdlib.h"
#include "display.h"
#include "display_internal.h"
#include "st7735_commands.h"

void init_display_commands() {
	// Wake up display
	send_command(ST7735_SLPOUT);
	sleep_ms(120);

	// Set pixel format to RGB565 (16-bit color)
	send_command(ST7735_COLMOD);
	send_data_byte(0x05);  // 0x05 = 16-bit/pixel (RGB565)

	// Set screen orientation (optional, depends on your physical setup)
	send_command(ST7735_MADCTL);
	send_data_byte(0x00);  // 0x00 = normal, or try 0xC0, 0x60, 0xA0 for rotation

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
