# Alpha Channel in Graphics

## What is Alpha Channel?

The **alpha channel** is a transparency component in image data that controls pixel opacity.

### Color Representation

**RGB** (No transparency):
```
Red:   0-255
Green: 0-255
Blue:  0-255
```

**RGBA** (With transparency):
```
Red:   0-255
Green: 0-255
Blue:  0-255
Alpha: 0-255  (0 = fully transparent, 255 = fully opaque)
```

## Alpha Values

| Alpha | Opacity | Visual Effect                   |
|-------|---------|---------------------------------|
| 0     | 0%      | Invisible (fully transparent)   |
| 64    | 25%     | Heavily blended with background |
| 128   | 50%     | Half transparent                |
| 192   | 75%     | Mostly opaque                   |
| 255   | 100%    | Fully visible (no transparency) |

## Use Cases in PocketGateEngine

### 1. Sprite Transparency
**Without alpha** (binary transparency):
```
Character sprite: 32×32 pixels
Problem: Rectangular background visible
Result: Ugly boxes around sprites
```

**With alpha**:
```
Character sprite: 32×32 pixels
- Character pixels: alpha = 255 (opaque)
- Background pixels: alpha = 0 (transparent)
Result: Only character visible, no rectangular boundary
```

### 2. Anti-Aliasing (Smooth Edges)

**Jagged edge** (no alpha):
```
████████
████■■░░  ← Hard edge
███■■■░░  ← Staircase effect
```

**Smooth edge** (with alpha):
```
████████
████■▓▒░  ← Gradient edge
███■▓▒▒░  ← Smooth curve
     ^^^ Semi-transparent pixels (alpha: 192, 128, 64)
```

Edge pixels have **partial alpha values** that blend with the background, eliminating jaggies.

### 3. Soft Shadows

Realistic shadows have gradual falloff:

```
Shadow sprite (top-down view):
┌─────────────┐
│ α=0  α=30   │  ← Outer penumbra (barely visible)
│ α=30 α=80   │  ← Mid region (translucent)
│ α=80 α=120  │  ← Inner umbra (darker)
└─────────────┘
```

When rendered over ground tiles:
- **Alpha=30**: Slight darkening (barely noticeable)
- **Alpha=80**: Medium shadow (still see ground texture)
- **Alpha=120**: Dark shadow (strong darkening)

### 4. UI Overlays

Semi-transparent UI elements:
```
Health bar background: alpha=180 (translucent black)
Dialog box: alpha=220 (mostly opaque)
Tooltip: alpha=200 (slightly transparent)
```

## Alpha Blending Formula

When compositing a sprite pixel over a background:

```
Output = (Sprite × Alpha) + (Background × (1 - Alpha))
```

**Example**: Red sprite (255,0,0) at alpha=128 over blue background (0,0,255)

```
Alpha normalized: 128/255 = 0.5

Red channel:   (255 × 0.5) + (0 × 0.5)   = 127
Green channel: (0 × 0.5)   + (0 × 0.5)   = 0
Blue channel:  (0 × 0.5)   + (255 × 0.5) = 127

Result: Purple (127, 0, 127)
```

## Implementation in PocketGateEngine

### Memory Constraint Challenge
**Problem**: Full RGBA requires 4 bytes per pixel
- 32×32 sprite = 4,096 bytes (vs 2,048 for RGB565)
- Limited Pico RAM (264KB)

### Solution: Pre-multiplied Alpha + 1-bit Mask

**Option 1**: 1-bit alpha mask (binary transparency)
```cpp
struct Sprite {
    uint16_t* pixels;      // RGB565 data (2 bytes/pixel)
    uint8_t* alpha_mask;   // 1 bit per pixel (packed)
};

// Check if pixel is transparent
bool is_transparent(int x, int y) {
    int bit_index = y * width + x;
    return (alpha_mask[bit_index / 8] & (1 << (bit_index % 8))) == 0;
}
```

