#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

//Display pins
#define PIN_SCK     2             // SPI Clock
#define PIN_MOSI    3             // SPI Data Out
#define PIN_MISO    19            // SPI Data In (SD card only, not needed for display)
#define PIN_BL      17            // Backlight (LITE)
#define PIN_TFT_CS  20            // Display Chip Select
#define PIN_CARD_CS 21            // SD Card Chip Select
#define PIN_DC      22            // Data/Command (D/C)
#define PIN_RESET   26            // Display Reset

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

#endif
