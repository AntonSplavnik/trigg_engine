# Isometric Rendering

Isometric rendering creates a 2.5D perspective where the game world appears three-dimensional using 2D graphics.

---

## What is Isometric Projection?

**Isometric** means "equal measure" - all three axes (X, Y, Z) are drawn at equal angles (120° apart).

```
    Top view (world):          Isometric view (screen):

    y
    ↑                              (0,0)
    |                                 ╱╲
    |                                ╱  ╲
    +----→ x                        ╱    ╲
                                   ╱  🧍  ╲
                                  ╱        ╲
                                 ╱__________╲
```

---

## Projection Theory

### Projection vs Rasterization

These are related but distinct concepts:

- **Projection** — Mathematical transformation converting 3D world coordinates into 2D screen coordinates. Answers: "Where on screen does this 3D point appear?"

- **Rasterization** — Converting geometric primitives (lines, triangles) into actual pixels. Answers: "Which pixels need to be filled to draw this shape?"

Projection happens first (3D → 2D coordinates), then rasterization (2D shapes → pixels).

### Parallel Projection

All projection lines are parallel — no convergence to a vanishing point. Objects don't shrink with distance. This is the opposite of perspective projection.

### Axonometric Projections (Subcategory of Parallel)

Show multiple faces of an object at once by viewing from an angle:

| Type | Foreshortening | Angle |
|------|----------------|-------|
| **Isometric** | All 3 axes equal | ~30° (true) |
| **Dimetric** | 2 axes equal, 1 different | Varies |
| **Trimetric** | All 3 axes different | Varies |

### 2:1 Dimetric Projection (What Games Use)

What game developers call "isometric" is technically **dimetric projection** with a **2:1 pixel ratio**:

- For every 2 horizontal pixels, move 1 vertical pixel
- Creates clean diagonal lines without anti-aliasing
- Actual angle: ~26.565° (arctan(0.5)), not true 30°
- Perfect for pixel art and integer math

This became the standard in classic games (SimCity 2000, Diablo, Fallout, Age of Empires).

**Why 2:1 matters for embedded systems:**
- No floating point needed
- Simple bit shifts instead of division
- Clean pixel-perfect lines

---

## Diamond-Shaped Tiles

**Standard isometric tile dimensions:** 32×16 pixels

```
      Width: 32 pixels
         ┌──────┐
         ╱╲      Height: 16 pixels
        ╱  ╲
       ╱    ╲    ↕
      ╱______╲
```

**Why 32×16?**
- 2:1 ratio (width is 2× height)
- Creates proper isometric angle (26.565°)
- Divides evenly (power of 2 dimensions)

---

## Coordinate Conversion

### World Coordinates → Screen Coordinates

**Formula:**
```cpp
screen_x = (world_x - world_y) * (tile_width / 2)
screen_y = (world_x + world_y) * (tile_height / 2)
```

**With 32×16 tiles:**
```cpp
screen_x = (world_x - world_y) * 16;
screen_y = (world_x + world_y) * 8;
```

**Example:**
```cpp
// World tile at (3, 2)
int world_x = 3;
int world_y = 2;

// Convert to screen position
int screen_x = (3 - 2) * 16 = 16;   // 1 * 16
int screen_y = (3 + 2) * 8  = 40;   // 5 * 8

// Draw tile at pixel (16, 40)
drawTile(screen_x, screen_y, grass_tile);
```

---

## Screen Coordinates → World Coordinates (Mouse Picking)

**Formula:**
```cpp
world_x = (screen_x / (tile_width/2) + screen_y / (tile_height/2)) / 2
world_y = (screen_y / (tile_height/2) - screen_x / (tile_width/2)) / 2
```

**With 32×16 tiles:**
```cpp
world_x = (screen_x / 16 + screen_y / 8) / 2;
world_y = (screen_y / 8 - screen_x / 16) / 2;
```

**Example:**
```cpp
// User taps pixel (16, 40)
int screen_x = 16;
int screen_y = 40;

// Convert to world tile
int world_x = (16/16 + 40/8) / 2 = (1 + 5) / 2 = 3;
int world_y = (40/8 - 16/16) / 2 = (5 - 1) / 2 = 2;

// User tapped tile (3, 2)
```

---

## Rendering a Tilemap

### Simple Approach (Render All Tiles):

```cpp
void renderMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Get tile ID from tilemap
            uint8_t tile_id = tilemap[x + y * MAP_WIDTH];

            // Convert to screen coordinates
            int screen_x = (x - y) * 16;
            int screen_y = (x + y) * 8;

            // Get tile sprite from Flash
            const uint16_t* tile = getTile(tile_id);

            // Draw tile to framebuffer
            blitTile(framebuffer, tile, screen_x, screen_y);
        }
    }
}
```

---

## Camera/Viewport

The screen is 128×160 pixels, so you can't show the entire map at once.

### Adding Camera Offset:

```cpp
// Camera position in world coordinates
int camera_x = 10;
int camera_y = 10;

void renderMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Convert to screen coordinates
            int screen_x = (x - y) * 16 - camera_x;
            int screen_y = (x + y) * 8 - camera_y;

            // Skip if off-screen (culling)
            if (screen_x < -32 || screen_x > 128) continue;
            if (screen_y < -16 || screen_y > 160) continue;

            // Draw tile
            uint8_t tile_id = tilemap[x + y * MAP_WIDTH];
            blitTile(framebuffer, getTile(tile_id), screen_x, screen_y);
        }
    }
}

// Move camera to follow player
void updateCamera() {
    camera_x = player_x * 16 - 64;  // Center on player
    camera_y = player_y * 8 - 80;
}
```

