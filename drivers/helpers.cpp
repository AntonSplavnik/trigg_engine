#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware_config.h"
#include "display.h"

void send_command(uint8_t cmd) {
	gpio_put(PIN_TFT_CS, 0);
	gpio_put(PIN_DC, 0);
	spi_write_blocking(spi0, &cmd, 0);
	gpio_put(PIN_TFT_CS, 1);
}

void send_data(uint8_t* data, size_t len) {
	gpio_put(PIN_TFT_CS, 0);
	gpio_put(PIN_DC, 1);
	spi_write_blocking(spi0, data, len);
	gpio_put(PIN_TFT_CS, 1);
}

void send_data_byte(uint8_t data) {
	gpio_put(PIN_TFT_CS, 0);
	gpio_put(PIN_DC, 1);
	spi_write_blocking(spi0, &data, 1);
	gpio_put(PIN_TFT_CS, 1);
}

void init_pin(int pin, enum gpio_dir GPIO, int voltage) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO);
	gpio_put(pin, voltage);  // Start HIGH (not selected)
}

void reset_display() {
	gpio_put(PIN_RESET, 0);  // Pull RESET low
	sleep_ms(10);            // Wait 10ms
	gpio_put(PIN_RESET, 1);  // Pull RESET high
	sleep_ms(100);           // Wait for display to boot
}