**Option 2**: Separate 8-bit alpha channel (smooth blending)
```cpp
struct SpriteAlpha {
    uint16_t* pixels;  // RGB565 colors
    uint8_t* alpha;    // 8-bit alpha per pixel (3 bytes total per pixel)
};

// Blend pixel with background
uint16_t blend_pixel(uint16_t sprite_color, uint8_t alpha, uint16_t bg_color) {
    if (alpha == 0) return bg_color;
    if (alpha == 255) return sprite_color;

    // Extract RGB565 components
    uint8_t sr = (sprite_color >> 11) & 0x1F;
    uint8_t sg = (sprite_color >> 5) & 0x3F;
    uint8_t sb = sprite_color & 0x1F;

    uint8_t br = (bg_color >> 11) & 0x1F;
    uint8_t bg = (bg_color >> 5) & 0x3F;
    uint8_t bb = bg_color & 0x1F;

    // Blend (alpha is 0-255)
    uint8_t r = (sr * alpha + br * (255 - alpha)) / 255;
    uint8_t g = (sg * alpha + bg * (255 - alpha)) / 255;
    uint8_t b = (sb * alpha + bb * (255 - alpha)) / 255;

    return (r << 11) | (g << 5) | b;
}
```

### Tool: sprite_to_cpp_alpha.py

Your conversion tool preserves alpha from PNG:
- Reads RGBA PNG source
- Extracts alpha channel
- Outputs C++ arrays with both RGB565 and alpha data
- Stored in Flash (not RAM)

## Performance Considerations

### Blending Cost
**Per-pixel alpha blending** is expensive:
- Extract RGB components: 3 operations
- Blend calculation: 3 multiplies, 3 adds, 3 divides
- Repack RGB565: 2 operations
- **~15-20 CPU cycles per pixel**

### Optimization Strategies

1. **Binary alpha** (1-bit): Fast, skip transparent pixels entirely
2. **Pre-blend common cases**: Cache alpha=128 blends
3. **Skip full opacity**: if (alpha==255) just copy pixel
4. **Batch operations**: Process 4 pixels with SIMD-like tricks
5. **Limit blended sprites**: Most sprites are binary transparent, few need smooth alpha

### Frame Budget
At 60 FPS (16.67ms/frame):
- 10 sprites × 400 visible pixels × 20 cycles = 80,000 cycles
- Pico @ 125 MHz = ~0.64ms (acceptable)
- Stay under 3-4ms for sprite rendering

## Relationship to Hardware

**Important**: Alpha blending happens in **software** (engine), not hardware.

1. Engine renders sprites with alpha blending → framebuffer (RGB565)
2. Framebuffer sent to ST7735 display → **no alpha**, just RGB565
3. Display shows final composited image

The display only understands RGB565 (no transparency). Alpha is purely an engine rendering technique.

## Best Practices

1. **Store in Flash**: Sprite data with alpha lives in Flash, not RAM
2. **Binary when possible**: Use 1-bit mask for most sprites (memory efficient)
3. **Smooth for quality**: Use 8-bit alpha only for AA edges and shadows
4. **Avoid real-time blending**: Pre-bake shadows/effects when possible
5. **Profile first**: Measure actual FPS impact before optimizing

## Visual Examples

### Isometric Character Sprite
```
Top-down view of 32×32 sprite:

░░░░░░░░░░░░░░░░  ← alpha=0 (transparent background)
░░░░░■■■■░░░░░░░  ← alpha=255 (opaque character)
░░░░■■■■■■░░░░░░
░░░░■■■■■■░░░░░░
░░░░░■■■■░░░░░░░
░░░░░░■■░░░░░░░░
Edge pixels: alpha=128-192 (AA smoothing)
```

### Shadow Under Character
```
Ground tile: ████████████
Character:   ░░■■■■░░
Shadow:      ▒▒▒▒▒▒▒▒  ← alpha=80 (darkens ground)
Result:      ▓▓■■■■▓▓  ← Blended composite
```

## Resources

- PNG spec: Supports 8-bit alpha channel
- Your tool: `tools/sprite_to_cpp_alpha.py`
- Related: `Docs/hardware/rgb565.md` (display color format)
- Related: `engine/graphics/sprite.h` (sprite rendering)
