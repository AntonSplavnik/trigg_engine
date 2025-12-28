#ifndef DISPLAY_INTERNAL_H
#define DISPLAY_INTERNAL_H

#include "stdlib.h"

void send_command(uint8_t cmd);
void send_data(uint8_t* data, size_t len);
void send_data_byte(uint8_t data);

void init_display_commands();
void display_toggle_test();
void color_test();
void framebuffer_test();

#endif
