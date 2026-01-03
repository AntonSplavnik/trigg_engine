# GPIO (General Purpose Input/Output)

**What is GPIO?**

GPIO stands for **General Purpose Input/Output**. These are configurable electrical pins on the Raspberry Pi Pico that can be programmed to either send signals (output) or receive signals (input).

Think of GPIO pins as Swiss Army knife pins - they're "general purpose" because you decide what they do in your code.

---

## How GPIO Works

### The Two Modes

Every GPIO pin can be configured as either:

**OUTPUT Mode** - Pico controls the pin
- The Pico sets the voltage level (HIGH or LOW)
- Used to control external devices: LEDs, buzzers, chip select signals
- Example: Turning on the display backlight

**INPUT Mode** - Pico reads the pin
- The Pico measures the voltage level from an external source
- Used to read buttons, sensors, or signals from other chips
- Example: Reading button presses from the Sprig controller

---

## Voltage Levels

The Raspberry Pi Pico uses **3.3V logic** (unlike Arduino's 5V):

| State | Voltage | Binary | Meaning |
|-------|---------|--------|---------|
| HIGH  | 3.3V    | 1      | ON / TRUE / Active |
| LOW   | 0V      | 0      | OFF / FALSE / Inactive |

**IMPORTANT:** Never connect 5V to Pico GPIO pins - it can damage the chip!

---

## Basic GPIO Operations

### Output Example: Controlling Display Backlight

```cpp
#include "hardware/gpio.h"

#define PIN_BL 18  // Backlight pin

// Setup
gpio_init(PIN_BL);              // Initialize pin
gpio_set_dir(PIN_BL, GPIO_OUT); // Set as output

// Turn backlight ON
gpio_put(PIN_BL, 1);  // Set HIGH (3.3V)

// Turn backlight OFF
gpio_put(PIN_BL, 0);  // Set LOW (0V)
```

### Input Example: Reading a Button

```cpp
#include "hardware/gpio.h"

#define BUTTON_PIN 15  // Example button pin

// Setup
gpio_init(BUTTON_PIN);              // Initialize pin
gpio_set_dir(BUTTON_PIN, GPIO_IN);  // Set as input
gpio_pull_up(BUTTON_PIN);           // Enable pull-up resistor

// Read button state
if (gpio_get(BUTTON_PIN) == 0) {
    // Button is pressed (LOW)
    // Note: With pull-up, pressed = LOW, released = HIGH
}
```

---

## Button Polling for Games

### Why Polling Instead of Interrupts?

For continuous game input (character movement), **polling is better than interrupts**:

**Polling (reads button state once per frame):**
- Simple and predictable
- Minimal CPU overhead (~16 cycles per frame at 60 FPS)
- Natural timing synchronized with game loop
- No debouncing needed - frame time (16.67ms) handles bounce

**Level Interrupts (`GPIO_IRQ_LEVEL_HIGH`) - DO NOT USE:**
- Fire continuously while button held (1000s per second)
- Overwhelm CPU with context switches
- Destroy game performance

**Edge Interrupts (`GPIO_IRQ_EDGE_FALL`) - Only for UI:**
- Good for detecting single button press events
- Not suitable for continuous movement (miss held state)

### Implementation Pattern

```cpp
// drivers/buttons.h
struct ButtonState {
    bool w, a, s, d;
    bool i, j, k, l;
};

namespace Buttons {
    void init_buttons_pins();
    ButtonState button_polling();
}

// drivers/buttons.cpp
void init_button(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);  // Active-LOW buttons
}

ButtonState button_polling() {
    ButtonState state;
    state.w = !gpio_get(BTN_W);  // Invert: LOW = pressed
    state.a = !gpio_get(BTN_A);
    state.s = !gpio_get(BTN_S);
    state.d = !gpio_get(BTN_D);
    // ... etc
    return state;
}

// Game loop
void game_loop() {
    while(true) {
        ButtonState buttons = button_polling();

        // Continuous movement - works while button held
        if (buttons.w) player_y -= 1;
        if (buttons.s) player_y += 1;
        if (buttons.a) player_x -= 1;
        if (buttons.d) player_x += 1;

        // Diagonal movement works automatically
        // (multiple if statements execute in same frame)

        render();
        swap_buffers();
    }
}
```

### Performance Cost

At 60 FPS with 8 buttons:
- 8 × `gpio_get()` calls per frame
- Each call ~1-2 CPU cycles (register read)
- Total: **~16 cycles = 0.0001ms on 133MHz Pico**
- **0.0006% of 16.67ms frame budget**

Polling buttons will not slow down your game.

### Screen Boundary Clamping

When implementing movement, prevent objects from going off-screen:

```cpp
void update_player(ButtonState buttons, Player& player) {
    // Check bounds BEFORE moving (uint16_t can't be negative!)
    if (buttons.w && player.y > 0)
        player.y -= 1;
    if (buttons.s && player.y + player.height < SCREEN_HEIGHT)
        player.y += 1;
    if (buttons.a && player.x > 0)
        player.x -= 1;
    if (buttons.d && player.x + player.width < SCREEN_WIDTH)
        player.x += 1;
}
```

**Why check before moving?**
- Position uses `uint16_t` (unsigned integers)
- Cannot be negative - wraps to 65535 instead
- Must prevent decrement below 0 or increment beyond screen bounds

---

## Pull-Up and Pull-Down Resistors

### The Problem

When a GPIO pin is set as INPUT but nothing is connected to it, it "floats" - the voltage is undefined and random. The pin might read HIGH or LOW unpredictably.

### The Solution

**Pull-up Resistor** - Weakly pulls the pin to HIGH (3.3V) when nothing else controls it
```cpp
gpio_pull_up(pin);   // Pin defaults to HIGH
```
- Use for buttons that connect pin to GND when pressed
- Pressed button = LOW, Released button = HIGH

**Pull-down Resistor** - Weakly pulls the pin to LOW (0V) when nothing else controls it
```cpp
gpio_pull_down(pin);  // Pin defaults to LOW
```
- Use for buttons that connect pin to 3.3V when pressed
- Pressed button = HIGH, Released button = LOW

### Why "Weakly"?

The resistor is weak enough that when you press a button or drive the pin externally, the external signal overrides the pull resistor. But when nothing is connected, the pull resistor keeps the pin at a known state.

---

## GPIO vs Specialized Functions

The Pico has **30 GPIO pins**, but many can be assigned to specialized hardware peripherals:

### GPIO as Regular I/O
```cpp
gpio_init(pin);
gpio_set_dir(pin, GPIO_OUT);
gpio_put(pin, 1);
```
You manually control the pin with software (slow but flexible).

### GPIO as SPI Peripheral
```cpp
gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
```
The SPI hardware automatically controls the pin (fast, no CPU overhead).

### Available Functions

Each pin can be assigned to different peripherals:
- `GPIO_FUNC_SPI` - Serial Peripheral Interface (display, SD card)
- `GPIO_FUNC_UART` - Serial communication (debugging, GPS modules)
- `GPIO_FUNC_I2C` - Inter-Integrated Circuit (sensors, EEPROMs)
- `GPIO_FUNC_PWM` - Pulse Width Modulation (motors, servo control, analog output)
- `GPIO_FUNC_SIO` - Software controlled I/O (regular GPIO)

**When to use specialized functions:**
- Display communication → Use SPI (much faster than bit-banging)
- Button inputs → Use regular GPIO (simple, no special hardware needed)
- Chip Select signals → Use regular GPIO (need manual control)

---

## TriggEngine GPIO Usage

### Display Control Pins (Regular GPIO)

These pins need manual software control:

```cpp
#define PIN_BL      17     // Backlight (OUTPUT)
#define PIN_TFT_CS  20     // Display Chip Select (OUTPUT)
#define PIN_CARD_CS 21     // SD Card Chip Select (OUTPUT)
#define PIN_DC      22     // Data/Command (OUTPUT)
#define PIN_RESET   26     // Display Reset (OUTPUT)
```

**Why not SPI?** These are control signals, not data. We need to set them HIGH/LOW at specific times in our code.

For detailed pin voltage reference and behavior, see `Docs/hardware/display_pins.md`.

### SPI Communication Pins (Specialized Function)

These pins are assigned to SPI hardware:

```cpp
#define PIN_SCK     2      // SPI Clock
#define PIN_MOSI    3      // SPI Data Out
#define PIN_MISO    19     // SPI Data In

gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
```

**Why SPI?** The hardware can send 62.5 million bits per second automatically. Doing this manually with GPIO would be impossible.

### Game Input Pins (Regular GPIO)

8 buttons (W/A/S/D and I/J/K/L) for game controls:
- Regular GPIO in INPUT mode with pull-ups
- Polled once per frame in game loop with `gpio_get(pin)`
- Continuous press detection for free movement

---

## Chip Select Pattern

A common GPIO pattern in TriggEngine is **Chip Select (CS)**:

### Why Chip Select Exists

Multiple devices share the same SPI bus (SCK, MOSI, MISO). The CS pin tells each device "this data is for YOU":

```
              SCK ────┬──── Display
             MOSI ────┤
             MISO ────┼──── SD Card
                      │
    Display CS (20) ──┤
    SD Card CS (21) ──┘
```

### How It Works

**CS LOW (0)** = Device is selected and processes SPI data
**CS HIGH (1)** = Device ignores SPI bus

**CRITICAL RULE:** Never set both CS pins LOW simultaneously!

### Code Pattern

```cpp
// Talk to display
gpio_put(PIN_CARD_CS, 1);   // SD card OFF
gpio_put(PIN_TFT_CS, 0);    // Display ON
// ... send display data via SPI ...
gpio_put(PIN_TFT_CS, 1);    // Display OFF

// Talk to SD card
gpio_put(PIN_TFT_CS, 1);    // Display OFF
gpio_put(PIN_CARD_CS, 0);   // SD card ON
// ... read SD card data via SPI ...
gpio_put(PIN_CARD_CS, 1);   // SD card OFF
```

---

## GPIO Pin Limitations on Pico

### Total Pins: 30 GPIO pins (GP0-GP29)

Not all pins are equal:

**Safe for any use:** GP0-GP22
**ADC capable:** GP26-GP29 (can read analog voltages 0-3.3V)
**Special functions:** Some pins are preferred for certain peripherals

### Pin Planning

When designing your circuit:
1. Reserve pins for critical peripherals first (SPI, I2C)
2. Use any remaining pins for buttons and LEDs
3. Check Pico pinout diagram - some pins have multiple functions

**TriggEngine pin allocation:**
- **SPI (3 pins):** GP2, GP3, GP19
- **Display control (5 pins):** GP18, GP20, GP22, GP26
- **SD card control (1 pin):** GP21
- **Available for buttons:** ~21 pins remaining

---

## Common GPIO Mistakes

### 1. Forgetting to Initialize
```cpp
// WRONG - pin not initialized
gpio_put(PIN_BL, 1);  // Undefined behavior!

// CORRECT
gpio_init(PIN_BL);
gpio_set_dir(PIN_BL, GPIO_OUT);
gpio_put(PIN_BL, 1);  // Works correctly
```

### 2. Wrong Direction
```cpp
// WRONG - trying to write to input pin
gpio_set_dir(BUTTON_PIN, GPIO_IN);
gpio_put(BUTTON_PIN, 1);  // Does nothing!

// CORRECT - read from input pin
if (gpio_get(BUTTON_PIN) == 0) {
    // Button pressed
}
```

### 3. Floating Inputs
```cpp
// WRONG - button pin floats when not pressed
gpio_init(BUTTON_PIN);
gpio_set_dir(BUTTON_PIN, GPIO_IN);
// Button reads random values!

// CORRECT - use pull-up resistor
gpio_init(BUTTON_PIN);
gpio_set_dir(BUTTON_PIN, GPIO_IN);
gpio_pull_up(BUTTON_PIN);  // Pin stable when button not pressed
```

### 4. Voltage Mismatch
```cpp
// DANGER - connecting 5V Arduino output to Pico
// This can DESTROY the Pico!
// Pico is 3.3V only - use level shifter for 5V devices
```

---

## Performance Considerations

### GPIO Speed

**Fast:** ~30 MHz theoretical maximum for bit-banging
**Realistic:** 1-10 MHz for software-controlled GPIO

```cpp
// This is SLOW - CPU must execute each instruction
for (int i = 0; i < 1000; i++) {
    gpio_put(pin, 1);
    gpio_put(pin, 0);
}
```

**Hardware peripherals are MUCH faster:**
- SPI hardware: 62.5 MHz (20x faster than software GPIO)
- Use specialized functions when speed matters

### When Speed Matters

**Use regular GPIO when:**
- Reading buttons (only need to check 60 times/second)
- Controlling chip select (happens a few times per frame)
- One-time initialization (reset pin pulse)

**Use hardware peripherals when:**
- Sending 40KB framebuffer to display (needs SPI)
- High-frequency PWM for smooth motor control
- Precise timing requirements

---

## Reference

See `Docs/PICO_REFERENCE.md` for complete GPIO function list and examples.

See `drivers/display.cpp` for real-world GPIO usage in TriggEngine.
