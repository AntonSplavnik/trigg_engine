# SD Card Storage (Optional)

## What is an SD Card?

An SD card provides **massive non-volatile storage** (gigabytes) that can be swapped by the user. It's connected via the SPI bus and shares pins with the display.

**Physical location:** SD card slot on display module

---

## Characteristics

| Property | Value |
|----------|-------|
| **Size** | Gigabytes (massive) |
| **Read speed** | ~50-100 microseconds per byte |
| **Write speed** | ~100-200 microseconds per byte |
| **Seek time** | 10-50ms to find data |
| **Write cycles** | ~10,000-100,000 (cheap cards) |
| **Access method** | SPI protocol + FatFS filesystem |
| **Volatile?** | No - keeps data when powered off |
| **User-modifiable?** | Yes - can swap cards |

---

## Hardware Connection

The SD card shares the SPI bus with the display:

```cpp
// Shared pins (both display and SD card)
#define PIN_SCK     2      // SPI Clock
#define PIN_MOSI    3      // SPI Data Out (to SD card)
#define PIN_MISO    19     // SPI Data In (from SD card)

// Separate chip selects
#define PIN_TFT_CS  20     // Display select
#define PIN_CARD_CS 21     // SD card select
```

**Important:** Only set ONE chip select LOW at a time!

```cpp
// Select SD card
gpio_put(PIN_TFT_CS, 1);   // Display OFF
gpio_put(PIN_CARD_CS, 0);  // SD card ON

// Read from SD card...

// Select display
gpio_put(PIN_CARD_CS, 1);  // SD card OFF
gpio_put(PIN_TFT_CS, 0);   // Display ON
```

---

## Speed Problem: SD is 50-100× Slower Than Flash

### Reading 1KB of data:

```
Flash:    ~3 microseconds
SD Card:  ~150 microseconds  (50× slower!)
```

### Why so slow?

1. **Seek time:** 10-50ms to find data on card
2. **Protocol overhead:** SPI + FatFS filesystem
3. **Lower SPI speed:** 12.5 MHz (vs 62.5 MHz for display)

**This is why you CAN'T read sprites from SD at 60 FPS!**

---

## When SD Card is NEEDED

### ✅ Use SD Card For:

**1. Background Music (Large Files)**
```cpp
// Music files are 3-5MB - too big for Flash!
Background music (1 min, WAV):  3-5MB

// Stream from SD card in chunks
void playMusic() {
    FIL file;
    uint8_t buffer[4096];

    f_open(&file, "music/level1.wav", FA_READ);

    while(playing) {
        f_read(&file, buffer, 4096, &bytes_read);
        // Send to audio DAC...
    }

    f_close(&file);
}
```

**2. User-Generated Content**
```cpp
// Players create levels on PC, copy to SD card
f_open(&file, "custom_levels/player_level.dat", FA_READ);
f_read(&file, &level, sizeof(Level), &bytes_read);
```

**3. Development Workflow**
```
Edit level1.dat on computer
  ↓
Copy to SD card
  ↓
Test in game immediately
  ↓
No recompilation needed!
```

**4. DLC/Expansion Packs**
```cpp
// Ship game with 20 levels in Flash
// Later add 10 more levels on SD card
if (sd_card_detected()) {
    load_expansion_levels();
}
```

---

## When SD Card is NOT Needed

### ❌ Don't Use SD Card For:

**1. Sprites/Tiles (Too Slow for 60 FPS)**
```cpp
// ❌ IMPOSSIBLE: Too slow
void drawTile() {
    f_open(&file, "tiles/grass.bin", FA_READ);  // 10ms!
    f_read(&file, tile, 1024, &br);             // 150µs
    f_close(&file);                              // 5ms

    blitTile(framebuffer, tile, x, y);
}

// Drawing 10 tiles: 150ms per frame
// Target: 16.67ms (60 FPS) ← IMPOSSIBLE!
```

**2. Level Maps (Fit in Flash)**
```
100 levels × 11KB = 1.1MB
Flash has 2MB available ← Plenty of space!
```

**3. Save Games (Fit in Flash)**
```
10 save slots × 1KB = 10KB
Easily fits in Flash
```

---

## Using FatFS Library

**Popular library:** [no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)

