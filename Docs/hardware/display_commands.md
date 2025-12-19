# Display Commands & Communication

## Overview

This document explains how the Raspberry Pi Pico communicates with the ST7735 display using command bytes sent over SPI.

---

## Hexadecimal Notation

Display commands use **hexadecimal (hex)** notation with the `0x` prefix.

### Why Hexadecimal?

**1 byte = 8 bits = 2 hex digits exactly**

```
Decimal:  255
Hex:      0xFF
Binary:   11111111

Decimal:  17
Hex:      0x11
Binary:   00010001
```

### Hex Digits

- **0-9**: Same as decimal
- **A-F**: A=10, B=11, C=12, D=13, E=14, F=15

---

## The Data Flow: Code to Hardware

### Step 1: Your Code

```cpp
send_command(ST7735_SLPOUT);  // ST7735_SLPOUT = 0x11
```

### Step 2: Function Execution

```cpp
void send_command(uint8_t cmd) {
    gpio_put(PIN_CS, 0);      // Select display
    gpio_put(PIN_DC, 0);      // DC LOW = Command mode
    spi_write_blocking(spi0, &cmd, 1);  // Send the byte
    gpio_put(PIN_CS, 1);      // Deselect display
}
```

### Step 3: SPI Hardware

- Converts `0x11` (hex) into 8 electrical pulses
- Clock (SCK) ticks 8 times to synchronize
- Each pulse represents one bit: `00010001`

### Step 4: Display Chip Receives

```
Receives:  00010001 (binary)
Checks DC: LOW (it's a command!)
Decodes:   0x11 = "Sleep Out" command
Executes:  Wakes up from sleep mode
```

---

## Commands vs Data

### Command Byte

Tells the display **what to do**:
```cpp
send_command(ST7735_COLMOD);  // "Set pixel format"
```

### Data Bytes

Tells the display **how to do it** (parameters):
```cpp
send_data_byte(0x05);  // Use RGB565 (16-bit color)
```

### Example Sequence

```cpp
// Command: "I want to set the pixel format"
send_command(ST7735_COLMOD);  // 0x3A

// Data: "Use RGB565 format"
send_data_byte(0x05);  // 0x05 = 16-bit/pixel
```

The display chip knows: "After COLMOD command, next byte is the format parameter."

---

## Common ST7735 Commands

These values come from the **ST7735 datasheet** (chip manufacturer specification).

```cpp
// System Commands
#define ST7735_SWRESET 0x01  // Software Reset
#define ST7735_SLPIN   0x10  // Enter Sleep Mode
#define ST7735_SLPOUT  0x11  // Exit Sleep Mode
#define ST7735_NORON   0x13  // Normal Display Mode

// Display Commands
#define ST7735_INVOFF  0x20  // Display Inversion Off
#define ST7735_INVON   0x21  // Display Inversion On
#define ST7735_DISPOFF 0x28  // Display Off
#define ST7735_DISPON  0x29  // Display On

// Memory Commands
#define ST7735_CASET   0x2A  // Column Address Set
#define ST7735_RASET   0x2B  // Row Address Set
#define ST7735_RAMWR   0x2C  // Memory Write

// Configuration Commands
#define ST7735_MADCTL  0x36  // Memory Access Control (rotation)
#define ST7735_COLMOD  0x3A  // Pixel Format Set
```

---

## Display Initialization Sequence

### Example from `st7735_commands.cpp`

```cpp
void init_display_commands() {
    // 1. Wake up display
    send_command(ST7735_SLPOUT);  // 0x11
    sleep_ms(120);  // CRITICAL: Display needs time to boot

    // 2. Set pixel format
    send_command(ST7735_COLMOD);  // 0x3A = "Set pixel format"
    send_data_byte(0x05);         // 0x05 = RGB565 (16-bit)

    // 3. Set orientation
    send_command(ST7735_MADCTL);  // 0x36 = "Memory access control"
    send_data_byte(0x00);         // 0x00 = normal orientation

    // 4. Configure frame rate
    send_command(ST7735_FRMCTR1); // 0xB1 = "Frame rate control"
    send_data_byte(0x01);         // RTNA parameter
    send_data_byte(0x2C);         // FPA parameter
    send_data_byte(0x2D);         // BPA parameter

    // 5. Normal display mode
    send_command(ST7735_NORON);   // 0x13

    // 6. Turn on display
    send_command(ST7735_DISPON);  // 0x29
    sleep_ms(100);
}
```

### Why This Order?

1. **SLPOUT first**: Display must be awake before configuration
2. **Wait 120ms**: Hardware requirement from datasheet
3. **Configure before enabling**: Set all parameters while display is off
4. **DISPON last**: Turn on after everything is ready

---

## Frame Rate Control Parameters

The **FRMCTR1** (0xB1) command controls the display refresh rate using three timing parameters.

### The Three Parameters: RTNA, FPA, BPA

