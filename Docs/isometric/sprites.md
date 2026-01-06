# Graphics System Documentation

## Static Sprites

### What is a Sprite?

A sprite is **pixel data with color information**. Unlike a solid-color rectangle, a sprite contains a pattern of multiple colors arranged in a grid.

**Comparison:**
- **Rectangle:** Single color, defined by position + dimensions
- **Sprite:** Multiple colors in a pattern, defined by position + **pixel array**

### Sprite Data Structure

A sprite consists of:
1. **Width & Height** - dimensions in pixels
2. **Pixel Array** - color value for each pixel (RGB565 format)
3. **Transparency** (optional) - which pixels should be "invisible"

Think of a sprite as a **2D grid of colors**. For example:
- 16x16 sprite = 256 color values
- 32x32 sprite = 1024 color values

### Storage Strategy

**During Level Load:**
- Sprite data copied from Flash → RAM
- Stored as const arrays in RGB565 format
- All necessary sprites loaded at level start

**During Rendering:**
- Data already in RAM - no loading mid-frame
- Direct copy from sprite array to framebuffer
- No decompression needed

### Drawing Process

Instead of filling a rectangle with one color:

1. Loop through the sprite's pixel array (row by row)
2. For each pixel: copy its color to framebuffer at position (x + pixel_x, y + pixel_y)
3. **Skip transparent pixels** (typically using a "magic color" like magenta 0xF81F)

**Performance Note:** This is slower than drawing a solid rectangle because each pixel is written individually. Optimization through batching is important.

---

## Animated Sprites

### Concept

Animation = **multiple sprites (frames) shown in sequence**, like a flipbook.

An animated sprite doesn't move on its own:
- **Animation** changes the visual appearance
- **Game logic** changes the position (x, y)

### How It Works

1. **Sprite Sheet** - all animation frames stored together, divided into individual frames
2. **Frame Timing** - switch frames every X milliseconds
3. **Frame Index** - tracks which frame is currently displayed

**Example: Walking Animation**
```
Frame 0: Left foot forward
Frame 1: Both feet together
Frame 2: Right foot forward
Frame 3: Both feet together
→ Loop back to Frame 0
```

### Storage

Instead of one pixel array, an animated sprite has:
- **Array of pixel arrays** (one per frame)
- **Frame duration** - how long each frame displays (in milliseconds)
- **Total frame count** - number of frames in the animation

### Update Logic

On each game tick:
1. Check elapsed time since last frame change
2. If enough time passed → increment frame index
3. If reached last frame → loop back to first frame (or stop if non-looping)
4. Draw the **current frame's sprite** at the object's position

---

## Key Concepts

### Transparency
- Use a "magic color" to mark transparent pixels (common: magenta 0xF81F)
- During drawing, skip pixels that match the transparency color
- Allows sprites to have irregular shapes (not just rectangles)

### Frame Budget
- Each sprite draw takes time
- With 60 FPS target: ~16.67ms per frame total
- Must render all sprites + send to display within this budget
- Optimize by batching operations, minimizing individual pixel writes

### Sprite vs Rectangle Performance
- **Rectangle:** Fast - single memset operation per row
- **Sprite:** Slower - individual pixel operations with transparency checks
- **Solution:** Minimize sprite count, optimize drawing routines

---

## Implementation Considerations

### Memory (RAM)
- Limited to 264KB total on Pico
- Pre-load only active level sprites
- Unload unused sprites between levels

### Storage (Flash)
- Store all sprite data in Flash
- Copy to RAM only when needed
- Use const arrays for sprite definitions

### Display Format
- RGB565 (16-bit color)
- Each pixel = 2 bytes
- Match sprite data format to display format (no conversion needed)
