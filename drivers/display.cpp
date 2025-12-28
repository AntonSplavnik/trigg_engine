#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

#include "hardware_config.h"
#include "display_internal.h"
#include "spi.h"
#include "display.h"

const uint16_t DISPLAY_WIDTH = SCREEN_WIDTH;
const uint16_t DISPLAY_HEIGHT = SCREEN_HEIGHT;

// Low-level SPI communication helpers
void send_command(uint8_t cmd) {
	gpio_put(PIN_TFT_CS, 0);
	gpio_put(PIN_DC, 0);
	spi_write_blocking(spi0, &cmd, 1);
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

// GPIO helper
void init_pin(int pin, enum gpio_dir GPIO, int voltage) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO);
	gpio_put(pin, voltage);
}

void init_backlight_pwm() {
	// Tell GPIO 17 it's controlled by PWM
	gpio_set_function(PIN_BL, GPIO_FUNC_PWM);

	// Find PWM slice for GPIO 17
	uint slice_num = pwm_gpio_to_slice_num(PIN_BL);

	// Set PWM frequency (1 kHz is good for backlights)
	pwm_set_wrap(slice_num, 999);  // TOP value (0-999 = 1000 steps)

	// Clock divider
	pwm_set_clkdiv(slice_num, 125.0f); // 1 kHz frequency

	// Set initial brightness (50% = 500)
	pwm_set_gpio_level(PIN_BL, 0);

	// Enable PWM slice
	pwm_set_enabled(slice_num, true);
}
void set_brightness_level(uint16_t level) {
	// level: 0 (off) to 999 (max brightness)
	if(level > 999) level = 999;
	pwm_set_gpio_level(PIN_BL, level);
}
void set_brightness_percent(uint16_t percent) {
	if(percent > 100) percent = 100;
	set_brightness_level((percent * 999) / 100);
}

void init_display_pins() {
	// Control pins as GPIO
	init_pin(PIN_TFT_CS, GPIO_OUT, 1);  // Initialize CS (Chip Select) Start HIGH (not selected)
	init_pin(PIN_DC, GPIO_OUT, 0);     // Initialize DC (Data/Command)
	init_pin(PIN_RESET, GPIO_OUT, 1); // Initialize RESET - Start HIGH (not in reset)

	// Backlight as PWM
	init_backlight_pwm();
}

// Hardware reset sequence
void reset_display() {
	gpio_put(PIN_RESET, 0);  // Pull RESET low
	sleep_ms(10);            // Wait 10ms
	gpio_put(PIN_RESET, 1);  // Pull RESET high
	sleep_ms(120);           // Wait for display to boot
}

// Main display initialization
void init_display(){
	init_SPI_bus();
	init_SPI_pins();
	init_display_pins();
	reset_display();
	init_display_commands();
	set_brightness_level(500);
}
