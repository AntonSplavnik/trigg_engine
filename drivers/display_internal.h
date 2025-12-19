#ifndef DISPLAY_INTERNAL_H
#define DISPLAY_INTERNAL_H

#include "stdlib.h"

void init_display_commands();
void send_command(uint8_t cmd);
void send_data(uint8_t* data, size_t len);
void send_data_byte(uint8_t data);

#endif
