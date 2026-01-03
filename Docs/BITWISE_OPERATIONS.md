# Bitwise Operations in C++

A comprehensive guide to understanding bits, bit shifting, and bitwise operators used in embedded programming.

---

## Table of Contents

1. [What Are Bits?](#what-are-bits)
2. [Bit Shifting](#bit-shifting)
3. [Bitwise Operators](#bitwise-operators)
4. [Practical Examples](#practical-examples)
5. [Common Patterns](#common-patterns)

---

## What Are Bits?

A number in a computer is stored as a sequence of 0s and 1s (binary):

```
Decimal: 159
Binary:  10011111
         ││││││││
         └┴┴┴┴┴┴┴── These are individual bits (8 bits = 1 byte)
```

Each position has a value (power of 2):

```
Position:  7   6   5   4   3   2   1   0
Value:    128  64  32  16   8   4   2   1
Bit:       1   0   0   1   1   1   1   1

159 = 128 + 16 + 8 + 4 + 2 + 1
```

**Important sizes:**
- **1 byte** = 8 bits = 0 to 255
- **2 bytes (uint16_t)** = 16 bits = 0 to 65,535
- **4 bytes (uint32_t)** = 32 bits = 0 to 4,294,967,295

---

## Bit Shifting

**Shifting means physically moving all bits left or right.**

Think of it like sliding beads on an abacus or moving train cars on a track.

### Right Shift (`>>`)

Moves bits to the right → **divides by powers of 2**

**Formula:** `value >> n` = divide by `2^n`

**Shift right by 1: `>> 1`**
```
BEFORE:  1 0 0 1 1 1 1 1  (159)
         ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
         Move everything one position to the right →
         ↓ ↓ ↓ ↓ ↓ ↓ ↓
AFTER:   0 1 0 0 1 1 1 1  (79)
         ↑               ↑
         New 0           This bit falls off!
```

Result: `159 >> 1 = 79` (159 ÷ 2 = 79)

**Shift right by 4: `>> 4`**
```
BEFORE:  1 0 0 1 1 1 1 1  (159)
         ↓ ↓ ↓ ↓
         Move 4 positions right →→→→
         ↓ ↓ ↓ ↓
AFTER:   0 0 0 0 1 0 0 1  (9)
```

Result: `159 >> 4 = 9` (159 ÷ 16 = 9)

**Shift right by 8: `>> 8`**
```
BEFORE:  0 0 0 0 0 0 0 1   0 0 1 0 1 1 0 0  (16-bit: 300)
         ← High byte →     ← Low byte →

         Move 8 positions right →→→→→→→→

AFTER:   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 1  (1)
                            ↑
                            High byte moved here!
```

Result: `300 >> 8 = 1` (300 ÷ 256 = 1)

**Common shift amounts:**
```
>> 1  = divide by 2
>> 2  = divide by 4
>> 3  = divide by 8
>> 4  = divide by 16
>> 5  = divide by 32
>> 6  = divide by 64
>> 7  = divide by 128
>> 8  = divide by 256
```

### Left Shift (`<<`)

Moves bits to the left → **multiplies by powers of 2**

**Formula:** `value << n` = multiply by `2^n`

**Shift left by 1: `<< 1`**
```
BEFORE:  0 0 1 0 1 1 0 0  (44)
         ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
         ← Move everything one position left
           ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
AFTER:   0 1 0 1 1 0 0 0  (88)
         ↑               ↑
    Fell off!         New 0
```

Result: `44 << 1 = 88` (44 × 2 = 88)

**Shift left by 8: `<< 8`**
```
BEFORE:  0 0 0 0 0 0 0 0   0 0 1 0 1 1 0 0  (16-bit: 44)
                            ← Low byte →

         ←←←←←←←← Move 8 positions left
         ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓

AFTER:   0 0 1 0 1 1 0 0   0 0 0 0 0 0 0 0  (11,264)
         ← Now high byte → ← zeros →
```

Result: `44 << 8 = 11,264` (44 × 256 = 11,264)

**Common shift amounts:**
```
<< 1  = multiply by 2
<< 2  = multiply by 4
<< 3  = multiply by 8
<< 4  = multiply by 16
<< 5  = multiply by 32
<< 6  = multiply by 64
<< 7  = multiply by 128
<< 8  = multiply by 256
```

### Real-World Analogy

Think of decimal digits sliding:

```
Number: 1234

Shift right by 2 digits:
  1234
    → →
    12  (34 fell off)

Shift left by 2 digits:
  1234
  ← ←
  123400  (added two zeros)
```

Bit shifting is the **same concept in binary**!

---

## Bitwise Operators

### AND (`&`) - Filter/Mask

Keeps bits only where **BOTH** are 1.

**Truth table:**
```
1 & 1 = 1  ← Only this makes 1
1 & 0 = 0
0 & 1 = 0
0 & 0 = 0
```

**Example: Extract low byte**
```
  0b00000001 00101100  (300)
& 0b00000000 11111111  (0xFF = mask)
─────────────────────
  0b00000000 00101100  (44)
  ↑↑↑↑↑↑↑↑
  High byte zeroed out!
```

**Use cases:**
- Extract specific bits
- Mask off unwanted bits
- Check if bits are set

**Visual analogy:** Like an eraser mask
```
Original: ████████
Mask:     ████░░░░
Result:   ████░░░░  (erased the right side)
```

### OR (`|`) - Combine/Merge

Sets bit to 1 if **EITHER** is 1.

**Truth table:**
```
1 | 1 = 1
1 | 0 = 1
0 | 1 = 1
0 | 0 = 0  ← Only this makes 0
```

**Example: Combine high and low bytes**
```
  0b00000001 00000000  (256 from high << 8)
| 0b00000000 00101100  (44 from low)
─────────────────────
  0b00000001 00101100  (300)
  ← High →  ← Low →
```

**Use cases:**
- Combine multiple values
- Set specific bits
- Merge bit patterns

**Visual analogy:** Like adding paint layers
```
Layer 1:  ████░░░░
Layer 2:  ░░░░████
Result:   ████████  (combined both layers)
```

### XOR (`^`) - Toggle/Flip

Returns 1 if bits are **DIFFERENT**.

**Truth table:**
```
1 ^ 1 = 0  ← Same = 0
1 ^ 0 = 1  ← Different = 1
0 ^ 1 = 1  ← Different = 1
0 ^ 0 = 0  ← Same = 0
```

**Example: Toggle bits**
```
  0b11110000  (240)
^ 0b11111111  (0xFF)
─────────────
  0b00001111  (15)
  ↑↑↑↑↑↑↑↑
  All bits flipped!
```

**Use cases:**
- Toggle bits on/off
- Simple encryption
- Swap values without temporary variable

### NOT (`~`) - Invert

Flips all bits: 1→0, 0→1.

**Example:**
```
~0b00001111 = 0b11110000

~44 = ~0b00101100
    =  0b11010011
    = 211 (in 8-bit)
```

**Use cases:**
- Invert all bits
- Create inverse masks

---

## Practical Examples

### Example 1: Splitting 16-bit into 2 Bytes

**Used in display communication:**

```cpp
uint16_t x2 = 300;  // Display coordinate

// Extract high byte (upper 8 bits)
uint8_t high = x2 >> 8;
// 300 >> 8 = 0b00000001 00101100 >> 8
//          = 0b00000000 00000001
//          = 1

// Extract low byte (lower 8 bits)
uint8_t low = x2 & 0xFF;
// 300 & 0xFF = 0b00000001 00101100 & 0b11111111
//            = 0b00000000 00101100
//            = 44

// Send to display: 0x01, 0x2C
send_data_byte(high);  // 1
send_data_byte(low);   // 44
```

**Why this works:**
```
300 in binary: 0b00000001 00101100
               ← High →  ← Low →
                 (1)       (44)

300 = (1 × 256) + 44
```

### Example 2: Combining 2 Bytes into 16-bit

**Reconstructing coordinate from bytes:**

```cpp
uint8_t high_byte = 1;
uint8_t low_byte = 44;

// Build 16-bit value
uint16_t value = (high_byte << 8) | low_byte;

// Step by step:
// high_byte << 8 = 1 << 8
//                = 0b00000001 << 8
//                = 0b00000001 00000000
//                = 256

// Then OR with low byte:
//   0b00000001 00000000  (256)
// | 0b00000000 00101100  (44)
// ─────────────────────
//   0b00000001 00101100  (300) ✓
```

### Example 3: RGB565 Color Format

**Packing RGB values into 16-bit color:**

```cpp
// RGB values (0-255)
uint8_t red = 255;    // Full red
uint8_t green = 128;  // Half green
uint8_t blue = 64;    // Quarter blue

// Convert to RGB565 (5 bits red, 6 bits green, 5 bits blue)
uint16_t color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);

// Breakdown:
// Red:   255 >> 3 = 31 (5 bits), shift to bits 11-15
// Green: 128 >> 2 = 32 (6 bits), shift to bits 5-10
// Blue:  64 >> 3  = 8  (5 bits), position at bits 0-4

// Result: 0bRRRRR GGGGGG BBBBB
```

### Example 4: Setting Flags

**Managing multiple boolean states in one byte:**

```cpp
// Define bit positions
#define FLAG_VISIBLE  (1 << 0)  // 0b00000001
#define FLAG_ACTIVE   (1 << 1)  // 0b00000010
#define FLAG_DIRTY    (1 << 2)  // 0b00000100
#define FLAG_ENABLED  (1 << 3)  // 0b00001000

uint8_t flags = 0;

// Set flags
flags |= FLAG_VISIBLE;  // Turn on visible
flags |= FLAG_ACTIVE;   // Turn on active
// flags = 0b00000011

// Check if flag is set
if (flags & FLAG_VISIBLE) {
    // Visible is on!
}

// Clear a flag
flags &= ~FLAG_ACTIVE;  // Turn off active
// flags = 0b00000001

// Toggle a flag
flags ^= FLAG_VISIBLE;  // Flip visible
```

### Example 5: Extracting Nibbles (4-bit values)

**Getting high and low nibbles from a byte:**

```cpp
uint8_t value = 0xA7;  // 0b10100111

// High nibble (upper 4 bits)
uint8_t high_nibble = value >> 4;
// 0b10100111 >> 4 = 0b00001010 = 0xA = 10

// Low nibble (lower 4 bits)
uint8_t low_nibble = value & 0x0F;
// 0b10100111 & 0b00001111 = 0b00000111 = 0x7 = 7
```

---

## Common Patterns

### Extract Byte from Multi-byte Value

```cpp
uint32_t value = 0x12345678;

uint8_t byte3 = (value >> 24) & 0xFF;  // 0x12 (highest)
uint8_t byte2 = (value >> 16) & 0xFF;  // 0x34
uint8_t byte1 = (value >> 8) & 0xFF;   // 0x56
uint8_t byte0 = value & 0xFF;          // 0x78 (lowest)
```

### Combine Bytes into Multi-byte Value

```cpp
uint8_t b0 = 0x78, b1 = 0x56, b2 = 0x34, b3 = 0x12;

uint32_t value = ((uint32_t)b3 << 24) |
                 ((uint32_t)b2 << 16) |
                 ((uint32_t)b1 << 8) |
                 b0;
// Result: 0x12345678
```

### Check if Bit is Set

```cpp
uint8_t value = 0b10110100;
uint8_t bit_position = 5;

bool is_set = (value >> bit_position) & 1;
// or
bool is_set = (value & (1 << bit_position)) != 0;
```

### Set a Specific Bit

```cpp
uint8_t value = 0b00000000;
uint8_t bit_position = 3;

value |= (1 << bit_position);  // Set bit 3
// Result: 0b00001000
```

### Clear a Specific Bit

```cpp
uint8_t value = 0b11111111;
uint8_t bit_position = 3;

value &= ~(1 << bit_position);  // Clear bit 3
// Result: 0b11110111
```

### Toggle a Specific Bit

```cpp
uint8_t value = 0b10101010;
uint8_t bit_position = 0;

value ^= (1 << bit_position);  // Toggle bit 0
// Result: 0b10101011
```

### Align to Power of 2

```cpp
// Round up to next multiple of 256
uint32_t addr = 1234;
uint32_t aligned = (addr + 255) & ~255;
// Result: 1280 (next 256-byte boundary)
```

---

## Quick Reference Table

| Operation   | Operator | Purpose          | Example                |
|-------------|----------|------------------|------------------------|
| Right shift | `>>`     | Divide by 2^n    | `160 >> 4 = 10`        |
| Left shift  | `<<`     | Multiply by 2^n  | `10 << 4 = 160`        |
| AND         | `&`      | Filter/mask bits | `0xF0 & 0xFF = 0xF0`   |
| OR          | `\|`     | Combine bits     | `0xF0 \| 0x0F = 0xFF`  |
| XOR         | `^`      | Toggle bits      | `0xF0 ^ 0xFF = 0x0F`   |
| NOT         | `~`      | Invert all bits  | `~0xF0 = 0x0F` (8-bit) |

## Memory Sizes

| Type       | Bytes | Bits | Range                          |
|------------|-------|------|--------------------------------|
| `uint8_t`  | 1     | 8    | 0 - 255                        |
| `uint16_t` | 2     | 16   | 0 - 65,535                     |
| `uint32_t` | 4     | 32   | 0 - 4,294,967,295              |
| `uint64_t` | 8     | 64   | 0 - 18,446,744,073,709,551,615 |

## Power of 2 Reference

```
2^0  = 1
2^1  = 2
2^2  = 4
2^3  = 8
2^4  = 16
2^5  = 32
2^6  = 64
2^7  = 128
2^8  = 256
2^9  = 512
2^10 = 1024 (1 KB)
2^16 = 65536 (64 KB)
```

---

## Tips for Embedded Programming

1. **Use shifts for power-of-2 operations**
   - `value >> 3` is faster than `value / 8`
   - `value << 2` is faster than `value * 4`

2. **Masks are your friend**
   - `0xFF` = keep lower 8 bits
   - `0x0F` = keep lower 4 bits (nibble)
   - `0xF0` = keep upper 4 bits

3. **Byte order matters**
   - Display protocols often expect **big-endian** (high byte first)
   - ARM Cortex-M0+ (Pico) is **little-endian** internally
   - Always check datasheet for byte order!

4. **Avoid unnecessary casts**
   - Shifting can promote to `int`, be careful with types
   - Use explicit casts when combining bytes: `(uint16_t)high << 8`

5. **Comment your bit manipulations**
   - Bitwise operations can be cryptic
   - Explain what each mask/shift does
   - Future you will thank you!

---

## See Also

- `drivers/st7735_driver.cpp` - Real-world usage in `set_window()`
- `hardware/display_commands.md` - Display communication protocol
- `CPP_EMBEDDED.md` - Embedded C++ best practices
