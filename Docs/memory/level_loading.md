# Level Loading Strategies

## Architecture Overview

**PocketGateEngine uses a cartridge-like system:**
- **Engine** → Stored in Flash (compiled into firmware)
- **Games** → Stored on SD card (swappable, no recompilation)
- **Level data** → Loaded from SD to RAM at level transitions

```
┌──────────────┐      Load Level      ┌──────────────┐      Gameplay      ┌──────────────┐
│   SD Card    │ ──────────────────→  │     RAM      │ ─────────────────→ │    Stack     │
│  (GB, slow)  │    (one-time, slow)  │ (264KB, fast)│  (per frame, fast) │ (temp refs)  │
└──────────────┘                      └──────────────┘                    └──────────────┘
  Game files                            Level data                         Local variables
```

**Key principle:** Load entire level from SD → Keep in RAM → Use stack references during gameplay

---

## Two Allocation Strategies

### Strategy 1: Fixed BSS Allocation

**Concept:** Pre-allocate a fixed-size buffer at compile time. All levels must fit within this limit.

#### Implementation

```cpp
// globals.h
#define MAX_LEVEL_SIZE (150 * 1024)  // 150KB limit
extern uint8_t level_data[MAX_LEVEL_SIZE];
extern size_t level_data_used;

// level_loader.cpp
uint8_t level_data[MAX_LEVEL_SIZE];  // BSS allocation (global)
size_t level_data_used = 0;

struct LevelHeader {
    uint32_t tilemap_size;
    uint32_t sprite_count;
    uint32_t sprite_data_size;
    uint32_t total_size;
};

bool load_level(const char* sd_path) {
    // Reset usage
    level_data_used = 0;

    // Read level header from SD
    LevelHeader header;
    if (!sd_read_header(sd_path, &header)) {
        return false;
    }

    // Validate size constraint
    if (header.total_size > MAX_LEVEL_SIZE) {
        // Level too large for hardware
        show_error("Level exceeds 150KB limit");
        return false;
    }

    // Load entire level into BSS buffer
    if (!sd_read_data(sd_path, level_data, header.total_size)) {
        return false;
    }

    level_data_used = header.total_size;

    // Parse loaded data (set pointers into level_data)
    parse_level(&header);

    return true;
}

void parse_level(const LevelHeader* header) {
    uint8_t* ptr = level_data;

    // Point to different sections (no new allocations!)
    tilemap = ptr;
    ptr += header->tilemap_size;

    sprite_data = (Sprite*)ptr;
    ptr += header->sprite_data_size;

    // etc...
}
```

#### Pros & Cons

| Pros                               | Cons                                            |
|------------------------------------|-------------------------------------------------|
| ✅ Zero fragmentation              | ❌ Fixed size limit (some levels may not fit)   |
| ✅ Deterministic memory usage      | ❌ Wastes RAM for small levels                  |
| ✅ No malloc/free complexity       | ❌ Must enforce size constraint in level design |
| ✅ Compile-time allocation (BSS)   |                                                 |

#### Memory Layout

```
BSS Section:
├─ level_data[150KB]  ← Pre-allocated at compile time
│   ├─ [Tilemap: 40KB]     (level 1 example)
│   ├─ [Sprites: 80KB]
│   ├─ [Metadata: 5KB]
│   └─ [Unused: 25KB]      ← Wasted for this level
```

#### When to Use

- **Cartridge-style design:** Like Game Boy, NES (enforced size limits)
- **Predictable performance:** No malloc overhead
- **Simple implementation:** Less error-prone
- **Conservative hardware:** Guaranteed to work

---

### Strategy 2: Dynamic Heap Allocation

**Concept:** Allocate exactly the size needed for each level. Free completely before loading next level.

#### Implementation

