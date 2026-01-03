#ifndef BUTTONS_H
#define BUTTONS_H

struct ButtonState {
	bool w, a, s, d;
	bool i, j, k, l;
};

namespace Buttons {

	void init_buttons_pins();
	ButtonState button_polling();
}

#endif
