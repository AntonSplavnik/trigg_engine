#ifndef PLATFORM_DESKTOP_H
#define PLATFORM_DESKTOP_H

#include <cstdint>

// Screen dimensions (same as stm32 hardware)
#define SCREEN_HEIGHT 820
#define SCREEN_WIDTH 1480

constexpr uint16_t DISPLAY_HEIGHT = SCREEN_HEIGHT;
constexpr uint16_t DISPLAY_WIDTH = SCREEN_WIDTH;

#endif
