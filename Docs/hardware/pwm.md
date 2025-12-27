# PWM (Pulse Width Modulation)

**What is PWM?**

PWM stands for **Pulse Width Modulation**. It's a technique to **simulate analog voltage** using a **digital on/off signal**. Instead of continuously varying voltage (0V to 3.3V), PWM rapidly switches between **fully ON (3.3V)** and **fully OFF (0V)** so fast that it **averages out** to a middle voltage.

---

## Table of Contents

1. [How PWM Works](#how-pwm-works)
2. [Visual Example](#visual-example)
3. [Key Terminology](#key-terminology)
4. [Why PWM for Brightness Control](#why-pwm-for-brightness-control)
5. [RP2040 Hardware PWM](#rp2040-hardware-pwm)
6. [Implementation: Backlight Brightness](#implementation-backlight-brightness)
7. [Performance Considerations](#performance-considerations)
8. [Common Applications](#common-applications)

---

## How PWM Works

Instead of outputting a variable voltage (which requires analog circuitry), PWM outputs a **square wave** that switches between HIGH and LOW at a fixed frequency.

### The Magic: Averaging

The key insight is that many devices (LEDs, motors, speakers) **respond to the average voltage**, not the instantaneous voltage.

When you switch a signal ON and OFF rapidly:
- **50% ON, 50% OFF** = average voltage of 1.65V (half of 3.3V)
- **75% ON, 25% OFF** = average voltage of 2.475V (3/4 of 3.3V)
- **25% ON, 75% OFF** = average voltage of 0.825V (1/4 of 3.3V)

---

## Visual Example

### Different Duty Cycles

```
100% Duty Cycle (Always ON):
PIN_BL: ████████████████████████████████  (3.3V constant)
Average: 3.3V
Brightness: Maximum


75% Duty Cycle:
PIN_BL: ██████░░██████░░██████░░██████░░  (3.3V 75% of the time)
Average: 2.475V
Brightness: 75%


50% Duty Cycle:
PIN_BL: ████░░░░████░░░░████░░░░████░░░░  (3.3V half the time)
Average: 1.65V
Brightness: 50%


25% Duty Cycle:
PIN_BL: ██░░░░░░██░░░░░░██░░░░░░██░░░░░░  (3.3V 1/4 of the time)
Average: 0.825V
Brightness: 25%


10% Duty Cycle:
PIN_BL: █░░░░░░░░█░░░░░░░░█░░░░░░░░█░░░░  (3.3V 10% of the time)
Average: 0.33V
Brightness: 10% (dim)


0% Duty Cycle (Always OFF):
PIN_BL: ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  (0V constant)
Average: 0V
Brightness: Off
```

**Legend:**
- `█` = HIGH (3.3V, LED ON)
- `░` = LOW (0V, LED OFF)

---

## Key Terminology

### Duty Cycle

**Definition:** The percentage of time the signal is HIGH during one period.

```
Duty Cycle = (Time HIGH / Total Period) × 100%
```

**Examples:**
- 100% duty cycle = always ON = full brightness
- 50% duty cycle = half ON, half OFF = half brightness
- 0% duty cycle = always OFF = no brightness

### Frequency

**Definition:** How many complete ON/OFF cycles occur per second, measured in Hertz (Hz).

```
Frequency = 1 / Period

Examples:
1 kHz = 1,000 cycles per second = 1ms per cycle
10 kHz = 10,000 cycles per second = 0.1ms per cycle
```

### Period

**Definition:** The time duration of one complete ON/OFF cycle.

```
Period = Time HIGH + Time LOW

Example at 1 kHz:
Period = 1ms
50% duty cycle = 0.5ms HIGH, 0.5ms LOW
25% duty cycle = 0.25ms HIGH, 0.75ms LOW
```

### Wrap Value (TOP)

**Definition:** The counter's maximum value before it resets to 0. Determines the resolution of your PWM.

```
Wrap Value = 999:
- 1,000 steps (0-999)
- Setting level to 500 = 50% duty cycle
- Setting level to 250 = 25% duty cycle

Wrap Value = 255:
- 256 steps (0-255)
- Setting level to 128 = 50% duty cycle
- Lower resolution than 999
```

**Higher wrap values = finer brightness control**

---

## Why PWM for Brightness Control

### Why It Works for LEDs

LEDs (including your backlight) respond to **average current**, not instantaneous current. When you switch an LED ON/OFF at 1,000 times per second (1 kHz):

1. **LED sees:** Average power at 50%
2. **Your eye sees:** Smooth brightness at 50%
3. **What's actually happening:** LED flickering ON/OFF 1,000 times per second

The flickering is **too fast for your eye to perceive** (humans can't detect flicker above ~100 Hz), so it appears as smooth dimming.

### Why Use PWM Instead of Analog Voltage?

| Method | Pros | Cons |
|--------|------|------|
| **PWM** | ✅ Simple (digital on/off)<br>✅ Efficient (no heat)<br>✅ Precise control<br>✅ Built into RP2040 | ❌ Can cause flicker if frequency too low |
| **Analog Voltage** | ✅ True smooth voltage | ❌ Requires DAC (expensive)<br>❌ Wastes power as heat<br>❌ Complex circuitry<br>❌ Not built into RP2040 |

**Verdict:** PWM is the standard method for LED brightness control on microcontrollers.

---

## RP2040 Hardware PWM

### What is Hardware PWM?

The RP2040 has **dedicated PWM hardware** that generates the square wave automatically, with **zero CPU overhead**.

**Software PWM (bad):**
```cpp
// CPU must manually toggle pin thousands of times per second
while (true) {
    gpio_put(PIN_BL, 1);
    sleep_us(500);  // ON for 500µs
    gpio_put(PIN_BL, 0);
    sleep_us(500);  // OFF for 500µs
}
// CPU is 100% busy doing nothing useful!
```

**Hardware PWM (good):**
```cpp
// Configure once, hardware does the rest
pwm_set_gpio_level(PIN_BL, 500);  // Set and forget
// CPU is free to do other work!
```

### PWM Slices

The RP2040 has **8 independent PWM slices** (PWM0-PWM7), each controlling 2 GPIO pins.

**Pin to Slice Mapping:**
```
GPIO 0, 16  →  Slice 0
GPIO 1, 17  →  Slice 0  ← PIN_BL is here!
GPIO 2, 18  →  Slice 1
GPIO 3, 19  →  Slice 1
...
GPIO 14, 30 →  Slice 7
GPIO 15     →  Slice 7
```

**Our backlight (GPIO 17) uses PWM Slice 0.**

### Finding Your Slice

```cpp
#include "hardware/pwm.h"

uint slice_num = pwm_gpio_to_slice_num(PIN_BL);
// For GPIO 17: slice_num = 0
```

---

## Implementation: Backlight Brightness

### Current Implementation (On/Off Only)

In `drivers/display.cpp:43`, the backlight is currently just a simple GPIO:

```cpp
// Simple ON/OFF - no brightness control
init_pin(PIN_BL, GPIO_OUT, 1);  // Turn backlight ON at 100%
```

**Limitations:**
- ❌ Only 2 states: fully ON or fully OFF
- ❌ No dimming
- ❌ No power savings
- ❌ No user comfort adjustment

### Upgraded Implementation (PWM Brightness Control)

Here's how to add smooth brightness control:

```cpp
#include "hardware/pwm.h"
#include "hardware_config.h"

void init_backlight_pwm() {
    // Step 1: Tell GPIO 17 it's controlled by PWM hardware
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);

    // Step 2: Find which PWM slice controls GPIO 17
    uint slice_num = pwm_gpio_to_slice_num(PIN_BL);

    // Step 3: Set PWM frequency
    // System clock: 125 MHz (peripheral clock)
    // Desired frequency: 1 kHz (good for LED backlights)
    // Formula: frequency = sys_clock / (wrap + 1) / divider

    pwm_set_wrap(slice_num, 999);  // 0-999 = 1,000 steps
    pwm_set_clkdiv(slice_num, 125.0f);  // 125 MHz / 125 = 1 MHz
    // Result: 1 MHz / 1,000 = 1 kHz PWM frequency

    // Step 4: Set initial brightness (50%)
    pwm_set_gpio_level(PIN_BL, 500);

    // Step 5: Enable PWM slice
    pwm_set_enabled(slice_num, true);
}

void set_brightness(uint16_t level) {
    // level: 0 (off) to 999 (max brightness)
    // Examples:
    //   999 = 100% brightness
    //   500 = 50% brightness
    //   100 = 10% brightness (dim)
    //     0 = 0% brightness (off)

    uint slice_num = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_gpio_level(PIN_BL, level);
}

void set_brightness_percent(uint8_t percent) {
    // percent: 0-100
    // Convenience function for percentage-based control

    if (percent > 100) percent = 100;
    uint16_t level = (percent * 999) / 100;
    set_brightness(level);
}
```

### Usage Examples

```cpp
// In your game initialization
init_backlight_pwm();
set_brightness(999);  // Start at full brightness

// During gameplay - user settings
set_brightness_percent(100);  // Maximum (bright room)
set_brightness_percent(75);   // High (normal room)
set_brightness_percent(50);   // Medium (dim room)
set_brightness_percent(25);   // Low (dark room)
set_brightness_percent(10);   // Minimum (night mode)

// Power saving mode
set_brightness(100);  // Very dim but still visible

// Screen off (but display still updating)
set_brightness(0);    // Backlight completely off
```

### Integration into display.cpp

Replace the simple GPIO initialization with PWM:

```cpp
// drivers/display.cpp

void init_control_pins_as_GPIO() {
    // Initialize CS (Chip Select)
    init_pin(PIN_TFT_CS, GPIO_OUT, 1);
    // Initialize DC (Data/Command)
    init_pin(PIN_DC, GPIO_OUT, 0);
    // Initialize RESET
    init_pin(PIN_RESET, GPIO_OUT, 1);

    // Initialize Backlight with PWM (not simple GPIO)
    init_backlight_pwm();  // Use PWM instead of init_pin()
}
```

---

## Performance Considerations

### Choosing PWM Frequency

**Too Low (<100 Hz):**
- ❌ Visible flicker (uncomfortable)
- ❌ Can cause eye strain
- ❌ May appear in photos/videos

**Good Range (1-10 kHz):**
- ✅ No visible flicker
- ✅ Efficient
- ✅ Clean signal
- ✅ Standard for LED control

**Too High (>50 kHz):**
- ❌ EMI (electromagnetic interference)
- ❌ Switching losses
- ❌ Unnecessary (human eye can't see >100 Hz)

**Recommendation for backlight: 1 kHz**

### Frequency Calculation

```
Formula:
PWM Frequency = System Clock / (Wrap + 1) / Clock Divider

Our settings:
System Clock: 125 MHz (RP2040 peripheral clock)
Wrap: 999 (1,000 steps)
Divider: 125

Result:
125,000,000 Hz / 1,000 / 125 = 1,000 Hz = 1 kHz

Period per cycle:
1 / 1,000 Hz = 1 millisecond
```

### CPU Overhead

**Hardware PWM: ~0% CPU usage**
- ✅ PWM hardware runs independently
- ✅ CPU only involved when changing brightness
- ✅ No impact on game loop performance

**Setting brightness is a single register write** (~10 CPU cycles = negligible)

### Resolution vs Frequency Trade-off

```
Fixed system clock: 125 MHz

High Resolution (999 steps):
Wrap = 999
Divider = 125
Frequency = 1 kHz
✅ Smooth brightness steps
✅ Still fast enough (no flicker)

Lower Resolution (255 steps):
Wrap = 255
Divider = 31.25
Frequency = 15.6 kHz
✅ Higher frequency
❌ Coarser brightness steps

Recommendation: 999 steps at 1 kHz
```

---

## Common Applications

### 1. LED Brightness (TriggEngine Use Case)

**Application:** Display backlight dimming

```cpp
set_brightness_percent(50);  // Half brightness
```

**Benefits:**
- Power savings (important for battery-powered consoles)
- User comfort (adjust for ambient light)
- Extend LED lifespan (lower current = longer life)

### 2. Motor Speed Control

**Application:** DC motor speed control (future robotics?)

```cpp
pwm_set_gpio_level(MOTOR_PIN, 750);  // 75% speed
```

**How it works:** Motor sees 75% of voltage = 75% of max speed

### 3. Servo Control

**Application:** Precise angle control for servo motors

```cpp
// Servo expects 1-2ms pulses at 50 Hz
pwm_set_wrap(slice, 20000);  // 20ms period = 50 Hz
pwm_set_gpio_level(SERVO_PIN, 1500);  // 1.5ms = center position
```

### 4. Analog Output Simulation

**Application:** Generate audio tones, control analog circuits

```cpp
// 440 Hz tone (musical note A)
pwm_set_wrap(slice, 2272);  // Period for 440 Hz
pwm_set_gpio_level(SPEAKER_PIN, 1136);  // 50% duty = square wave
```

### 5. RGB LED Color Mixing

**Application:** Full-color LED control (future RGB backlight?)

```cpp
// Red, Green, Blue channels each on separate PWM pin
pwm_set_gpio_level(LED_R, 999);  // Full red
pwm_set_gpio_level(LED_G, 500);  // Half green
pwm_set_gpio_level(LED_B, 0);    // No blue
// Result: Orange color
```

---

## PWM Best Practices

### DO:

✅ **Use hardware PWM** instead of software bit-banging
✅ **Choose appropriate frequency** (1-10 kHz for LEDs)
✅ **Use higher wrap values** for smoother control (999 > 255)
✅ **Initialize PWM once** during setup
✅ **Consider power savings** - dim during idle, low battery

### DON'T:

❌ **Don't use frequency <100 Hz** (visible flicker)
❌ **Don't change settings every frame** (unnecessary overhead)
❌ **Don't forget to enable the slice** (`pwm_set_enabled`)
❌ **Don't use software PWM** if hardware is available
❌ **Don't exceed GPIO current limits** (check RP2040 datasheet)

---

## Common PWM Mistakes

### 1. Forgetting to Set Pin Function

```cpp
// WRONG - pin still in GPIO mode
gpio_init(PIN_BL);
pwm_set_gpio_level(PIN_BL, 500);  // Does nothing!

// CORRECT - tell pin to use PWM hardware
gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
pwm_set_gpio_level(PIN_BL, 500);  // Works!
```

### 2. Not Enabling the Slice

```cpp
// WRONG - PWM configured but not running
pwm_set_wrap(slice, 999);
pwm_set_gpio_level(PIN_BL, 500);  // No output!

// CORRECT - enable the slice
pwm_set_wrap(slice, 999);
pwm_set_gpio_level(PIN_BL, 500);
pwm_set_enabled(slice, true);  // Now it runs!
```

### 3. Level Exceeds Wrap Value

```cpp
// WRONG - level is higher than wrap
pwm_set_wrap(slice, 255);
pwm_set_gpio_level(PIN_BL, 500);  // Clamped to 255, unexpected!

// CORRECT - level ≤ wrap
pwm_set_wrap(slice, 999);
pwm_set_gpio_level(PIN_BL, 500);  // 50% duty cycle ✓
```

### 4. Frequency Too Low

```cpp
// WRONG - visible flicker at 60 Hz
pwm_set_wrap(slice, 999);
pwm_set_clkdiv(slice, 2083.0f);  // 60 Hz - will flicker!

// CORRECT - use 1 kHz
pwm_set_wrap(slice, 999);
pwm_set_clkdiv(slice, 125.0f);  // 1 kHz - smooth ✓
```

---

## Technical Details: RP2040 PWM

### Hardware Specifications

| Parameter | Value |
|-----------|-------|
| **PWM Slices** | 8 independent slices |
| **Channels per Slice** | 2 (A and B channels) |
| **Resolution** | 16-bit counter (0-65535 max) |
| **System Clock** | 125 MHz (peripheral clock) |
| **Max Frequency** | 62.5 MHz (impractical) |
| **Practical Frequency** | 1 Hz - 10 MHz |

### Slice to GPIO Mapping

```
Slice 0:  GPIO 0, 1, 16, 17  ← PIN_BL (GPIO 17) is here
Slice 1:  GPIO 2, 3, 18, 19
Slice 2:  GPIO 4, 5, 20, 21
Slice 3:  GPIO 6, 7, 22
Slice 4:  GPIO 8, 9, 24, 25
Slice 5:  GPIO 10, 11, 26, 27
Slice 6:  GPIO 12, 13, 28
Slice 7:  GPIO 14, 15
```

**Note:** Each slice can control 2-4 GPIOs, but both share the same frequency and wrap settings. Only the duty cycle (level) can differ between channels.

### PWM Counter Operation

```
The PWM counter counts up from 0 to WRAP, then resets:

Counter: 0 → 1 → 2 → ... → 998 → 999 → 0 → 1 → ...
Output:  HIGH when counter < level
         LOW when counter ≥ level

Example with level=500, wrap=999:
Counter 0-499:   Output HIGH  (500 cycles)
Counter 500-999: Output LOW   (500 cycles)
Result: 50% duty cycle
```

---

## Quick Reference

### Basic Setup

```cpp
#include "hardware/pwm.h"

// Initialize PWM on a GPIO pin
gpio_set_function(pin, GPIO_FUNC_PWM);
uint slice = pwm_gpio_to_slice_num(pin);
pwm_set_wrap(slice, 999);          // 1,000 steps
pwm_set_clkdiv(slice, 125.0f);     // 1 kHz frequency
pwm_set_gpio_level(pin, 500);      // 50% duty cycle
pwm_set_enabled(slice, true);      // Start PWM
```

### Common Functions

```cpp
// Configure
pwm_set_wrap(slice, top_value);           // Set max counter value
pwm_set_clkdiv(slice, divider);           // Set clock divider
pwm_set_gpio_level(pin, level);           // Set duty cycle
pwm_set_enabled(slice, true/false);       // Enable/disable

// Query
uint slice = pwm_gpio_to_slice_num(pin);  // Get slice for GPIO
uint channel = pwm_gpio_to_channel(pin);  // Get channel (A=0, B=1)
```

### Brightness Control Template

```cpp
void init_brightness() {
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_wrap(slice, 999);
    pwm_set_clkdiv(slice, 125.0f);
    pwm_set_gpio_level(PIN_BL, 999);  // Start at 100%
    pwm_set_enabled(slice, true);
}

void set_brightness(uint16_t level) {
    pwm_set_gpio_level(PIN_BL, level);  // 0-999
}
```

---

## See Also

- **[gpio.md](gpio.md)** - GPIO basics and pin control
- **[display_pins.md](display_pins.md)** - Display pin configuration
- **[../TECHNICAL_SPECS.md](../TECHNICAL_SPECS.md)** - Hardware specifications
- **[../PICO_SDK_REFERENCE.md](../PICO_SDK_REFERENCE.md)** - Pico SDK PWM functions
- **drivers/display.cpp** - Current backlight implementation