```cpp
// level_loader.cpp
struct LevelData {
    uint8_t* raw_data;      // Single heap allocation
    size_t data_size;

    // Pointers into raw_data (no extra allocations!)
    uint8_t* tilemap;
    Sprite* sprites;
    size_t sprite_count;
};

LevelData* current_level = nullptr;

bool load_level(const char* sd_path) {
    // CRITICAL: Full unload first (prevents fragmentation!)
    if (current_level) {
        free(current_level->raw_data);
        free(current_level);
        current_level = nullptr;
        // Heap is now clean and unfragmented
    }

    // Read level size from SD
    LevelHeader header;
    if (!sd_read_header(sd_path, &header)) {
        return false;
    }

    // Allocate exactly what we need (ONE allocation)
    current_level = (LevelData*)malloc(sizeof(LevelData));
    current_level->raw_data = (uint8_t*)malloc(header.total_size);
    current_level->data_size = header.total_size;

    if (!current_level->raw_data) {
        // Out of memory
        free(current_level);
        current_level = nullptr;
        return false;
    }

    // Load entire level from SD into single buffer
    if (!sd_read_data(sd_path, current_level->raw_data, header.total_size)) {
        free(current_level->raw_data);
        free(current_level);
        current_level = nullptr;
        return false;
    }

    // Parse: just set pointers, no new allocations!
    parse_level(current_level, &header);

    return true;
}

void parse_level(LevelData* level, const LevelHeader* header) {
    uint8_t* ptr = level->raw_data;

    // Point into the single allocated buffer
    level->tilemap = ptr;
    ptr += header->tilemap_size;

    level->sprites = (Sprite*)ptr;
    level->sprite_count = header->sprite_count;
    ptr += header->sprite_data_size;
}
```

#### Pros & Cons

| Pros                          | Cons                                 |
|-------------------------------|--------------------------------------|
| ✅ No wasted RAM (exact size) | ❌ Fragmentation risk if misused     |
| ✅ Flexible level sizes       | ❌ Malloc overhead (~16 bytes)       |
| ✅ Can handle large levels    | ❌ Must be careful with unload order |
| ✅ Better RAM utilization     | ❌ Slightly more complex             |

#### Memory Layout

```
Heap (Level 1 - 125KB):
├─ [LevelData struct: 32B]
└─ [raw_data: 125KB]
    ├─ [Tilemap: 40KB]
    ├─ [Sprites: 80KB]
    └─ [Metadata: 5KB]
[Free: ~137KB]

After unload → [Free: 264KB contiguous]

Heap (Level 2 - 200KB):
├─ [LevelData struct: 32B]
└─ [raw_data: 200KB]
    ├─ [Tilemap: 60KB]
    ├─ [Sprites: 135KB]
    └─ [Metadata: 5KB]
[Free: ~62KB]
```

#### When to Use

- **Variable level sizes:** Some levels 50KB, others 200KB
- **Maximum RAM efficiency:** Every byte counts
- **Modern design:** Flexible content creation
- **Strict discipline:** Team follows unload rules

---

## Fragmentation - The Critical Difference

### What Causes Fragmentation?

**❌ BAD Pattern - Fragmentation occurs:**
```cpp
// Multiple allocations
void* tilemap = malloc(50KB);   // Block 1
void* sprites = malloc(80KB);   // Block 2
void* sounds = malloc(30KB);    // Block 3

// Free middle block
free(sprites);

// Memory: [Tilemap:50KB][HOLE:80KB][Sounds:30KB][Free]
// Can't allocate 100KB even though 110KB is free!
```

**✅ GOOD Pattern - No fragmentation:**
```cpp
// Single allocation per level
void* level_data = malloc(160KB);  // One block

// Free everything
free(level_data);

// Memory: [Free: 264KB contiguous]
// Can allocate ANY size up to 264KB!
```

### The Safe Heap Rule

**If you follow this pattern, you will NEVER get fragmentation:**

```cpp
1. Free ALL previous level memory
2. Allocate ONE single block for new level
3. Never allocate during gameplay
```

**Why it works:**
- Heap is completely cleared between levels
- Only one allocation exists at a time
- Size changes don't matter (100KB → 200KB → 50KB all work)

---

## Gameplay Usage (Both Strategies)

Once level is loaded, **both strategies use the same gameplay pattern:**

```cpp
void game_loop() {
    while (running) {
        update_frame();   // No allocations
        render_frame();   // No allocations
    }
}

void render_frame() {
    // Access level data from RAM (BSS or Heap)
    // Use stack references only

    for (int i = 0; i < sprite_count; i++) {
        Sprite& s = sprites[i];  // Stack reference
        draw_sprite(framebuffer, s);  // No allocation
    }
}

void draw_sprite(uint16_t* fb, const Sprite& sprite) {
    // sprite data is already in RAM
    // Just copy pixels to framebuffer
    memcpy(fb + offset, sprite.pixels, sprite.size);
}
```

**Key rules:**
- ✅ Level data lives in RAM for entire level
- ✅ Use stack references during gameplay
- ✅ Never allocate/free during gameplay
- ✅ Only allocate at level transitions

---

## Comparison Table