### Basic Setup:

```cpp
#include "ff.h"
#include "hw_config.h"

// Configure SD card SPI
static sd_spi_if_t spi_if = {
    .spi = spi0,
    .miso_gpio = 19,
    .mosi_gpio = 3,
    .sck_gpio = 2,
    .baud_rate = 12500000  // 12.5 MHz (slower than display)
};

static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if,
    .ss_gpio = 21  // PIN_CARD_CS
};

// Mount filesystem
FATFS fs;
FRESULT res = f_mount(&fs, "0:", 1);

if (res == FR_OK) {
    // SD card ready!
}
```

### Reading a File:

```cpp
FIL file;
UINT bytes_read;
uint8_t buffer[1024];

// Open file
f_open(&file, "0:levels/level1.dat", FA_READ);

// Read data
f_read(&file, buffer, sizeof(buffer), &bytes_read);

// Close file
f_close(&file);
```

### Writing a File:

```cpp
FIL file;
UINT bytes_written;
SaveData save = { player_x, player_y, score, ... };

// Open file for writing
f_open(&file, "0:saves/save1.dat", FA_WRITE | FA_CREATE_ALWAYS);

// Write data
f_write(&file, &save, sizeof(SaveData), &bytes_written);

// Close file
f_close(&file);
```

---

## Loading Levels from SD During Loading Screen

**This is acceptable because user expects wait time:**

```cpp
void loadLevel(int level_num) {
    // 1. Show loading screen
    drawText(framebuffer, "Loading...", 10, 10, 0xFFFF);
    sendFramebuffer();

    // 2. Load from SD (50-100ms - OK during loading!)
    char filename[32];
    sprintf(filename, "0:levels/level%d.dat", level_num);

    FIL file;
    f_open(&file, filename, FA_READ);
    f_read(&file, &current_level, sizeof(Level), &bytes_read);
    f_close(&file);
    // ↑ 50-100ms delay is acceptable here

    // 3. Initialize entities
    spawnEntities(&current_level);

    // 4. Start gameplay (now reading from RAM at 60 FPS)
    gameloop();
}
```

---

## SPI Speed Consideration

**Display:** 62.5 MHz (main.cpp:9)
**SD Card:** Typically 12.5-25 MHz

**Problem:** Both share the same SPI bus!

### Solution 1: Reconfigure SPI Speed When Switching
```cpp
void selectDisplay() {
    spi_set_baudrate(spi0, 62500000);  // 62.5 MHz
    gpio_put(PIN_CARD_CS, 1);
    gpio_put(PIN_TFT_CS, 0);
}

void selectSDCard() {
    spi_set_baudrate(spi0, 12500000);  // 12.5 MHz
    gpio_put(PIN_TFT_CS, 1);
    gpio_put(PIN_CARD_CS, 0);
}
```

### Solution 2: Use Separate SPI Controllers
```cpp
// Display on spi0
spi_init(spi0, 62500000);

// SD card on spi1 (different pins)
spi_init(spi1, 12500000);
```

---

## Recommended Strategy

### Phase 1-3: NO SD Card
```
Flash (2MB):
├─ Engine code, sprites, tiles
├─ 100 levels (1.1MB)
└─ Save games (10KB)

Everything fits in Flash!
SD card not needed yet.
```

### Phase 4+: Add SD Card (Optional)
```
Flash (2MB):
├─ Engine + core assets (same as above)

SD Card (GB):
├─ Background music:    5 tracks × 4MB = 20MB
├─ User custom levels:  Variable
└─ Save backups:        Optional
```

---

## Performance Example

**Drawing tiles at 60 FPS:**

```cpp
// ❌ From SD card: IMPOSSIBLE
void drawMap_FromSD() {
    for(int i = 0; i < 100; i++) {
        f_open(...);                  // 10ms × 100 = 1000ms!
        f_read(&file, tile, 1KB);     // 150µs × 100 = 15ms
        f_close(...);                 // 5ms × 100 = 500ms
        blitTile(framebuffer, tile, x, y);
    }
}
// Total: 1515ms per frame ← Impossible at 60 FPS!

// ✅ From Flash: WORKS
void drawMap_FromFlash() {
    for(int i = 0; i < 100; i++) {
        const uint16_t* tile = tiles[i];  // 3µs × 100 = 300µs
        blitTile(framebuffer, tile, x, y);
    }
}
// Total: 300µs per frame ← Easy at 60 FPS!
```

