# C++ for Embedded Systems

## Table of Contents
- [Memory Storage: Flash vs RAM](#memory-storage-flash-vs-ram)
- [Random Number Generation](#random-number-generation)

---

## Memory Storage: Flash vs RAM

### The `const` Keyword in Embedded

**Critical rule:** Use `const` for read-only data to save RAM.

```cpp
// ❌ BAD - Stored in RAM (wastes precious 264KB)
static NamedColor COLORS[] = {
    {"RED", 0xF800},
    {"GREEN", 0x07E0}
};

// ✅ GOOD - Stored in Flash (2MB available)
static const NamedColor COLORS[] = {
    {"RED", 0xF800},
    {"GREEN", 0x07E0}
};
```

**Why it matters:**
- Pico has **264KB RAM** (precious) vs **2MB Flash** (abundant)
- `const` global data → Compiler puts it in Flash ROM
- Non-`const` global data → Compiler puts it in RAM
- Flash is read-only but perfect for constants, lookup tables, assets

---

### Memory Allocation Types

**Three types of allocation (NOT dynamic):**

```cpp
// 1. FLASH (compile-time, read-only)
static const uint16_t LOOKUP[] = {1, 2, 3};  // Flash ROM

// 2. RAM - Global/Static (compile-time, read-write)
static uint16_t framebuffer[20480];  // RAM .bss section

// 3. STACK (automatic, function scope)
void foo() {
    uint16_t temp[10];  // Stack, destroyed on return
}

// HEAP (manual, avoid in embedded)
uint16_t* ptr = new uint16_t[10];  // ❌ Heap fragmentation risk
```

**Common misconception:**
- `[]` syntax **does NOT mean dynamic allocation**
- Only `new`/`malloc` allocate dynamically (heap)
- Array size known at compile-time → static allocation

---

### When to Use What

| Data Type | Storage | Use Case |
|-----------|---------|----------|
| `const` globals | Flash | Color palettes, tile data, sprites, strings |
| Non-`const` globals | RAM | Framebuffer, game state |
| Stack arrays | Stack | Temporary buffers, small scratch space |
| Heap (`new`/`malloc`) | Heap | ❌ Avoid - fragmentation risk |

**Best practice:** Default to `const` for any data that won't change at runtime.

---

### Why Framebuffer Can't Use Flash

```cpp
// ❌ IMPOSSIBLE - Flash is read-only hardware
const uint16_t framebuffer[20480];  // Can't change pixels!

// ✅ REQUIRED - Framebuffer changes 60 times/second
uint16_t framebuffer[20480];  // RAM
```

**Flash limitations:**
- Read-only memory (ROM)
- Write speed: ~milliseconds (vs <16ms per frame needed)
- Write endurance: ~10,000 cycles (dies in 3 min at 60 FPS)
- Only use Flash for immutable data

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
