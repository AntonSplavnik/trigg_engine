# Fixed-Point Arithmetic

A guide to fixed-point math for embedded systems without floating-point hardware.

---

## Table of Contents

1. [What is Fixed-Point?](#what-is-fixed-point)
2. [Q Notation](#q-notation)
3. [Why Fixed-Point on Embedded?](#why-fixed-point-on-embedded)
4. [Conversions](#conversions)
5. [Arithmetic Rules](#arithmetic-rules)
6. [Why Exponents Add in Multiplication](#why-exponents-add-in-multiplication)
7. [Why int64 Intermediate but int32 Storage](#why-int64-intermediate-but-int32-storage)
8. [Implementation](#implementation)

---

## What is Fixed-Point?

Fixed-point is a way to represent fractional numbers using integers by dedicating a fixed number of bits to the fractional part.

```
Integer:     00000101                    = 5
Fixed Q8.8:  00000101 00000000           = 5.0
Fixed Q8.8:  00000101 10000000           = 5.5  (0x80 = 128 = half of 256)
             ├──int──┤├──frac──┤
```

Instead of a floating decimal point, the "point" is at a fixed position — hence "fixed-point."

### Decimal Analogy

Think of it like always having 2 decimal places:

```
Integer: 5
Fixed:   500  (represents 5.00, scale factor = 100)
         550  (represents 5.50)
         525  (represents 5.25)
```

To get the real value: `fixed_value / scale_factor`

In binary, the scale factor is a power of 2 (e.g., 65536 for 16 fractional bits).

---

## Q Notation

Q notation describes the format: **Q[integer bits].[fractional bits]**

### Common Formats

| Format  | Total Bits | Integer Range     | Fractional Precision | Scale Factor |
|---------|------------|-------------------|----------------------|--------------|
| Q8.8    | 16-bit     | -128 to 127       | 1/256 ≈ 0.004        | 256          |
| Q16.16  | 32-bit     | -32,768 to 32,767 | 1/65,536 ≈ 0.00002   | 65,536       |
| Q24.8   | 32-bit     | ±8,388,607        | 1/256 ≈ 0.004        | 256          |

### Q16.16 (Used in this engine)

```
32-bit signed integer:
├─────── 16 bits integer ───────┤├─ 16 bits fraction ─┤

Range:    -32,768 to 32,767 (integer part)
Precision: 1/65,536 ≈ 0.000015 (smallest step)
Scale:     65,536 (2^16)
```

### Why Q16.16 over Q24.8?

For isometric game math, Q16.16 is preferred:

1. **Better precision for division**: The world-to-screen formula divides by 2. More fractional bits (16 vs 8) means less rounding error accumulation.

2. **Smooth sub-pixel movement**: Velocity like 0.3 pixels/frame requires fine precision. 1/65,536 is much finer than 1/256.

3. **Sufficient range**: ±32,767 tiles is enormous for a 160×128 display. Even with 16-pixel tiles, that's 500,000+ pixels of world space.

4. **Clean math**: 16 is half of 32, making mental math and debugging easier.

### When Q24.8 makes sense

- You need huge world coordinates (millions of units)
- Your math is simple (just positioning, no physics/interpolation)
- Memory-mapping directly to byte-aligned data

---

## Why Fixed-Point on Embedded?

### The Problem with Float

The Raspberry Pi Pico (ARM Cortex-M0+) has **no floating-point unit (FPU)**. Float operations are emulated in software:

| Operation      | Fixed-Point | Float (Software) |
|----------------|-------------|------------------|
| Addition       | 1 cycle     | ~20-50 cycles    |
| Multiplication | 1-4 cycles  | ~50-100 cycles   |
| Division       | ~10 cycles  | ~100-200 cycles  |

For 60 FPS with coordinate math every frame, this adds up quickly.

### Fixed-Point Advantages

- **Fast**: Uses integer ALU (native hardware)
- **Predictable**: Same speed regardless of value
- **Deterministic**: No floating-point rounding surprises
- **Memory efficient**: Same size as integers

### When Float is Acceptable

- One-time calculations (initialization, asset loading)
- PC-side tools (not on embedded target)
- When precision matters more than speed

---

## Conversions

### Integer to Fixed-Point

Multiply by scale factor (shift left by fractional bits):

```
int → fixed:  value << 16

Example:
  5 << 16 = 5 × 65,536 = 327,680 (raw)

  Binary:
  00000000 00000000 00000000 00000101        (5)
                     ↓ shift left 16
  00000000 00000101 00000000 00000000        (327,680 raw = 5.0 fixed)
```

### Fixed-Point to Integer

Divide by scale factor (shift right by fractional bits):

```
fixed → int:  raw >> 16

Example:
  360,448 >> 16 = 360,448 / 65,536 = 5 (truncated)

  Binary:
  00000000 00000101 10000000 00000000        (360,448 raw = 5.5 fixed)
                     ↓ shift right 16
  00000000 00000000 00000000 00000101        (5)
```

**Note:** Right shift truncates (rounds toward zero). For rounding to nearest, add half before shifting:

```cpp
int rounded = (raw + 32768) >> 16;  // 32768 = half of 65536
```

---

## Arithmetic Rules

### Addition and Subtraction

**No adjustment needed** — scale factors are the same:

```
a.raw + b.raw = result.raw

Example (Q16.16):
  2.5 + 1.25 = 3.75

  163,840 + 81,920 = 245,760
  (2.5 × 65,536) + (1.25 × 65,536) = (3.75 × 65,536) ✓
```

The fractional bits stay at 16.

### Multiplication

**Fractional bits add up** — must shift back:

```
a.raw × b.raw = intermediate with (16 + 16) = 32 fractional bits
intermediate >> 16 = result with 16 fractional bits
```

**Example:**
```
1.5 × 2.0 = 3.0

1.5 in Q16.16: raw = 98,304   (1.5 × 65,536)
2.0 in Q16.16: raw = 131,072  (2.0 × 65,536)

98,304 × 131,072 = 12,884,901,888  (32 fractional bits)
12,884,901,888 >> 16 = 196,608     (16 fractional bits)
196,608 / 65,536 = 3.0 ✓
```

**Overflow warning:** The intermediate result can overflow 32 bits. Use 64-bit for the multiplication:

```cpp
int32_t result = (static_cast<int64_t>(a.raw) * b.raw) >> 16;
```

### Division

**Fractional bits subtract** — must shift before dividing:

```
(a.raw << 16) / b.raw = result.raw
```

**Why:** Division undoes multiplication's effect:
```
Q16.16 / Q16.16 would give Q16.0 (no fractional bits)

Pre-shift to preserve precision:
(Q16.16 << 16) / Q16.16 = Q32.32 / Q16.16 = Q16.16
```

**Example:**
```
3.0 / 1.5 = 2.0

  (196,608 << 16) / 98,304 = 12,884,901,888 / 98,304 = 131,072
  131,072 / 65,536 = 2.0 ✓
```

### Summary Table

| Operation      | Formula                               | Fractional Bits  |
|----------------|---------------------------------------|------------------|
| Addition       | `a.raw + b.raw`                       | 16 (unchanged)   |
| Subtraction    | `a.raw - b.raw`                       | 16 (unchanged)   |
| Multiplication | `(int64_t(a.raw) * b.raw) >> 16`      | 16+16-16 = 16    |
| Division       | `(int64_t(a.raw) << 16) / b.raw`      | 16+16-16 = 16    |

---

## Why Exponents Add in Multiplication

This is the key insight for understanding why we shift after multiplication.

### The Concept

Fixed-point stores values as: `actual_value × scale_factor`

For Q16.16, scale factor = 2^16 = 65,536

```
A.raw = A_actual × 2^16
B.raw = B_actual × 2^16
```

When you multiply:

```
A.raw × B.raw = (A_actual × 2^16) × (B_actual × 2^16)
              = A_actual × B_actual × 2^16 × 2^16
              = A_actual × B_actual × 2^32
```

But you want the result in Q16.16 format:

```
result.raw = A_actual × B_actual × 2^16  (one factor of 2^16, not two)
```

So you divide by 2^16 (shift right 16) to remove the extra scaling:

```
(A.raw × B.raw) >> 16 = A_actual × B_actual × 2^32 / 2^16
                      = A_actual × B_actual × 2^16 ✓
```

### Scientific Notation Analogy

Same principle as scientific notation:

```
10³ × 10³ = 10⁶  (exponents add: 3 + 3 = 6)
```

In Q16.16:

```
2^16 × 2^16 = 2^32  (exponents add: 16 + 16 = 32)
```

To get back to 2^16 scaling, subtract 16 from the exponent (shift right by 16).

### Why Addition Doesn't Need Adjustment

```
A.raw + B.raw = (A_actual × 2^16) + (B_actual × 2^16)
              = (A_actual + B_actual) × 2^16
```

The scale factor stays at 2^16 — no correction needed.

---

## Why int64 Intermediate but int32 Storage

### The Problem

When multiplying two 32-bit numbers, the result can be up to 64 bits:

```
Max int32: 2,147,483,647
Max int32 × Max int32 = 4,611,686,014,132,420,609 (requires 63 bits)
```

If you use 32-bit multiplication, you get overflow and wrong results.

### The Solution

Use 64-bit for the intermediate calculation, then shift back to 32-bit:

```cpp
int32_t result = (static_cast<int64_t>(a.raw) * b.raw) >> 16;
```

### Why Not Store as int64?

The RP2040 (Cortex-M0+) is a **32-bit CPU** with no native 64-bit operations:

| Operation | 32-bit      | 64-bit on M0+        |
|-----------|-------------|----------------------|
| Add       | 1 cycle     | ~4-6 cycles          |
| Multiply  | 1 cycle     | ~20-40 cycles        |
| Shift     | 1 cycle     | ~6-10 cycles         |
| Storage   | 4 bytes     | 8 bytes              |

Every 64-bit operation becomes multiple 32-bit instructions emulated by the compiler.

### The Balance

- **Storage**: int32_t (fast access, half the memory)
- **Multiplication intermediate**: int64_t (prevents overflow)
- **Result**: Shifted back to int32_t (fast for subsequent operations)

This gives you overflow safety during multiplication without the constant overhead of 64-bit math.

### Range Consideration

With Q16.16 in int32_t:
- Integer range: ±32,767
- For a 160×128 screen with 16px tiles: ~2,000 tiles visible
- ±32,767 tiles is 16× larger than needed

If you truly needed millions of units, Q24.8 would give ±8 million range while keeping 32-bit storage.

---

## Implementation

### C++ Struct (Q16.16)

```cpp
struct Fixed_q16 {
    int32_t raw;

    // Constructors
    Fixed_q16() : raw(0) {}
    Fixed_q16(int32_t value) : raw(value << 16) {}

    // Convert back to integer
    int32_t to_int() const { return raw >> 16; }

    // Operators
    Fixed_q16 operator+(const Fixed_q16& other) const {
        Fixed_q16 result;
        result.raw = raw + other.raw;
        return result;
    }

    Fixed_q16 operator-(const Fixed_q16& other) const {
        Fixed_q16 result;
        result.raw = raw - other.raw;
        return result;
    }

    Fixed_q16 operator*(const Fixed_q16& other) const {
        Fixed_q16 result;
        result.raw = (static_cast<int64_t>(raw) * other.raw) >> 16;
        return result;
    }

    Fixed_q16 operator/(const Fixed_q16& other) const {
        Fixed_q16 result;
        result.raw = (static_cast<int64_t>(raw) << 16) / other.raw;
        return result;
    }
};
```

### Usage

```cpp
Fixed_q16 position(10);      // 10.0
Fixed_q16 velocity(2);       // 2.0
Fixed_q16 dt_fixed;
dt_fixed.raw = 16384;        // 0.25 (16384/65536) - direct raw assignment

position = position + velocity * dt_fixed;  // 10.0 + 2.0 * 0.25 = 10.5

int screen_x = position.to_int();  // 10 (for pixel drawing)
```

---

## See Also

- [Bitwise Operations](BITWISE_OPERATIONS.md) - Shift mechanics and bit manipulation
- `engine/isometric/iso_math.h` - Engine implementation

