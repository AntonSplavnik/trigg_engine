# Pico SDK Reference

## GPIO (General Purpose Input/Output)

```cpp
#include "hardware/gpio.h"

gpio_init(pin)                    // Initialize pin for use
gpio_set_dir(pin, GPIO_OUT)       // Set as output (Pico controls)
gpio_set_dir(pin, GPIO_IN)        // Set as input (read buttons)
gpio_put(pin, 1)                  // Set HIGH (3.3V, ON)
gpio_put(pin, 0)                  // Set LOW (0V, OFF)
gpio_get(pin)                     // Read pin value (returns 0 or 1)
gpio_pull_up(pin)                 // Enable internal pull-up resistor
gpio_pull_down(pin)               // Enable internal pull-down resistor
```

## Timing

```cpp
#include "pico/stdlib.h"

sleep_ms(250)                     // Pause for 250 milliseconds
sleep_us(100)                     // Pause for 100 microseconds
time_us_32()                      // Get current time in microseconds (for seeding RNG)
```

## USB/Serial Communication (stdio)

```cpp
#include "pico/stdlib.h"

stdio_init_all()                  // Initialize USB/UART for printf/scanf (call once at start)
printf("text %d\n", value)        // Print to USB serial (requires stdio_init_all)
```

**Usage:**
```cpp
int main() {
    stdio_init_all();             // Enable USB communication
    sleep_ms(1000);               // Wait for USB enumeration
    printf("Hello from Pico!\n"); // Debug output
}
```

**CMakeLists.txt requirement:** `pico_enable_stdio_usb(${PROJECT_NAME} 1)`
**View output:** `screen /dev/cu.usbmodem* 115200` (Exit: Ctrl+A then K)
**Note:** Enables `picotool -f` remote flashing

## Constants

```cpp
const uint LED_PIN = 25;          // Onboard LED pin number
GPIO_OUT                          // Output direction constant
GPIO_IN                           // Input direction constant
```

## Trigg Console Hardware Pins

### ST7735 Display (128x160)
```cpp
#define PIN_SCK     2             // SPI Clock
#define PIN_MOSI    3             // SPI Data Out
#define PIN_MISO    19            // SPI Data In (SD card only, not needed for display)
#define PIN_BL      17            // Backlight (LITE)
#define PIN_TFT_CS  20            // Display Chip Select
#define PIN_CARD_CS 21            // SD Card Chip Select
#define PIN_DC      22            // Data/Command (D/C)
#define PIN_RESET   26            // Display Reset
```

## SPI Functions

```cpp
#include "hardware/spi.h"

spi_init(spi0, baudrate)                    // Initialize SPI (baudrate in Hz, e.g., 62500000)
spi_set_format(spi0, bits, cpol, cpha, order) // Set SPI format (8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST)
gpio_set_function(pin, GPIO_FUNC_SPI)       // Assign pin to SPI hardware
spi_write_blocking(spi0, data, len)         // Send data array (blocking until complete)
```

## ST7735 Display Commands

```cpp
// Initialization Commands
0x01  // SWRESET  - Software Reset
0x11  // SLPOUT   - Sleep Out (wake up display)
0x13  // NORON    - Normal Display Mode On
0x20  // INVOFF   - Display Inversion Off
0x21  // INVON    - Display Inversion On
0x28  // DISPOFF  - Display Off
0x29  // DISPON   - Display On

// Configuration Commands
0x36  // MADCTL   - Memory Access Control (screen orientation)
0x3A  // COLMOD   - Color Mode (pixel format: 0x05 = RGB565)
0xB1  // FRMCTR1  - Frame Rate Control

// Drawing Commands
0x2A  // CASET    - Column Address Set (X coordinates)
0x2B  // RASET    - Row Address Set (Y coordinates)
0x2C  // RAMWR    - Memory Write (start sending pixel data)
```

---

*This file will be updated as we learn more functions*
