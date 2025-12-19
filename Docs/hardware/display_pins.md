# Display Pin Descriptions

## Pin Configuration

The ST7735 display connects to the Pico using the following pins:

```cpp
#define PIN_SCK     2      // SPI Clock
#define PIN_MOSI    3      // SPI Data Out
#define PIN_MISO    19     // SPI Data In (SD card only)
#define PIN_BL      17     // Backlight (LITE) - was incorrectly 18
#define PIN_TFT_CS  20     // Display Chip Select
#define PIN_CARD_CS 21     // SD Card Chip Select
#define PIN_DC      22     // Data/Command
#define PIN_RESET   26     // Display Reset
```

---

## Sprig Console Pin Voltage Reference

All pins configured as OUTPUT unless noted otherwise.

| GPIO | Signal | HIGH (1 / 3.3V) | LOW (0 / 0V) | Notes |
|------|--------|-----------------|--------------|-------|
| **2** | **SCK** | Clock pulse high | Clock pulse low | SPI hardware controlled |
| **3** | **MOSI** | Data bit = 1 | Data bit = 0 | SPI hardware controlled |
| **19** | **MISO** | Data bit = 1 | Data bit = 0 | SPI hardware controlled (SD card) |
| **17** | **BL (LITE)** | Backlight ON | Backlight OFF | Display LED backlight |
| **20** | **TFT_CS** | Display IGNORES SPI | Display LISTENS to SPI | Active-low chip select |
| **21** | **CARD_CS** | SD card IGNORES SPI | SD card LISTENS to SPI | Active-low chip select |
| **22** | **DC** | Data mode (pixels) | Command mode (instructions) | Display data/command select |
| **26** | **RESET** | Normal operation | Display in RESET | Active-low reset |

### Active-Low vs Active-High Signals

**Active-Low Signals** (inverted logic - LOW = active):
- **CS pins (20, 21)**: Must be LOW to select device
- **RESET (26)**: Must pulse LOW to reset display

**Active-High Signals** (normal logic - HIGH = active):
- **Backlight (17)**: HIGH = backlight on
- **DC (22)**: HIGH = sending pixel data, LOW = sending commands

### Critical: SPI Bus Sharing

The display and SD card **share the same SPI bus** (SCK, MOSI, MISO). Only CS pins are separate.

**NEVER select both devices simultaneously:**
```cpp
// WRONG - Both devices selected simultaneously!
gpio_put(PIN_TFT_CS, 0);   // Display ON
gpio_put(PIN_CARD_CS, 0);  // SD card ON - BUS CONFLICT!
// This causes data corruption and unpredictable behavior!
```

**CORRECT - Only one device at a time:**
```cpp
// Talk to display
gpio_put(PIN_CARD_CS, 1);  // SD card OFF
gpio_put(PIN_TFT_CS, 0);   // Display ON
// ... send display data via SPI ...
gpio_put(PIN_TFT_CS, 1);   // Display OFF

// Talk to SD card
gpio_put(PIN_TFT_CS, 1);   // Display OFF
gpio_put(PIN_CARD_CS, 0);  // SD card ON
// ... read SD card data via SPI ...
gpio_put(PIN_CARD_CS, 1);  // SD card OFF
```

**Why this matters:**
- Multiple devices on one SPI bus is normal and efficient
- CS (Chip Select) tells each device "listen" or "ignore"
- If both listen simultaneously, they both try to respond → data corruption
- Always deselect one before selecting the other

---

## Pin Details

### MOSI (GPIO 3) - Master Out Slave In
- **Purpose:** Pico sends DATA to display
- **Data sent:** Pixel colors, commands, parameters
- **Direction:** Pico → Display (one-way)
- **Example:** Sending the command "fill screen red"

### SCK (GPIO 2) - Serial Clock
- **Purpose:** Timing signal (like a metronome)
- **How it works:** Data changes on each clock pulse
- **Speed:** 62.5 MHz on Trigg console
- **Analogy:** Like a conductor's baton keeping musicians in sync

### CS (GPIO 20) - Chip Select
- **Purpose:** Tells the display "I'm talking to YOU now"
- **LOW (0):** Display listens and processes data
- **HIGH (1):** Display ignores the SPI bus
- **Why needed:** Multiple devices can share the same SPI bus

### DC (GPIO 22) - Data/Command
- **Purpose:** Tells display what type of information you're sending
- **LOW (0):** Command mode (instructions like "clear screen", "set brightness")
- **HIGH (1):** Data mode (actual pixel colors, coordinates)
- **Critical:** Display needs to know if you're giving orders or sending pixels

### RST (GPIO 26) - Reset
- **Purpose:** Restarts the display chip (like rebooting a computer)
- **How to use:** Pulse LOW then HIGH
- **When to use:** At startup, or if display gets stuck
- **Timing:** Hold LOW for 10ms, then HIGH and wait 100ms

### BL (GPIO 17) - Backlight (LITE)
- **Purpose:** Controls screen brightness
- **HIGH (1):** Backlight ON (screen visible)
- **LOW (0):** Backlight OFF (screen dark, saves power)
- **Note:** Display still works when backlight is off, you just can't see it
- **Schematic:** Connected to LITE pin on ST7735 display module
- **Important:** Common mistake - GPIO18 is NOT the backlight pin!

### MISO (GPIO 19) - Master In Slave Out
- **Purpose:** Receive data FROM display (not used for ST7735 drawing)
- **Used for:** SD card communication on Trigg console
- **Display drawing:** You only need MOSI (one-way communication)

---

## Display Initialization Sequence

The ST7735 display needs to be told how to operate:

1. **Wake from sleep** - Display starts in low-power mode
2. **Set pixel format** - Tell it we're using RGB565 (16-bit color)
3. **Configure orientation** - Portrait vs landscape, mirroring
4. **Define frame rate** - How fast to refresh (60Hz typical)
5. **Turn on display** - Enable the actual screen output

Without these commands, the display won't show anything even if backlight is on.
