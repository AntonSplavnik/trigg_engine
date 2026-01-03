#include "pico/stdlib.h"
#include "hardware_config.h"
#include "buttons.h"

void init_button(int pin, enum gpio_dir gpio) {
	gpio_init(pin);
	gpio_set_dir(pin, gpio);
	gpio_pull_up(pin);
}

void Buttons::init_buttons_pins() {

	// Left side (wasd)
	init_button(BTN_W, GPIO_IN);
	init_button(BTN_A, GPIO_IN);
	init_button(BTN_S, GPIO_IN);
	init_button(BTN_D, GPIO_IN);

	// Right side (ijkl)
	init_button(BTN_I, GPIO_IN);
	init_button(BTN_J, GPIO_IN);
	init_button(BTN_K, GPIO_IN);
	init_button(BTN_L, GPIO_IN);
}



ButtonState Buttons::button_polling( ) {
	ButtonState state;
	state.w = !gpio_get(BTN_W);
	state.a = !gpio_get(BTN_A);
	state.s = !gpio_get(BTN_S);
	state.d = !gpio_get(BTN_D);
	state.i = !gpio_get(BTN_I);
	state.j = !gpio_get(BTN_J);
	state.k = !gpio_get(BTN_K);
	state.l = !gpio_get(BTN_L);
	return state;
}
