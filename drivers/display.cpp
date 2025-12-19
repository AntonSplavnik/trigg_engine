#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware_config.h"
#include "display.h"

void init_display(){

	// Initialize SPI at 62.5 MHz
	spi_init(spi0, 62500000);
	// Set SPI format (8 bits, mode 0)
	spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	// Assign pins to SPI function
	gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

	// Initialize Control Pins (CS, DC, RESET, BL)
	// Initialize CS (Chip Select)
	init_pin(PIN_TFT_CS, GPIO_OUT, 1); // Start HIGH (not selected)
	// Initialize DC (Data/Command)
	init_pin(PIN_DC, GPIO_OUT, 0);
	// Initialize RESET
	init_pin(PIN_RESET, GPIO_OUT, 1); // Start HIGH (not in reset)
	// Initialize Backlight
	init_pin(PIN_BL, GPIO_OUT, 1); // Turn backlight ON

	// Hardware reset sequence
	reset_display();
}
