# Isometric Engine Implementation Plan

**Target**: Move from sprite-based rendering to full isometric engine
**Status**: Phase 1 - Single Tile

---

## Current State

**Working:**
- ✅ Display driver (ST7735, 128x160)
- ✅ Double buffering
- ✅ Sprite rendering with alpha
- ✅ Button input system
- ✅ Basic movement tracking

**Missing for Isometric:**
- ❌ World-to-screen coordinate conversion
- ❌ Tile rendering system
- ❌ Camera/viewport system
- ❌ Depth sorting for entities
- ❌ Isometric tile assets

---

## Phase 1: Single Diamond Tile

**Goal**: Render one diamond-shaped tile at screen center

### Tasks:

1. **Add angled line drawing function**
   - Location: `engine/graphics/framebuffer.h/cpp`
   - Implement Bresenham's line algorithm
   - Function signature: `void draw_line_angled(int x0, int y0, int x1, int y1, uint16_t color)`
   - Needed for: Drawing diamond outline, debug visualization

2. **Create diamond drawing helper**
   - Function: `void draw_diamond_outline(int center_x, int center_y, int width, int height, uint16_t color)`
   - Use draw_line_angled to draw 4 edges
   - Test with 32x16 dimensions

3. **Test in main.cpp**
   - Add `diamond_tile_test()` function
   - Draw diamond at screen center (64, 80)
   - Verify proportions look correct on display

**Success Criteria:**
- Diamond outline visible at screen center
- Proper 2:1 ratio (32x16 or adjusted for screen)
- No visual distortion

---

## Phase 2: Tile Grid (3x3)

**Goal**: Render 3x3 grid of diamond tiles in isometric formation

### Tasks:

1. **Implement coordinate conversion functions**
   - Location: New file `engine/isometric/iso_math.h/cpp`
   - Functions:
     ```cpp
     struct IsoPoint {
         int16_t screen_x;
         int16_t screen_y;
     };

     IsoPoint world_to_screen(int tile_x, int tile_y, int tile_width, int tile_height);
     ```

2. **Create tile grid renderer**
   - Function: `void render_tile_grid(int grid_width, int grid_height)`
   - Loop through tiles (0,0) to (2,2)
   - Convert each tile position to screen coordinates
   - Draw diamond outline at each position

3. **Test tile spacing**
   - Verify tiles connect properly (no gaps)
   - Check overlap creates proper isometric view
   - Adjust tile dimensions if needed

**Success Criteria:**
- 3x3 grid of diamonds visible
- Proper isometric formation (diamond grid)
- Tiles overlap correctly (back-to-front)

---

## Phase 3: Camera Offset

**Goal**: Add camera system and scrolling

### Tasks:

1. **Create camera structure**
   - Location: `engine/isometric/camera.h/cpp`
   - Structure:
     ```cpp
     struct Camera {
         float world_x;
         float world_y;
     };
     ```

2. **Update coordinate conversion**
   - Add camera offset to world_to_screen function
   - `screen_x -= camera.world_x`
   - `screen_y -= camera.world_y`

3. **Add camera movement**
   - Map button input to camera position
   - Test scrolling the tile grid
   - Add boundary checks

**Success Criteria:**
- Camera moves with button input
- Tile grid scrolls smoothly
- Proper boundary handling

---

## Phase 4: Filled Tiles

**Goal**: Replace wireframe diamonds with filled tile graphics

### Tasks:

1. **Create tile asset structure**
   - Location: `engine/isometric/tile.h`
   - Structure:
     ```cpp
     struct Tile {
         const uint16_t* pixel_data;  // Flash pointer
         uint8_t width;
         uint8_t height;
         bool walkable;
     };
     ```

2. **Create test tile graphics**
   - Make 2-3 simple tile images (32x16 diamonds)
   - Colors: grass (green), stone (gray), water (blue)
   - Use existing png_to_sprite tool to convert
   - Store in `assets/tiles/` directory

3. **Implement tile blitting**
   - Function: `void draw_isometric_tile(int screen_x, int screen_y, const Tile* tile)`
   - Use existing draw_sprite_alpha for rendering
   - Handle transparency properly

4. **Create tilemap structure**
   - Simple 3x3 array of tile IDs
   - `uint8_t test_map[3][3] = {{0,1,0}, {1,0,1}, {0,1,0}}`

**Success Criteria:**
- Filled diamond tiles instead of outlines
- Multiple tile types visible
- Proper transparency/overlap

---

## Phase 5: Sprite on Tile

**Goal**: Place character sprite on isometric tile

### Tasks:

1. **Create entity structure**
   - Location: `engine/isometric/entity.h`
   - Structure:
     ```cpp
     struct Entity {
         float world_x;
         float world_y;
         const SpritePixel* sprite_data;
         uint8_t sprite_width;
         uint8_t sprite_height;
     };
     ```

2. **Calculate sprite anchor point**
   - Determine sprite bottom-center position
   - Convert entity world position to screen position
   - Offset by sprite height for proper positioning

3. **Test sprite placement**
   - Place wizard sprite at tile (1,1)
   - Verify sprite appears "on" the tile
   - Check depth (sprite draws after tiles)

**Success Criteria:**
- Sprite positioned correctly on tile
- Sprite appears to stand ON tile (not floating)
- Proper render order (sprite after tiles)

---

## Phase 6: Entity Movement

**Goal**: Move entity with input, camera follows

### Tasks:

1. **Implement entity movement**
   - Map button input to world coordinate changes
   - W/S → world_y changes
   - A/D → world_x changes
   - Smooth sub-tile movement or grid-snapped (decide)