---

## Multi-Game Architecture (Cartridge System)

### The Problem

**How do you develop and play multiple games without reflashing?**

- Flash is read-only at runtime (only writable via USB/SWD programmer)
- You can't install games to flash during gameplay
- Reflashing for each game is cumbersome for development/testing

### The Solution: Engine + Data Architecture

**Treat the system like a console (Game Boy, NES, etc.):**

```
Flash (2MB):                    SD Card (GB):
├── Core Engine (Fixed)         ├── game1/
│   ├── Renderer                │   ├── tilemap.dat
│   ├── Physics                 │   ├── sprites.dat
│   ├── Collision               │   ├── entities.dat
│   ├── Input handling          │   └── logic.dat
│   └── Display drivers         ├── game2/
├── Game Loader/Menu            │   ├── tilemap.dat
└── SD Card Interface           │   ├── sprites.dat
                                │   └── ...
                                └── game3/
```

**Workflow:**
1. Boot → Display game selection menu
2. User selects game from SD card
3. Engine loads game data into RAM
4. Game runs at full speed (engine code in Flash, data in RAM)

---

### Performance Analysis

#### ✅ **No Performance Impact During Gameplay**

**Hot loops run from Flash/RAM at full speed:**

```cpp
// Game loop (runs 60 FPS from Flash)
void gameLoop() {
    handleInput();      // From Flash
    updateEntities();   // From Flash, data in RAM
    renderIsometric();  // From Flash, tiles in RAM
    sendFramebuffer();  // From Flash
}
```

**Only loading is slower (acceptable):**

```cpp
// ONE-TIME load at game start
void loadGame(const char* gamePath) {
    // Load from SD (slow, but happens once)
    tilemap = sd.loadFile("game1/tilemap.dat");    // ~10ms
    sprites = sd.loadFile("game1/sprites.dat");    // ~20ms
    entities = sd.loadFile("game1/entities.dat");  // ~5ms

    // Total: ~35ms load time
    // Then 60 FPS gameplay from RAM/Flash
}
```

#### Performance Breakdown

| Operation | Location | Speed |
|-----------|----------|-------|
| **Engine code** | Flash (XIP) | ~100 MB/s |
| **Game loop** | Flash | 60 FPS (16.67ms/frame) |
| **Active assets** | RAM | CPU speed (~133 MHz) |
| **Level loading** | SD Card → RAM | 2-4 MB/s (one-time) |

**SD card read speeds:**
- Small file (10KB): ~5-10ms
- Tilemap (50KB): ~20-30ms
- Sprite sheet (100KB): ~40-60ms

**Total game load time: 50-100ms** (barely noticeable, like cartridge boot)

---

### Architecture Implementation

#### Game Data Format Example

```cpp
// game1/manifest.dat
struct GameManifest {
    char name[32];           // "Adventure Quest"
    uint8_t version;         // 1
    uint16_t tileCount;      // 20
    uint16_t spriteCount;    // 50
    uint32_t tilemapSize;    // Bytes
    uint32_t spriteDataSize; // Bytes
};

// game1/tilemap.dat
struct Tilemap {
    uint8_t width;
    uint8_t height;
    uint16_t tiles[];  // width × height tile IDs
};

// game1/sprites.dat
// Raw RGB565 pixel data for all sprites
```

#### Game Loader

```cpp
class GameLoader {
public:
    bool loadGame(const char* gamePath) {
        char manifestPath[64];
        sprintf(manifestPath, "%s/manifest.dat", gamePath);

        // Read manifest
        GameManifest manifest;
        if (!sd.readFile(manifestPath, &manifest, sizeof(manifest))) {
            return false;
        }

        // Load tilemap
        sprintf(buffer, "%s/tilemap.dat", gamePath);
        currentTilemap = loadTilemap(buffer);

        // Load sprites
        sprintf(buffer, "%s/sprites.dat", gamePath);
        currentSprites = loadSprites(buffer, manifest.spriteCount);

        // Load entities
        sprintf(buffer, "%s/entities.dat", gamePath);
        currentEntities = loadEntities(buffer);

        return true;
    }
};
```