---

## Culling (Performance Optimization)

**Don't draw tiles that are off-screen:**

```cpp
// Calculate visible tile range
int min_tile_x = (camera_x / 16) - 2;
int max_tile_x = min_tile_x + (128 / 16) + 4;
int min_tile_y = (camera_y / 8) - 2;
int max_tile_y = min_tile_y + (160 / 8) + 4;

// Clamp to map bounds
min_tile_x = max(0, min_tile_x);
max_tile_x = min(MAP_WIDTH, max_tile_x);
min_tile_y = max(0, min_tile_y);
max_tile_y = min(MAP_HEIGHT, max_tile_y);

// Only render visible tiles
for (int y = min_tile_y; y < max_tile_y; y++) {
    for (int x = min_tile_x; x < max_tile_x; x++) {
        // Render tile...
    }
}
```

**Performance gain:** Instead of rendering 64×64 = 4096 tiles, you only render ~10×10 = 100 tiles per frame!

---

## Render Order (Critical!)

**Wrong order causes visual artifacts:**

```
❌ BAD: Render left-to-right, top-to-bottom
┌─┬─┬─┐
│1│2│3│  Tile 3 draws over tile 2 (wrong!)
├─┼─┼─┤
│4│5│6│
└─┴─┴─┘
```

**Correct order: Back-to-front (top-left to bottom-right)**

```
✅ GOOD: Render diagonally
  1
 ╱ ╲
2   3
 ╲ ╱ ╲
  4   5
   ╲ ╱
    6
```

**Implementation:**
```cpp
// Render from top-left to bottom-right
for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
        renderTile(x, y);
    }
}

// This naturally renders back-to-front for isometric!
```

**See also:** [Depth Sorting](depth_sorting.md) for entities

---

## Example: Complete Renderer

```cpp
#define MAP_WIDTH 64
#define MAP_HEIGHT 64
#define TILE_WIDTH 32
#define TILE_HEIGHT 16

uint8_t tilemap[MAP_WIDTH * MAP_HEIGHT];
int camera_x = 0, camera_y = 0;

void renderIsometricMap() {
    // Calculate visible range
    int start_x = max(0, (camera_x / 16) - 2);
    int end_x = min(MAP_WIDTH, start_x + 10);
    int start_y = max(0, (camera_y / 8) - 2);
    int end_y = min(MAP_HEIGHT, start_y + 20);

    // Render visible tiles back-to-front
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            // World to screen
            int screen_x = (x - y) * 16 - camera_x;
            int screen_y = (x + y) * 8 - camera_y;

            // Skip if completely off-screen
            if (screen_x < -32 || screen_x > 128) continue;
            if (screen_y < -16 || screen_y > 160) continue;

            // Get tile and render
            uint8_t tile_id = tilemap[x + y * MAP_WIDTH];
            const uint16_t* tile_data = getTile(tile_id);
            blitIsometricTile(framebuffer, tile_data, screen_x, screen_y);
        }
    }
}
```

---

## Performance Target

From CLAUDE.md:
- **60 FPS with 10×10 visible tiles**
- **10-20 entities on screen**

**Frame budget:** 16.67ms per frame

**Tile rendering time:**
- 100 tiles × 3µs (Flash read) = 300µs
- Leaves 16.37ms for game logic, entities, display transfer

---

## Common Mistakes

### 1. Wrong Tile Dimensions
```cpp
// ❌ BAD: Not 2:1 ratio
#define TILE_WIDTH 32
#define TILE_HEIGHT 32  // Creates square, not isometric!

// ✅ GOOD: 2:1 ratio
#define TILE_WIDTH 32
#define TILE_HEIGHT 16
```

### 2. Wrong Render Order
```cpp
// ❌ BAD: Bottom-up causes overlap issues
for (int y = MAP_HEIGHT-1; y >= 0; y--) { ... }

// ✅ GOOD: Top-down renders back-to-front
for (int y = 0; y < MAP_HEIGHT; y++) { ... }
```

### 3. No Culling
```cpp
// ❌ BAD: Renders entire 64×64 map every frame (4096 tiles!)
for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) { ... }
}

// ✅ GOOD: Only render visible ~10×10 tiles (100 tiles)
for (int y = start_y; y < end_y; y++) { ... }
```

---

## Next Steps

- **[Depth Sorting](depth_sorting.md)** - Rendering entities with correct overlap
- **CLAUDE.md** - See isometric rendering section for more details

---

## Reference

**Coordinate conversion formulas:**
```cpp
// World → Screen
screen_x = (world_x - world_y) * (TILE_WIDTH / 2);
screen_y = (world_x + world_y) * (TILE_HEIGHT / 2);

// Screen → World
world_x = (screen_x / (TILE_WIDTH/2) + screen_y / (TILE_HEIGHT/2)) / 2;
world_y = (screen_y / (TILE_HEIGHT/2) - screen_x / (TILE_WIDTH/2)) / 2;
```

**Standard tile size:** 32×16 pixels (2:1 ratio)
