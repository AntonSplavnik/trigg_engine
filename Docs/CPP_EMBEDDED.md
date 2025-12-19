# C++ for Embedded Systems

## Table of Contents
- [Random Number Generation](#random-number-generation)

---

## Random Number Generation

### System Entropy on Embedded Devices

**Problem:**
```cpp
#include <random>

std::random_device rd;  // May NOT work on embedded systems!
std::mt19937 gen(rd());
```

**Why it fails:**
- `std::random_device` needs system entropy sources (hardware RNG, /dev/random, etc.)
- Raspberry Pi Pico and many microcontrollers **lack hardware random generators**
- May compile but return the same "random" value every time
- Not truly random without entropy source

---

**Solutions for Embedded Systems:**

### Option 1: Manual Seeding (Simple)
```cpp
#include <random>
#include "pico/stdlib.h"

// Use time or ADC noise as seed
uint32_t seed = time_us_32();  // Microsecond timestamp
std::mt19937 gen(seed);

std::uniform_int_distribution<> dist(0, 99);
int num = dist(gen);
```

**Pros:** Simple, works everywhere
**Cons:** Predictable if code runs at same time on boot

---

### Option 2: ADC Noise Seeding (Better)
```cpp
#include "hardware/adc.h"

// Read floating ADC pin for noise
adc_init();
adc_select_input(3);  // Use unconnected ADC pin
uint32_t seed = adc_read();  // LSBs are noise

std::mt19937 gen(seed);
```

**Pros:** More random than time-based
**Cons:** Requires ADC peripheral

---

### Option 3: Combine Multiple Sources (Best)
```cpp
uint32_t get_seed() {
    uint32_t seed = time_us_32();        // Time component
    seed ^= adc_read();                  // XOR with ADC noise
    seed ^= (uint32_t)&seed;             // XOR with stack address
    return seed;
}

std::mt19937 gen(get_seed());
```

**Pros:** Best randomness for embedded
**Cons:** More complex

---

### Option 4: External Hardware RNG
- Use external chips (e.g., hardware RNG modules)
- Read over SPI/I2C
- Most secure but adds cost

---

**Key Lesson:**
> Never assume `std::random_device` works on embedded systems. Always test or use manual seeding with hardware sources (time, ADC, etc.).

---

*This document will grow as we learn embedded-specific C++ patterns...*
