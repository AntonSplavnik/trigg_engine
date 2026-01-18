# Bresenham's Line Algorithm

## What Is It?

Bresenham's line algorithm (1962) is an efficient method for drawing straight lines between two points using only **integer arithmetic**. It determines which pixels to illuminate to form the best approximation of a straight line.

## Why Use It?

| Benefit.              | Description                                         |
|-----------------------|-----------------------------------------------------|
| **No floating-point** | Uses only integers — critical for Pico (no FPU)     |
| **No division**       | Only addition, subtraction, bit shifts              |
| **Deterministic**     | Fixed operations per pixel, predictable performance |
| **Memory efficient**  | No lookup tables required                           |
| **Pixel-perfect**     | Produces optimal rasterization                      |

For embedded systems like RP2040, avoiding float operations can be **10-100x faster**.

## The Core Idea

When drawing from point A to B, at each step we must choose between two pixels. Bresenham tracks an **error term** that tells us which pixel is closer to the ideal line.

```
Ideal line passes between pixels:

    ·───────────·───────────·
    │           │           │
    │     ○     │     ●     │  ← Which one is closer to the line?
    │      \    │    /      │
    ·───────\───·───/───────·
             \     /
              \   /
               \ /
                ● ← Current pixel
```

## Algorithm (Shallow Lines: dx > dy)

For lines where horizontal distance exceeds vertical:

```
1. Calculate deltas:
   dx = |x1 - x0|
   dy = |y1 - y0|

2. Initialize decision variable:
   d = 2*dy - dx

3. For each x from x0 to x1:
   - Plot pixel at (x, y)
   - If d > 0:
       y += 1        (move up)
       d -= 2*dx     (adjust error)
   - d += 2*dy       (accumulate error)
```

## Decision Variable Explained

The decision variable `d` represents **2× the signed distance** from the midpoint between two candidate pixels to the ideal line.

- `d > 0` → ideal line is **above** midpoint → choose upper pixel
- `d ≤ 0` → ideal line is **below** midpoint → choose lower pixel

Why multiply by 2? Eliminates fractions while preserving the sign comparison.

## Visual Example

Drawing line from (0,0) to (5,3):

```
dx = 5, dy = 3
d_initial = 2*3 - 5 = 1

Step | x | y | d before | d > 0? | Action      | d after
-----|---|---|----------|--------|-------------|--------
  0  | 0 | 0 |    1     |  yes   | y++, d-=10  |   -3
  1  | 1 | 1 |   -3     |  no    | —           |    3
  2  | 2 | 1 |    3     |  yes   | y++, d-=10  |   -1
  3  | 3 | 2 |   -1     |  no    | —           |    5
  4  | 4 | 2 |    5     |  yes   | y++, d-=10  |    1
  5  | 5 | 3 |    —     |  —     | final pixel |   —

Result:
    3 |       ●
    2 |     ● ●
    1 | ● ●
    0 | ●
      +----------
        0 1 2 3 4 5
```

## Implementation

From `engine/graphics/framebuffer.cpp`:

```cpp
void Framebuffer::draw_line_bresenham(uint16_t x0, uint16_t y0,
                                       uint16_t x1, uint16_t y1,
                                       uint16_t color) {
    // Bounds check
    if (x0 >= DISPLAY_WIDTH || x1 >= DISPLAY_WIDTH ||
        y0 >= DISPLAY_HEIGHT || y1 >= DISPLAY_HEIGHT) return;

    uint16_t dx = abs(x1 - x0);
    uint16_t dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;  // Step direction X
    int sy = (y0 < y1) ? 1 : -1;  // Step direction Y

    if (dx >= dy) {
        // Shallow line: X is the fast axis
        int y = y0;
        int d = 2*dy - dx;
        for (int x = x0; x != x1; x += sx) {
            back_buffer[y * DISPLAY_WIDTH + x] = color;
            if (d > 0) {
                y += sy;
                d -= 2*dx;
            }
            d += 2*dy;
        }
    } else {
        // Steep line: Y is the fast axis
        int x = x0;
        int d = 2*dx - dy;
        for (int y = y0; y != y1; y += sy) {
            back_buffer[y * DISPLAY_WIDTH + x] = color;
            if (d > 0) {
                x += sx;
                d -= 2*dy;
            }
            d += 2*dx;
        }
    }
    // Draw final pixel
    back_buffer[y1 * DISPLAY_WIDTH + x1] = color;
}
```

### Key Implementation Details

1. **Direction handling** (`sx`, `sy`): Supports all 8 octants
2. **Axis selection**: Swaps logic for steep vs shallow lines
3. **Final pixel**: Explicitly drawn since loop uses `!=` comparison

## Diamond Drawing

Isometric tiles are diamond-shaped. Draw using 4 Bresenham lines:

```cpp
void Framebuffer::draw_diamond_outline(int cx, int cy,
                                        int w, int h,
                                        uint16_t color) {
    // Left → Bottom
    draw_line_bresenham(cx - w, cy, cx, cy + h, color);
    // Left → Top
    draw_line_bresenham(cx - w, cy, cx, cy - h, color);
    // Right → Bottom
    draw_line_bresenham(cx + w, cy, cx, cy + h, color);
    // Right → Top
    draw_line_bresenham(cx + w, cy, cx, cy - h, color);
}
```

```
         (cx, cy-h)
            /\
           /  \
(cx-w,cy) /    \ (cx+w,cy)
          \    /
           \  /
            \/
         (cx, cy+h)
```

For isometric projection, use **2:1 ratio** (width : height).

## Performance

On RP2040 at 125MHz:
- ~10-15 cycles per pixel (integer math only)
- 100-pixel line: ~1,500 cycles ≈ 12μs
- Suitable for real-time rendering at 60 FPS

## References

- Bresenham, J.E. (1965). "Algorithm for computer control of a digital plotter"
- Original paper used for plotters, now standard in all rasterization