```cpp
send_command(ST7735_FRMCTR1);  // Frame rate control
send_data_byte(0x01);  // RTNA - Rate in Normal mode
send_data_byte(0x2C);  // FPA - Front Porch
send_data_byte(0x2D);  // BPA - Back Porch
```

### Parameter Explanations

**RTNA (Rate in Normal mode)**
- Controls the **line frequency divider**
- Lower value = faster refresh
- Range: 0x00-0x0F
- `0x01` = moderate speed

**FPA (Front Porch)**
- **Blank time AFTER drawing a line**, before starting the next
- Longer porch = slower frame rate, more stable display
- Range: 0x01-0x3F (1-63 in decimal)
- `0x2C` = 44 in decimal

**BPA (Back Porch)**
- **Blank time BEFORE drawing a line**
- Setup time for display electronics
- Range: 0x01-0x3F (1-63 in decimal)
- `0x2D` = 45 in decimal

### What's a "Porch"?

The display draws line by line, like an old CRT TV:

```
[Back Porch] → Draw Line → [Front Porch] → Next Line
     ↓                           ↓
   Setup time              Reset time
```

**Why needed?**
- Display electronics need time to stabilize between lines
- Too fast = visual artifacts, flickering, corruption
- Too slow = lower frame rate

### Frame Rate Calculation

From the ST7735 datasheet:

```
Frame Rate = Fosc / [(RTNA × 2 + 40) × (LINE + FPA + BPA)]

Where:
  Fosc = ~625 kHz (internal oscillator frequency)
  RTNA = 0x01 = 1
  LINE = 160 (display height in pixels)
  FPA  = 0x2C = 44
  BPA  = 0x2D = 45
```

**Our settings:**
```
Frame Rate = 625000 / [(1×2 + 40) × (160 + 44 + 45)]
           = 625000 / [42 × 249]
           = 625000 / 10458
           ≈ 60 Hz (60 FPS!)
```

### Common Value Sets

```cpp
// High frame rate (80+ FPS) - may be unstable
send_data_byte(0x00);  // RTNA
send_data_byte(0x06);  // FPA
send_data_byte(0x03);  // BPA

// Balanced (60 FPS) - standard setting ← YOU ARE HERE
send_data_byte(0x01);  // RTNA
send_data_byte(0x2C);  // FPA
send_data_byte(0x2D);  // BPA

// Conservative (40-50 FPS) - very stable
send_data_byte(0x05);  // RTNA
send_data_byte(0x3C);  // FPA
send_data_byte(0x3C);  // BPA
```

**The current values (0x01, 0x2C, 0x2D) are standard settings that achieve ~60 Hz**, perfectly matching the engine's 60 FPS target!

---

## The Hardware API Analogy

Think of display commands like a **hardware API**:

| Software API | Hardware API |
|--------------|--------------|
| `setPixelFormat(RGB565)` | `send_command(0x3A); send_data_byte(0x05);` |
| `display.turnOn()` | `send_command(0x29);` |
| `display.sleep()` | `send_command(0x10);` |

The chip designer chose these command codes, and we use them to control the display.

---

## Different Displays, Different Commands

### Same Family (Similar Commands)

**ST7735** and **ST7789** (both from Sitronix) share many commands:
```cpp
// These are the same for ST7735 and ST7789
#define SLPOUT  0x11  // Sleep Out
#define DISPON  0x29  // Display On
#define RAMWR   0x2C  // Memory Write
```

But they have **different initialization sequences** and parameters!

### Different Manufacturer (Different Commands)

**ILI9341** (from ILITek) uses different codes:
```cpp
// ILI9341-specific
#define ILI9341_PWCTR1  0xC0  // Different power control
#define ILI9341_VMCTR1  0xC5  // VCOM control (ST7735 doesn't have)
```

---

## Why This Architecture Works

```
Display-agnostic code (engine, game)
           ↓
      display.h (public API)
           ↓
   display_internal.h (SPI helpers)
           ↓
  st7735_commands.cpp (ST7735-specific codes)
  st7789_commands.cpp (ST7789-specific codes)
```

**SPI communication is universal**, but **command codes and sequences differ per chip**.

---

## Reading the Datasheet

The **ST7735 datasheet** is your reference manual. For each command it lists:

- **Command code** (hex value)
- **Parameters** (what data bytes to send)
- **Timing requirements** (delays needed)
- **Valid values** (parameter ranges)

Example entry:
```
Command: COLMOD (0x3A)
Parameters: 1 byte
  0x03 = 12-bit/pixel
  0x05 = 16-bit/pixel (RGB565)
  0x06 = 18-bit/pixel
```

---

## See Also

- **[SPI Communication](spi.md)** - How SPI protocol works
- **[RGB565 Color Format](rgb565.md)** - Pixel format details
- **[Display Pins](display_pins.md)** - Pin connections

---

## Code References

- `drivers/display.cpp` - SPI communication functions
- `drivers/st7735_commands.h` - Command definitions
- `drivers/st7735_commands.cpp` - Initialization sequence
