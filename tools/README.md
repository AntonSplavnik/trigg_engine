# TriggEngine Asset Tools

This directory contains tools for converting and inspecting game assets for the TriggEngine.

## Table of Contents

- [Binary Sprite Format](#binary-sprite-format)
- [Tool 1: PNG to Sprite Converter](#tool-1-png-to-sprite-converter)
- [Tool 2: Sprite Inspector](#tool-2-sprite-inspector)
- [Workflow Examples](#workflow-examples)
- [Troubleshooting](#troubleshooting)

---

## Binary Sprite Format

TriggEngine uses a simple binary format for sprites (`.sprite` files):

```
┌─────────────────────────────────────┐
│ Header (4 bytes)                    │
├─────────────────────────────────────┤
│ Width:  uint16_t (2 bytes)          │
│ Height: uint16_t (2 bytes)          │
├─────────────────────────────────────┤
│ Pixel Data (width × height × 2)    │
├─────────────────────────────────────┤
│ Pixel 0: uint16_t RGB565            │
│ Pixel 1: uint16_t RGB565            │
│ ...                                 │
│ Pixel N: uint16_t RGB565            │
└─────────────────────────────────────┘
```

### RGB565 Color Format

Each pixel is stored as a 16-bit value in RGB565 format:

```
Bit layout: RRRRR GGGGGG BBBBB
            ↑5    ↑6      ↑5 bits
```

**Conversion from RGB888 (8-bit per channel):**
```cpp
uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
```

### Transparency

- **Magic color:** `0xF81F` (magenta)
- Any pixel with this value is treated as transparent during rendering
- PNG pixels with alpha < 128 are converted to `0xF81F`

### File Size Calculation

```
File size = 4 bytes (header) + width × height × 2 bytes
```

**Examples:**
- 16×16 sprite = 4 + 256×2 = **516 bytes**
- 32×32 sprite = 4 + 1024×2 = **2,052 bytes**
- 64×64 sprite = 4 + 4096×2 = **8,196 bytes**

---

## Tool 1: PNG to Sprite Converter

**File:** `png_to_sprite_verbose.cpp`

Converts PNG images to binary `.sprite` format for TriggEngine.

### Features

✅ Supports PNG with or without alpha channel
✅ Automatic transparency detection (alpha < 128)
✅ RGB888 → RGB565 color conversion
✅ Verbose mode shows pixel-by-pixel analysis
✅ Validates input and reports warnings

### Build

```bash
# Requires stb_image.h in the same directory
g++ -std=c++11 png_to_sprite_verbose.cpp -o png_to_sprite_verbose
```

### Usage

```bash
# Basic conversion
./png_to_sprite_verbose <input.png>
# Output: <input>.sprite

# Specify output filename
./png_to_sprite_verbose <input.png> <output.sprite>

# Verbose mode (shows first 10 pixels)
./png_to_sprite_verbose <input.png> -v

# Verbose with custom output
./png_to_sprite_verbose <input.png> <output.sprite> -v
```

### Examples

```bash
# Convert player sprite
./png_to_sprite_verbose player_idle.png
# Output: player_idle.sprite

# Convert with verbose output
./png_to_sprite_verbose skeleton.png skeleton.sprite -v
```

### Output Explained

```
=== PNG to Sprite Converter ===
Input: skeleton.png
  Original channels: 4 (RGBA)           ← PNG has alpha channel
  Loaded as: 4 channels (RGBA forced)
  Dimensions: 59x43                     ← Sprite dimensions
  Total pixels: 2537

=== First 10 Pixels Analysis ===        ← Only with -v flag
Pixel [0,0]: RGBA(0,0,0,1) -> 0xF81F (TRANSPARENT)
Pixel [1,0]: RGBA(0,0,0,0) -> 0xF81F (TRANSPARENT)
...

=== Conversion Summary ===
Transparent pixels: 2388 (94%)          ← Alpha < 128
Opaque pixels: 149 (5%)                 ← Alpha >= 128
Output file: skeleton.sprite
File size: 5078 bytes                   ← 4 + 59×43×2

✓ Conversion complete!
```

### Warnings

**⚠️ No alpha channel detected:**
```
⚠️  WARNING: Original PNG has no alpha channel!
  Transparency detection may not work as expected.
  stb_image will fill alpha=255 (opaque) for missing alpha.
```
**Solution:** Re-save your PNG with transparency/alpha channel enabled in your image editor.

**⚠️ No transparent pixels found:**
```
⚠️  No transparent pixels found, but PNG has alpha channel.
  Check if your alpha values are all >= 128.
```
**Solution:** Your PNG might not actually have transparent areas, or the alpha values are too high (128-255 range).

---

## Tool 2: Sprite Inspector

**File:** `inspect_sprite.cpp`

Analyzes and validates `.sprite` binary files.

### Features

✅ Reads sprite dimensions
✅ Counts transparent vs opaque pixels
✅ Shows sample pixels with RGB values
✅ Detects common issues (empty, all transparent, etc.)
✅ No dependencies (pure C++11)

### Build

```bash
g++ -std=c++11 inspect_sprite.cpp -o inspect_sprite
```

### Usage

```bash
./inspect_sprite <sprite_file.sprite>
```

### Examples

```bash
# Inspect a sprite file
./inspect_sprite skeleton.sprite

# Inspect sprite from assets folder
./inspect_sprite ../assets/player.sprite
```

### Output Explained

```
=== Sprite File Inspector ===
File: skeleton.sprite
Dimensions: 59x43                       ← Read from header
Total pixels: 2537

=== First 10 Transparent Pixels (0xF81F) ===
=== First 10 Opaque Pixels ===
  Pixel [0,0]: 0xf81f (TRANSPARENT)
  Pixel [1,0]: 0xf81f (TRANSPARENT)
  ...
  Pixel [19,2]: 0x5249 -> RGB(80,72,72)  ← First opaque pixel
  Pixel [17,3]: 0x4a6a -> RGB(72,76,80)
  ...

=== Summary ===
Transparent pixels (0xF81F): 2388 (94%)
Opaque pixels: 149 (5%)

✓ Sprite looks valid!                  ← Health check
```

### Health Checks

**✓ Valid sprite:**
```
✓ Sprite looks valid!
```

**⚠️ No transparent pixels:**
```
⚠️  WARNING: No transparent pixels found!
Your PNG might not have alpha channel data.
```
**Cause:** PNG was saved without transparency, or all alpha values are >= 128.

**⚠️ All transparent:**
```
⚠️  WARNING: All pixels are transparent!
Your PNG might be completely transparent or empty.
```
**Cause:** Empty image, or all alpha values are < 128.

---

## Workflow Examples

### Basic Workflow

```bash
# 1. Convert PNG to sprite
./png_to_sprite_verbose enemy.png

# 2. Verify the conversion
./inspect_sprite enemy.sprite

# 3. Move to assets folder
mv enemy.sprite ../assets/
```

### Debugging Transparency Issues

```bash
# 1. Convert with verbose mode
./png_to_sprite_verbose character.png -v

# Look at the first few pixels - do they have low alpha values?
# Pixel [0,0]: RGBA(255,255,255,0) -> 0xF81F (TRANSPARENT)  ✓ Good
# Pixel [0,0]: RGBA(255,255,255,255) -> 0xFFFF (OPAQUE)    ✗ No transparency

# 2. If no transparency detected, check your PNG:
#    - Open in image editor
#    - Ensure it has an alpha channel
#    - Ensure transparent areas have alpha = 0-127
#    - Re-export as PNG with transparency enabled
```

### Batch Conversion

```bash
# Convert all PNGs in current directory
for file in *.png; do
    ./png_to_sprite_verbose "$file"
done

# Verify all sprites
for file in *.sprite; do
    echo "Checking $file..."
    ./inspect_sprite "$file"
    echo ""
done
```

### Checking Sprite Details

```bash
# Quick size check
ls -lh *.sprite

# Detailed analysis
./inspect_sprite character.sprite | grep -A 10 "First 10 Opaque"
```

---

## Troubleshooting

### Problem: "No transparent pixels found"

**Symptoms:**
- Converter reports 0% transparent pixels
- PNG has alpha channel in editor

**Causes:**
1. Alpha values are all >= 128 (semi-transparent)
2. PNG saved without alpha data

**Solutions:**
```bash
# 1. Check with verbose mode
./png_to_sprite_verbose mysprite.png -v

# 2. Look at alpha values in first 10 pixels
# If alpha is 128-255, pixels are treated as opaque

# 3. Re-export PNG with:
#    - Alpha channel enabled
#    - Transparent areas = alpha 0 (fully transparent)
#    - Opaque areas = alpha 255 (fully opaque)
```

### Problem: "All pixels are transparent"

**Symptoms:**
- Inspector reports 100% transparent
- Sprite appears empty on device

**Causes:**
1. PNG is actually empty
2. All pixels have alpha < 128

**Solutions:**
```bash
# Check the PNG in an image viewer
# Ensure you can see the actual graphic

# If graphic is visible but converter sees it as transparent,
# your alpha channel might be inverted
```

### Problem: "Wrong colors on device"

**Symptoms:**
- Colors look different on display vs PNG

**Causes:**
1. RGB565 color space has less precision than RGB888
2. Color calibration differences

**Solutions:**
```bash
# Check the actual RGB565 values
./inspect_sprite mysprite.sprite | grep "RGB("

# Compare to original PNG colors
# Remember: RGB888 (24-bit) → RGB565 (16-bit) loses precision
#   Red:   8-bit → 5-bit (loses 3 bits)
#   Green: 8-bit → 6-bit (loses 2 bits)
#   Blue:  8-bit → 5-bit (loses 3 bits)
```

### Problem: "File size is huge"

**Symptoms:**
- Sprite file is larger than expected

**Causes:**
1. Image dimensions are too large
2. Not using appropriate resolution for display

**Solutions:**
```bash
# Check dimensions
./inspect_sprite large_sprite.sprite

# TriggEngine display is 128×160 pixels
# Sprites should typically be 16×16, 32×32, or 64×64

# Resize in image editor if needed
```

### Problem: "Converter can't find stb_image.h"

**Symptoms:**
```
fatal error: 'stb_image.h' file not found
```

**Solutions:**
```bash
# Download stb_image.h to tools directory
curl -L https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -o stb_image.h

# Or copy it manually to tools/
# Then rebuild
g++ -std=c++11 png_to_sprite_verbose.cpp -o png_to_sprite_verbose
```

---

## Technical Details

### Why Binary Format?

**Advantages over C++ headers:**
- ✅ Smaller file size (no text overhead)
- ✅ Faster to load at runtime
- ✅ Works with SD card file system (FatFS)
- ✅ Can update assets without recompiling engine
- ✅ Fits cartridge-style game architecture

**Disadvantages:**
- ❌ Can't embed directly in Flash (requires runtime loading)
- ❌ Needs file I/O code

### Alpha Threshold: Why 128?

```cpp
if (alpha < 128) {
    // Transparent
} else {
    // Opaque
}
```

**Reasoning:**
- Values 0-127 = transparent (50% transparency or more)
- Values 128-255 = opaque (less than 50% transparency)
- Simple binary decision (TriggEngine doesn't support partial transparency)
- 128 is the midpoint of 0-255 range

### Endianness

Both tools use **little-endian** byte order (matches ARM Cortex-M0+ on Raspberry Pi Pico).

```cpp
// uint16_t is stored as:
// Byte 0: Low byte
// Byte 1: High byte
```

**Example:**
```
RGB565 value: 0xF81F (magenta)
Stored as:    0x1F 0xF8 (in file)
```

---

## File Format Specification

### Version 1.0

```
Offset | Size | Type     | Description
-------|------|----------|---------------------------
0x00   | 2    | uint16_t | Width (little-endian)
0x02   | 2    | uint16_t | Height (little-endian)
0x04   | W×H×2| uint16_t | Pixel array (RGB565, row-major)
```

**Row-major order:**
```
Pixel[0]     = top-left
Pixel[W-1]   = top-right
Pixel[W]     = second row, left
Pixel[W×H-1] = bottom-right
```

**Magic transparency color:**
- `0xF81F` (RGB565: R=31, G=0, B=31) = Magenta

---

## Dependencies

### png_to_sprite_verbose.cpp
- **stb_image.h** - Single-header image loading library
  - Source: https://github.com/nothings/stb
  - License: Public Domain / MIT
  - Version: Any recent version (tested with 2.28+)

### inspect_sprite.cpp
- **None** - Pure C++11 standard library

---

## Compiling on Different Platforms

### macOS / Linux
```bash
g++ -std=c++11 png_to_sprite_verbose.cpp -o png_to_sprite_verbose
g++ -std=c++11 inspect_sprite.cpp -o inspect_sprite
```

### Windows (MinGW)
```bash
g++ -std=c++11 png_to_sprite_verbose.cpp -o png_to_sprite_verbose.exe
g++ -std=c++11 inspect_sprite.cpp -o inspect_sprite.exe
```

### Windows (Visual Studio)
```bash
cl /std:c++11 /EHsc png_to_sprite_verbose.cpp
cl /std:c++11 /EHsc inspect_sprite.cpp
```

---

## License

These tools are part of TriggEngine and follow the same license as the main project.

---

## See Also

- [Engine Documentation](../Docs/)
- [Sprite System](../Docs/isometric/sprites.md)
- [Asset Storage](../Docs/memory/flash.md)
- [SD Card Usage](../Docs/memory/sd_card.md)
