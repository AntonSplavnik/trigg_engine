# Isometric Rendering: Core Concepts

**Target**: PocketGateEngine on Raspberry Pi Pico (C++11, 128x160 ST7735 display)

---

## 1. The Coordinate System Problem

Currently the engine thinks in **screen space**: pixel (x, y) on display.

Isometric needs **world space**: where objects exist in the game world independent of camera position.

**Three coordinate systems:**
- **World coordinates**: Where objects actually are (logical grid: tile 5,3)
- **Isometric coordinates**: 2D projection of 3D-like world
- **Screen coordinates**: Final pixel position on display

---

## 2. Isometric Projection (The Core Trick)

Imagine looking at a chess board from above and tilted 45 degrees. That's isometric.

**What it does:**
- Takes a square grid and rotates it 45°
- Squashes vertical axis by ~50%
- Creates diamond-shaped tiles
- Simulates depth without true 3D math

**Visual transformation:**
```
Square tile (top view):        Diamond tile (isometric):
    ┌──┐                             ╱╲
    │  │                            ╱  ╲
    └──┘                            ╲  ╱
                                     ╲╱
```

**Standard tile dimensions:**
- Diamond: 32x16 pixels (2:1 ratio)
- On 128x160 screen: ~10 tiles visible horizontally

---

## 3. The Rendering Order Problem (Z-sorting)

**Critical concept**: Back-to-front rendering.

In isometric view, objects further "back" must be drawn first, objects in "front" drawn last to create proper overlap.

**Depth sorting rule:**
- Objects at higher (world_y + world_x) values are "closer" to camera
- Render tiles/sprites starting from top-left of world, ending bottom-right
- This naturally creates correct occlusion

**Example:**
```
    [Tile 0,0] ← Draw first (back)
       ╱╲
      ╱  ╲──[Tile 1,0]
     ╱ P  ╱╲
    ╱    ╱  ╲
   ╲    ╱  P ╱
    ╲  ╱    ╱
     ╲╱────╱ ← Draw last (front)
   [Tile 0,1]  [Tile 1,1]

P = Player sprites (drawn after tiles in their layer)
```

**Render order:**
1. Tile[0,0]
2. Tile[1,0]
3. Tile[0,1]
4. Tile[1,1]
5. Sprites sorted by (world_y + world_x)

---

## 4. World-to-Screen Coordinate Conversion

**The core mathematical transformation:**

Given a tile at world position (tile_x, tile_y), calculate screen position:

```
Basic formula (without camera offset):
screen_x = (tile_x - tile_y) * (tile_width / 2)
screen_y = (tile_x + tile_y) * (tile_height / 2)

With camera offset:
screen_x = (tile_x - tile_y) * (tile_width / 2) - camera_x
screen_y = (tile_x + tile_y) * (tile_height / 2) - camera_y
```

**Example with 32x16 tiles:**
- Tile at world (0,0) → screen (0, 0)
- Tile at world (1,0) → screen (16, 8)
- Tile at world (0,1) → screen (-16, 8)
- Tile at world (1,1) → screen (0, 16)

---

## 5. Camera/Viewport System

You can't render the entire world on a 128x160 screen.

**Viewport concept:**
- World is larger than screen (e.g., 50x50 tiles)
- Camera follows player
- Only render tiles visible in camera's viewport
- Calculate which tiles are visible based on camera position

**World vs Screen:**
```
World (large):              Screen sees (small):
┌─────────────┐              ┌───┐
│ ░░░░░░░░░░░ │              │╱╲ │ ← Visible portion
│ ░░░░┌───┐░░ │              │ @│   (camera viewport)
│ ░░░░│╱╲ │░░ │ Camera →    └───┘
│ ░░░░│ @ │░░ │
│ ░░░░└───┘░░ │
│ ░░░░░░░░░░░ │
└─────────────┘
```

**Viewport culling:**
- Calculate visible tile range based on camera position
- Only iterate tiles within screen bounds + margin
- Skip rendering tiles outside viewport
- Critical for performance on limited hardware