| Feature            | BSS Fixed                   | Heap Dynamic                   |
|--------------------|-----------------------------|--------------------------------|
| **Max level size** | Fixed (e.g., 150KB)         | Flexible (up to available RAM) |
| **RAM waste**      | High (unused space)         | None (exact fit)               |
| **Fragmentation**  | Impossible                  | Possible if misused            |
| **Complexity**     | Low                         | Medium                         |
| **Level design**   | Must fit constraint         | Flexible sizes                 |
| **Performance**    | Slightly faster (no malloc) | Malloc overhead (~1ms)         |
| **Safety**         | Very safe                   | Safe if following rules        |

---

## Recommendations

**IMPORTANT: Pick ONE strategy - don't mix BSS and Heap!**

Mixing strategies wastes memory:
- BSS is always allocated (e.g., 100KB taken)
- If level overflows to heap, you use both (100KB BSS wasted + 150KB heap = 250KB total!)
- Worse than using either strategy alone

### Use BSS Fixed If:
- You want simple, bulletproof implementation
- You're okay with level size limits (like classic cartridge games)
- You prefer compile-time guarantees
- You're new to embedded systems
- **Design constraint:** "All levels must be ≤150KB" (enforced in level editor)

### Use Heap Dynamic If:
- You need flexible level sizes
- RAM efficiency is critical (different levels = different sizes)
- You have strict code discipline (always full unload before load)
- You understand fragmentation risks
- **Design freedom:** Levels can be any size up to available RAM

---

## Memory Budget Examples

### Example 1: BSS Strategy
```
RAM (264KB):
├─ Framebuffers: 80KB (double buffer)
├─ Stack: 16KB
├─ Engine data: 10KB
├─ Level data (BSS): 150KB (fixed)
└─ Available: 8KB
```

### Example 2: Heap Strategy (Small Level)
```
RAM (264KB):
├─ Framebuffers: 80KB
├─ Stack: 16KB
├─ Engine data: 10KB
├─ Level data (Heap): 80KB (loaded)
└─ Available: 78KB
```

### Example 2b: Heap Strategy (Large Level)
```
RAM (264KB):
├─ Framebuffers: 80KB
├─ Stack: 16KB
├─ Engine data: 10KB
├─ Level data (Heap): 180KB (loaded)
└─ Available: 0KB (but works!)
```

---

## Common Mistakes to Avoid

### ❌ Mistake 1: Multiple Allocations Per Level
```cpp
// DON'T DO THIS
level->tilemap = malloc(50KB);   // Allocation 1
level->sprites = malloc(80KB);   // Allocation 2
level->sounds = malloc(30KB);    // Allocation 3
// → Fragmentation risk!
```

### ❌ Mistake 2: Allocating During Gameplay
```cpp
void update() {
    // DON'T DO THIS
    Particle* p = new Particle();  // Every frame!
}
```

### ❌ Mistake 3: Partial Unload
```cpp
// DON'T DO THIS
free(level->sprites);  // Free only part
load_new_level();      // Fragmentation!

// DO THIS
free(level->raw_data);  // Free everything
load_new_level();       // Clean heap
```

### ❌ Mistake 4: BSS in Class (Non-Static)
```cpp
// DON'T DO THIS - not BSS!
class LevelManager {
    uint8_t buffer[150 * 1024];  // Where does this live?
};

// DO THIS - actual BSS
class LevelManager {
    static uint8_t buffer[150 * 1024];  // BSS!
};
```

---

## Level File Format (Recommended)

```
level.dat:
├─ [Header: 32 bytes]
│   ├─ uint32_t magic             // "LVL\0"
│   ├─ uint32_t version           // 1
│   ├─ uint32_t total_size        // For validation
│   ├─ uint32_t tilemap_size
│   ├─ uint32_t sprite_count
│   ├─ uint32_t sprite_data_size
│   └─ ... (padding to 32 bytes)
├─ [Tilemap data]
├─ [Sprite data]
└─ [Metadata]
```

---

## Performance Considerations

**Loading time (one-time cost):**
- SD read: ~200 µs/KB
- 150KB level: ~30ms to load (acceptable during transition)

**Gameplay performance (both strategies identical):**
- RAM access: <1 ns/byte
- 60 FPS: 16.67ms per frame
- No difference between BSS and Heap during gameplay

---

## See Also

- [RAM Overview](ram.md) - General RAM usage guidelines
- [SD Card Details](sd_card.md) - SD card access and performance
- [Flash Memory](flash.md) - Alternative for storing engine assets