2. **Camera following**
   - Update camera to center on entity
   - `camera.world_x = entity.world_x - SCREEN_WIDTH/2`
   - `camera.world_y = entity.world_y - SCREEN_HEIGHT/2`

3. **Test gameplay feel**
   - Movement feels responsive
   - Camera follows smoothly
   - Sprite stays centered on screen

**Success Criteria:**
- Entity moves with input
- Camera follows entity
- Proper isometric movement directions
- 60 FPS maintained

---

## Phase 7: Depth Sorting

**Goal**: Multiple entities with correct overlap

### Tasks:

1. **Implement entity sorting**
   - Sort entities by (world_y + world_x) before rendering
   - Use simple bubble sort (few entities)
   - Location: `engine/isometric/depth_sort.h/cpp`

2. **Test with multiple entities**
   - Place 3-4 wizard sprites at different positions
   - Verify proper overlap (back entities behind front)
   - Test with moving entities

3. **Optimize render order**
   - Render tiles first (back-to-front)
   - Then render sorted entities
   - Ensure proper layering

**Success Criteria:**
- Multiple entities render correctly
- Proper occlusion (back-to-front)
- No visual artifacts
- Performance acceptable

---

## Phase 8: Larger World Map

**Goal**: Expand to larger map with viewport culling

### Tasks:

1. **Create larger tilemap**
   - 20x20 or 32x32 world map
   - Mix of different tile types
   - Store in Flash as const array

2. **Implement viewport culling**
   - Calculate visible tile range based on camera
   - Only render tiles within viewport + margin
   - Function: `void calculate_visible_tiles(Camera cam, int* start_x, int* end_x, int* start_y, int* end_y)`

3. **Optimize rendering loop**
   - Loop only through visible tiles
   - Skip off-screen checks
   - Profile performance

4. **Add world boundaries**
   - Camera bounds checking
   - Entity collision with map edges
   - Prevent walking off map

**Success Criteria:**
- Large map (>20x20) renders at 60 FPS
- Viewport culling working
- Only visible tiles rendered
- Proper boundary handling

---

## Phase 9: Collision System

**Goal**: Add walkable/non-walkable tiles

### Tasks:

1. **Add tile properties**
   - Mark tiles as walkable/blocked
   - Water = blocked, grass = walkable, etc.

2. **Implement collision detection**
   - Check target tile before movement
   - Function: `bool is_tile_walkable(int tile_x, int tile_y)`
   - Prevent entity from moving to blocked tiles

3. **Test collision**
   - Create test map with mixed walkable/blocked
   - Verify entity cannot walk through walls/water
   - Smooth collision response

**Success Criteria:**
- Entity respects tile walkability
- No walking through obstacles
- Smooth collision feel

---

## Phase 10: Multi-layer Rendering

**Goal**: Support ground layer + object layer

### Tasks:

1. **Add rendering layers**
   - Layer 0: Ground tiles
   - Layer 1: Ground objects (items, shadows)
   - Layer 2: Entities
   - Layer 3: Tall objects (trees, buildings)

2. **Implement layer rendering**
   - Render each layer in order
   - Maintain depth sorting within layers
   - Allow multi-tile tall objects

3. **Test layering**
   - Add tree sprites (tall objects)
   - Verify proper occlusion with entities
   - Test walking behind/in-front of objects

**Success Criteria:**
- Multiple rendering layers working
- Proper depth across layers
- Tall objects occlude correctly

---

## File Structure Plan

```
engine/
├── graphics/
│   ├── framebuffer.h          [EXISTING - extend with draw_line_angled]
│   └── framebuffer.cpp
│
├── isometric/                  [NEW]
│   ├── iso_math.h             [Phase 2 - coordinate conversion]
│   ├── iso_math.cpp
│   ├── camera.h               [Phase 3 - camera system]
│   ├── camera.cpp
│   ├── tile.h                 [Phase 4 - tile structure]
│   ├── tilemap.h              [Phase 4 - tilemap storage]
│   ├── tilemap.cpp
│   ├── entity.h               [Phase 5 - entity structure]
│   ├── entity.cpp
│   ├── depth_sort.h           [Phase 7 - sorting]
│   ├── depth_sort.cpp
│   ├── iso_renderer.h         [Phase 8 - main renderer]
│   └── iso_renderer.cpp
│
assets/
├── tiles/                      [NEW]
│   ├── grass_tile.h
│   ├── stone_tile.h
│   └── water_tile.h
│
game/                           [NEW - Phase 10+]
├── game_state.h
└── game_logic.cpp
```

---

## Performance Budget (per frame @ 60 FPS)

**Total budget**: 16.67ms

| Task | Target Time | Notes |
|------|-------------|-------|
| Clear framebuffer | 1-2ms | memset operation |
| Render visible tiles | 3-5ms | ~100 tiles with culling |
| Sort entities | <0.5ms | Few entities, simple sort |
| Render entities | 2-3ms | 10-20 sprites |
| Game logic | 2-3ms | Movement, collision |
| Send to display | 3-5ms | SPI transfer |
| **Total** | **~13-15ms** | 1-3ms margin |

---

## Current Task: Phase 1

**Next step**: Implement `draw_line_angled()` using Bresenham's algorithm in framebuffer.cpp

**Blockers**: Need to decide on initial tile dimensions (32x16 or smaller for 128x160 screen)

---

## Notes

- See `Docs/isometric/rendering.md` for coordinate conversion formulas
- See `Docs/isometric/depth_sorting.md` for entity sorting details
- Use existing `draw_sprite_alpha()` for all tile/sprite rendering
- Store all assets in Flash (const), not RAM