---

## 6. Sprite Positioning in Isometric Space

**Problem**: Sprites are still rectangular images.
**Solution**: Position them in screen space based on world coordinates.

**Concept:**
- Sprite's world position (world_x, world_y) converts to screen position
- Sprite "anchor point" typically at bottom-center (character's feet)
- This creates illusion sprite is standing ON the tile
- Sprites sort with tiles based on world position

**Anchor point example:**
```
    Sprite image:          Positioned on tile:
    ┌─────┐                     ╱╲
    │ /^\ │                    ╱  ╲
    │ |█| │                   │ /^\│
    │ | | │                   │ |█|│
    │ └─┘ │                   │ | |│
    └──●──┘ ← Anchor          ╲ └─┘╱ ← Feet on tile
                               ╲  ╱
                                ╲╱
```

---

## 7. Layering Architecture

**Rendering order (back-to-front within each layer):**

1. **Ground layer**: Floor tiles (grass, stone, water)
2. **Ground objects**: Items on floor, shadows
3. **Entities**: Characters, enemies sorted by (world_y + world_x)
4. **Tall objects**: Trees, buildings (may span multiple tiles)
5. **Effects/UI**: Particles, interface elements

Each layer respects the depth sorting rule within itself.

---

## 8. Movement in Isometric

**Conceptual difference from current sprite movement:**

**Currently**:
- Sprite moves in screen pixels (x += 1 means move right)

**Isometric**:
- Input translates to world coordinate changes
- World position updates (tile_x++, tile_y++)
- Position converts to screen coordinates
- Screen coordinates animate between positions

**Input mapping example:**
```
Press UP    → world_y decreases → moves diagonal up-right on screen
Press DOWN  → world_y increases → moves diagonal down-left on screen
Press LEFT  → world_x decreases → moves diagonal up-left on screen
Press RIGHT → world_x increases → moves diagonal down-right on screen
```

**Movement types:**
- **Grid-based**: Snap to tile positions (simpler, classic RPG style)
- **Free movement**: Smooth sub-tile positioning (more fluid)

---

## 9. Memory Considerations (Critical for Pico)

**Tile data storage:**
- Store tile graphics as const arrays in Flash
- Don't duplicate tile graphics in RAM
- Reference same tile data for multiple map positions

**Example:**
```cpp
// In Flash (no RAM cost)
const uint16_t grass_tile[32*16] = { /* RGB565 pixel data */ };
const uint16_t stone_tile[32*16] = { /* RGB565 pixel data */ };

// World map in RAM (minimal cost)
// 50x50 map = 2,500 bytes for tile IDs only
uint8_t world_map[50][50] = {
    {0, 0, 1, 1, ...},  // 0=grass, 1=stone
    {0, 1, 1, 0, ...},
    ...
};
```

**Viewport rendering:**
- Calculate visible tile range
- Only iterate visible tiles (not entire map)
- Example: 128x160 screen shows ~12x8 tiles
- Loop over 96 tiles instead of 2,500 tiles

---

## 10. The Implementation Mental Model

Think of it as three interconnected systems:

**System 1: World State** (game logic)
- Where entities are located
- What tiles exist at each position
- Game rules and collisions
- Independent of rendering

**System 2: Coordinate Conversion** (math layer)
- Translates world → screen coordinates
- Handles camera offset
- Determines render order
- Calculates viewport bounds

**System 3: Renderer** (graphics)
- Takes sorted list of things to draw
- Blits sprites/tiles to framebuffer
- Respects depth order
- Uses existing draw_sprite_alpha() functions

---

## 11. Data Structures

**Tile definition:**
```cpp
struct Tile {
    const uint16_t* pixel_data;  // Pointer to Flash data
    uint8_t width;
    uint8_t height;
    bool walkable;  // For collision
};
```

**World map:**
```cpp
struct WorldMap {
    uint8_t* tiles;     // 2D array of tile IDs
    uint16_t width;
    uint16_t height;
};
```

**Entity (character/object):**
```cpp
struct Entity {
    float world_x;      // World position (can be sub-tile)
    float world_y;
    const SpritePixel* sprite;
    uint8_t sprite_width;
    uint8_t sprite_height;
};
```

**Camera:**
```cpp
struct Camera {
    float world_x;      // Camera position in world
    float world_y;
    uint16_t screen_width;
    uint16_t screen_height;
};
```

---

## 12. Phased Implementation Strategy

**Phase 1: Single Tile**
- Create one diamond tile (32x16)
- Render it at screen center
- No world coordinates yet
- Goal: Prove isometric shape displays correctly

**Phase 2: Tile Grid**
- Render 3x3 grid of tiles
- Implement world-to-screen conversion
- Hard-code camera at (0,0)
- Goal: Prove coordinate math works

**Phase 3: Camera Offset**
- Add camera position
- Offset all tile positions by camera
- Add button input to move camera
- Goal: Prove scrolling works

**Phase 4: Sprite Placement**
- Place one sprite on center tile
- Calculate sprite anchor point
- Ensure sprite renders after tiles
- Goal: Prove sprite positioning works

**Phase 5: Sprite Movement**
- Add entity world position
- Map input to world coordinate changes
- Camera follows entity
- Goal: Prove gameplay mechanics work

**Phase 6: Depth Sorting**
- Add multiple entities
- Sort by (world_y + world_x)
- Render in sorted order
- Goal: Prove occlusion works correctly

**Phase 7: Full World**
- Larger tile map (20x20 or more)
- Viewport culling
- Only render visible tiles
- Goal: Prove performance is acceptable

---

## 13. Key Conceptual Differences

| Current Engine            | Isometric Engine             |
|---------------------------|------------------------------|
| Screen coordinates only   | World → Screen conversion    |
| No depth sorting          | Back-to-front rendering      |
| Direct sprite positioning | Tile grid + entity placement |
| No camera                 | Viewport + camera following  |
| Draw order = code order   | Draw order = world position  |
| Full-screen rendering     | Viewport culling             |

---

## 14. Performance Targets

With 128x160 @ 60 FPS on Pico:

**Frame budget**: 16.67ms per frame
- Clear framebuffer: ~1-2ms
- Render 12x8 visible tiles: ~5-8ms
- Render 10-20 sprites: ~3-5ms
- Sort entities: <1ms
- Send to display: ~3-5ms
- **Total: ~13-15ms** (leaves 1-3ms margin)

**Optimization priorities:**
1. Viewport culling (don't draw off-screen)
2. Batch memcpy for tiles
3. Minimize sort operations
4. Cache coordinate conversions
5. Use fixed-point math if needed

---

## 15. Next Steps

1. Study existing framebuffer code (already has draw_sprite_alpha)
2. Create isometric tile graphics (32x16 diamonds)
3. Implement coordinate conversion functions
4. Start with Phase 1: Single tile rendering
5. Iterate through phases as each proves successful

---

## References

**Games using similar sprite-based isometric (hand-drawn 2D art):**
- Fallout 1 & 2 - Hand-drawn sprites with free movement
- GBA: Lord of the Rings - The Two Towers - Hand-drawn sprites
- StarCraft - Hand-drawn sprites and tiles
- Final Fantasy Tactics - Hand-drawn sprites on isometric grid

**Games using pre-rendered 3D to 2D sprites (same engine concept, different art pipeline):**
- Diablo 1 & 2 - 3D models rendered to sprite sheets, but engine runs 2D sprite-based isometric at runtime
- Baldur's Gate series - Pre-rendered 3D environments, sprite-based characters
- Age of Empires II - 3D models pre-rendered to sprites
- Note: These use the SAME type of 2D sprite engine we're building, just used 3D software to create the artwork

**Different architecture (not applicable to this engine):**
- Diablo 3 - True real-time 3D polygons with isometric camera angle
- SimCity 2000 - Pure tile-based, no free-moving sprites/entities
- Civilization series - Grid-based tile system without free sprite positioning
