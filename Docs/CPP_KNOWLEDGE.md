# C++ Knowledge Bank

## Table of Contents
- [Random Number Generation](#random-number-generation)
- [Smart Pointers](#smart-pointers)
- [Lambda Functions](#lambda-functions)
- [Move Semantics](#move-semantics)

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

## Smart Pointers

*To be added as we learn...*

---

## Lambda Functions

*To be added as we learn...*

---

## Move Semantics

*To be added as we learn...*