#### Boot Menu

```cpp
void bootMenu() {
    // Scan SD card for games
    std::vector<GameInfo> games = scanGamesFolder();

    // Display menu
    int selected = 0;
    while (true) {
        drawMenu(games, selected);

        if (buttonPressed(BTN_UP))    selected--;
        if (buttonPressed(BTN_DOWN))  selected++;
        if (buttonPressed(BTN_A)) {
            loadGame(games[selected].path);
            startGame();
            break;
        }
    }
}
```

---

### Memory Budget Example

**For 128×160 display with ~200KB usable RAM:**

```
RAM Allocation:
├── Framebuffer:       40KB  (128×160×2 bytes RGB565)
├── Tile cache:        40KB  (20 tiles × 32×16 × 2 bytes = ~20KB, double-buffered)
├── Sprite cache:      60KB  (20 sprites × 32×32 × 2 bytes = ~40KB)
├── Entity data:       20KB  (100 entities × 200 bytes)
├── Tilemap:           10KB  (100×100 tile IDs = 10KB)
├── Game logic state:  10KB  (variables, inventory, stats)
├── Stack/heap:        20KB  (runtime allocations)
└── Total:            ~200KB
```

**Key insight:** You only load what's needed for the current level, not the entire game.

---

### Data-Driven vs Scripting

#### Option A: Pure Data-Driven (Simple)
Games defined entirely by data files, engine interprets them.

**Pros:**
- No compilation needed
- Fast iteration
- Safe (can't crash engine)

**Cons:**
- Limited to engine capabilities
- No custom game logic

#### Option B: Bytecode/Scripting (Advanced)
Games include logic scripts (Lua, custom bytecode).

**Pros:**
- Full flexibility
- Custom game mechanics

**Cons:**
- Complex to implement
- Slower execution
- Larger memory footprint

**Recommendation for Phase 1:** Start with pure data-driven. Add scripting later if needed.

---

### Development Workflow

```
1. Edit game data on PC
   ├── Edit tilemap.dat
   ├── Edit sprites.png → convert to sprites.dat
   └── Edit entities.json → convert to entities.dat

2. Copy to SD card
   └── Copy entire game1/ folder

3. Test on console
   ├── Insert SD card
   ├── Power on
   ├── Select game from menu
   └── Play immediately (no reflash!)

4. Iterate
   └── Repeat steps 1-3
```

**No recompilation needed for game content changes!**

---

### Advantages of This Architecture

✅ **Multiple games on one SD card** - Just add folders
✅ **No reflashing** - Update game data without USB
✅ **Fast development** - Edit → Copy → Test cycle
✅ **Engine improvements benefit all games** - Update engine once
✅ **User-friendly** - Game selection menu like a console
✅ **Expansion packs** - Add DLC by copying files
✅ **Full performance** - Engine runs from Flash at 60 FPS
✅ **Scalable** - Add new games without engine changes

### Constraints

⚠️ **RAM budget** - Only ~200KB for active assets
⚠️ **Engine must be generic** - Can't optimize for one game
⚠️ **Loading time** - 50-100ms per game/level (acceptable)
⚠️ **Data format design** - Need well-designed file formats

---

### Comparison to Alternatives

| Approach | Pros | Cons |
|----------|------|------|
| **Engine + Data (Chosen)** | Multiple games, no reflash, fast iteration | RAM budget, generic engine |
| **Full binary per game** | Full performance, no constraints | Can't fit in RAM, complex bootloader |
| **Reflash per game** | Simple, full control | Requires USB, slow workflow |

---

## Key Takeaways

✅ **Use SD for:** Music, user content, development workflow
❌ **Don't use SD for:** Sprites, tiles (too slow for 60 FPS)
⚠️ **SD is 50-100× slower than Flash** - only access during loading screens
✅ **Massive storage:** Gigabytes (great for audio files)
🔄 **Shares SPI bus:** Must manage chip selects carefully
📦 **Optional:** Most games fit entirely in Flash

**Best practice:** Store frequently-accessed assets in Flash, large media files on SD card.

---

## See Also

- **[Level Loading Strategies](level_loading.md)** - How to load game data from SD to RAM (BSS vs Heap)
