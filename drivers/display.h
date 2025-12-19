#ifndef ST7735_DRIVER_H
#define ST7735_DRIVER_H

#include "stdlib.h"

void init_display();
void send_command(uint8_t cmd);
void send_data(uint8_t* data, size_t len);
void send_data_byte(uint8_t data);
void init_pin(int pin, enum gpio_dir GPIO, int voltage);
void reset_display();

#endif
