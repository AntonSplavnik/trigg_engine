# C++ Knowledge Bank

## Table of Contents
- [Compile-Time Constants (constexpr)](#compile-time-constants-constexpr)
- [Random Number Generation](#random-number-generation)
- [Volatile Keyword](#volatile-keyword)
- [Smart Pointers](#smart-pointers)
- [Lambda Functions](#lambda-functions)
- [Move Semantics](#move-semantics)

---

## Compile-Time Constants (constexpr)

### Why constexpr Matters

**Problem:** C++ requires array sizes to be known at compile-time.

```cpp
// ❌ Won't compile - runtime constant
extern const uint16_t WIDTH;
uint16_t buffer[WIDTH];  // Error: array size must be constant expression
```

```cpp
// ✅ Works - compile-time constant
constexpr uint16_t WIDTH = 160;
uint16_t buffer[WIDTH];  // OK
```

### const vs constexpr

**`const` = Runtime constant** (value set once, but at runtime):
```cpp
extern const uint16_t DISPLAY_WIDTH;  // Defined elsewhere
// Compiler doesn't know value until link time
```

**`constexpr` = Compile-time constant** (value must be known during compilation):
```cpp
constexpr uint16_t DISPLAY_WIDTH = 160;  // Value known at compile-time
// Can be used for array sizes, template parameters, etc.
```

### Practical Example: Display Constants

**drivers/display.h:**
```cpp
#include "hardware_config.h"

// Compile-time constants for engine layer
constexpr uint16_t DISPLAY_WIDTH = SCREEN_WIDTH;
constexpr uint16_t DISPLAY_HEIGHT = SCREEN_HEIGHT;
```

**engine/graphics/framebuffer.h:**
```cpp
#include "display.h"

// Now this works - constexpr allows array size
static uint16_t framebuffer[DISPLAY_HEIGHT * DISPLAY_WIDTH];
```

### Key Points
- Use `constexpr` for values needed at compile-time (array sizes, template args)
- Use `const` for runtime values that shouldn't change
- `constexpr` implies `const` (all constexpr are const, not vice versa)
- C++11 constexpr can only contain single return statement

---

## Random Number Generation

### C++11 Random Library

**Old Way (C-style, avoid):**
```cpp
#include <cstdlib>
#include <ctime>

srand(time(NULL));        // Seed with current time
int num = rand() % 100;   // Random 0-99 (poor distribution)
```

**Problems:**
- Global state (not thread-safe)
- Poor random distribution
- Hard to control ranges

---

**C++11 Way (Better):**
```cpp
#include <random>

// 1. Create random engine
std::random_device rd;                    // Obtains seed from OS
std::mt19937 gen(rd());                   // Mersenne Twister engine

// 2. Create distribution
std::uniform_int_distribution<> dist(0, 99);  // Range 0-99

// 3. Generate random number
int num = dist(gen);                      // Random number 0-99
```

**Components:**

**Random Engine (`std::mt19937`):**
- Generates pseudo-random numbers
- Fast and high-quality
- `mt19937` = Mersenne Twister (32-bit)
- `mt19937_64` = 64-bit version

**Random Device (`std::random_device`):**
- Gets seed from system entropy (hardware/OS)
- Non-deterministic (truly random)
- Use ONCE to seed the engine
- **Warning:** May not be available on all embedded systems!

**Distributions:**
```cpp
std::uniform_int_distribution<int>(min, max)      // Integers [min, max]
std::uniform_real_distribution<float>(min, max)   // Floats [min, max)
std::normal_distribution<double>(mean, stddev)    // Gaussian/bell curve
std::bernoulli_distribution(probability)          // true/false with probability
```

---

**Example: Dice Roll (1-6):**
```cpp
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dice(1, 6);

int roll = dice(gen);  // Random 1-6
```

---

**Reusable Random Generator:**
```cpp
class Random {
private:
    std::mt19937 gen;
public:
    Random() : gen(std::random_device{}()) {}

    int getInt(int min, int max) {
        std::uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }
};

// Usage:
Random rng;
int x = rng.getInt(0, 100);
```

---

## Volatile Keyword

### What is volatile?

The `volatile` keyword tells the compiler: **"This variable can change unexpectedly - don't optimize reads/writes to it."**

### Why volatile Matters in Embedded Systems

**Normal Variables:**
```cpp
int counter = 0;

// Compiler sees this loop:
while (counter < 10) {
    // Do something
}

// Compiler optimizes to:
if (counter < 10) {
    while (true) {  // Infinite loop!
        // Do something
    }
}
// Compiler assumes: "counter never changes inside loop,
// so check it once and cache the result"
```

**volatile Variables:**
```cpp
volatile int counter = 0;

// Compiler sees this loop:
while (counter < 10) {
    // Do something
}

// Compiler MUST re-read counter every iteration:
// Cannot optimize or cache the value
// Reads directly from memory each time
```

### When to Use volatile

**1. Variables Modified by Interrupts (ISR)**

```cpp
volatile bool button_pressed = false;

void button_isr() {
    button_pressed = true;  // ISR changes this
}

int main() {
    while (!button_pressed) {
        // Without volatile, compiler might cache button_pressed
        // Loop would never see ISR's change!
    }
}
```

**2. Hardware Registers (Memory-Mapped I/O)**

```cpp
volatile uint32_t* GPIO_REG = (uint32_t*)0x40014000;

// Hardware can change register value externally
uint32_t status = *GPIO_REG;  // Must read from hardware every time
```

**3. Variables Shared Between Threads**

```cpp
volatile bool stop_flag = false;

void worker_thread() {
    while (!stop_flag) {  // Must check actual memory
        // Work...
    }
}
```

### When NOT to Use volatile

**❌ Don't use volatile for thread synchronization:**
```cpp
// WRONG - not thread-safe!
volatile int shared_counter = 0;
shared_counter++;  // NOT atomic! Race condition!
```

Use proper synchronization (mutexes, atomics) instead.

**❌ Don't use volatile for regular variables:**
```cpp
// WRONG - unnecessary
volatile int total = a + b + c;
```

Only use when variable is modified externally (hardware, ISR, etc.).

### volatile vs const

You can combine them for read-only hardware registers:

```cpp
volatile const uint32_t* STATUS_REG = (uint32_t*)0x40014000;
// const = we can't modify it
// volatile = hardware can change it, always re-read
```

### Real Example: Button Input with ISR

**Without volatile (BROKEN):**
```cpp
bool button_w_pressed = false;  // ❌ Compiler will optimize

void button_callback(uint gpio, uint32_t events) {
    button_w_pressed = true;  // ISR sets flag
}

void game_loop() {
    while (true) {
        if (button_w_pressed) {  // Compiler caches this check!
            move_up();           // May never execute!
        }
    }
}
```

**With volatile (CORRECT):**
```cpp
volatile bool button_w_pressed = false;  // ✅ Always re-read

void button_callback(uint gpio, uint32_t events) {
    button_w_pressed = true;  // ISR sets flag
}

void game_loop() {
    while (true) {
        if (button_w_pressed) {  // Always reads from memory
            move_up();           // Works correctly!
        }
    }
}
```

### Performance Impact

**volatile prevents optimizations:**
- Forces memory read/write every access
- Cannot cache value in CPU register
- Slower than non-volatile access

**Use sparingly:**
- Only when external changes are expected
- Not needed for normal variables in single-threaded code
- **Note:** For button polling (not ISR), volatile is NOT needed:

```cpp
// Polling - no ISR, no volatile needed
bool pressed = !gpio_get(BTN_W);  // Direct hardware read
```

### Key Takeaways

- `volatile` = "This can change unexpectedly, don't optimize"
- Essential for ISR-modified variables and hardware registers
- Does NOT provide thread safety or atomicity
- Has performance cost - use only when necessary
- Not needed for polling-based input (direct hardware reads)

---

## Smart Pointers

*To be added as we learn...*

---

## Lambda Functions

*To be added as we learn...*

---

## Move Semantics

*To be added as we learn...*
