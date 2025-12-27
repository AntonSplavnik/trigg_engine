#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "drivers/display.h"
#include "stdlib.h"
#include "random"

const uint LED_PIN = 25;
const uint LED_L = 28;
const uint LED_R = 4;

static std::mt19937 rng(time_us_32());
int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

void init_led_pins_as_GPIO() {

	// Pico onboard LED
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	// Trigg Led_left
	gpio_init(LED_L);
	gpio_set_dir(LED_L, GPIO_OUT);

	// Trigg Led_right
	gpio_init(LED_R);
	gpio_set_dir(LED_R, GPIO_OUT);
}

void blik(){

	init_led_pins_as_GPIO();

	while (true) {
		gpio_put(LED_PIN, 1);
		printf("LED ON!\n");
		sleep_ms(randomInt(0, 250));
		gpio_put(LED_PIN, 0);
		printf("LED OFF!\n");

		gpio_put(LED_L, 1);
		sleep_ms(randomInt(0, 250));
		gpio_put(LED_L, 0);

		gpio_put(LED_R, 1);
		sleep_ms(randomInt(0, 250));
		gpio_put(LED_R, 0);
	}
}

int main(){
	stdio_init_all();
	sleep_ms(1000);
	printf("TriggEngine v0.1\n");
	init_display();
	blik();
	return 0;
}
