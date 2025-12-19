#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "drivers/display.h"
#include "stdlib.h"
#include "random"

static std::mt19937 rng(time_us_32());

int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

void blik(){

	const uint LED_PIN = 25;  // Onboard LED
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);


	while (true) {
		gpio_put(LED_PIN, 1);
		printf("LED ON!\n");
		sleep_ms(randomInt(0, 250));
		gpio_put(LED_PIN, 0);
		printf("LED OFF!\n");
		sleep_ms(randomInt(0, 250));
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
