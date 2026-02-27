# PocketGateEngine - Technical Specifications

Complete reference for all hardware specs, memory calculations, timing budgets, and unit conversions.

---

## Architecture Overview

**PocketGateEngine uses a cartridge-style architecture:**

```
Flash (2 MB)           SD Card (1 GB+)         RAM (264 KB)
────────────           ───────────────         ────────────
[Engine Code]    →→    [Game 1 Data]    →→    [Asset Cache]
[Renderer]             [Game 2 Data]           [Framebuffer]
[Physics]              [Game 3 Data]           [Game State]
[Game Loader]          [Saves]
```

**Benefits:**
- ✅ Develop multiple games without reflashing
- ✅ Fast iteration (edit data files, no recompilation)
- ✅ User can swap games like console cartridges
- ✅ Single engine benefits all games
- ✅ 60 FPS performance (assets cached in RAM)

**Workflow:**
1. Boot: Load engine from Flash (instant - XIP)
2. Menu: Display game list from SD card
3. Select: User picks a game
4. Load: Transfer game assets from SD card to RAM cache (50-100ms)
5. Play: Game runs at 60 FPS from RAM

See [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md#1-multi-game-architecture-cartridge-system) for full rationale.

---

## Table of Contents

1. [Hardware Core](#hardware-core)
2. [Display Specifications](#display-specifications)
3. [RGB565 Color Format](#rgb565-color-format)
4. [Framebuffer Memory](#framebuffer-memory)
5. [SPI Communication](#spi-communication)
6. [Performance & Timing](#performance--timing)
7. [Memory Architecture](#memory-architecture)
   - [RAM (264 KB)](#ram-264-kb)
   - [Flash (2 MB)](#flash-2-mb)
   - [SD Card (Optional, Recommended)](#sd-card-optional-recommended)
8. [Isometric Rendering](#isometric-rendering)
9. [Unit Conversions](#unit-conversions)
10. [Design Rationale](#design-rationale)

---

## Hardware Core

### Raspberry Pi Pico (RP2040)

| Component           | Specification                   |
|---------------------|---------------------------------|
| **CPU**             | Dual-core ARM Cortex-M0+        |
| **Clock Speed**     | 133 MHz                         |
| **RAM**             | 264 KB (270,336 bytes) SRAM     |
| **Flash**           | 2 MB (2,097,152 bytes) external |
| **GPIO Pins**       | 26 multi-function pins          |
| **SPI Controllers** | 2 (we use SPI0)                 |

### Memory Conversions

```
RAM: 264 KB
= 264 × 1,024 bytes
= 270,336 bytes

Flash: 2 MB
= 2 × 1,024 × 1,024 bytes
= 2,097,152 bytes
```

### CPU Performance

```
Clock Speed: 133 MHz = 133,000,000 cycles/second

Per microsecond: 133 cycles/µs
Per millisecond: 133,000 cycles/ms
Per frame (60 FPS): 2,216,667 cycles

Instruction throughput: ~1-3 cycles per instruction (average)
```

---

## Display Specifications

### ST7735 TFT Display

| Property         | Value                       |
|------------------|-----------------------------|
| **Model**        | ST7735 SPI TFT              |
| **Size**         | 1.8 inch diagonal           |
| **Resolution**   | 128 × 160 pixels            |
| **Total Pixels** | 20,480 pixels               |
| **Interface**    | SPI (4-wire + control pins) |
| **Color Depth**  | 16-bit (RGB565)             |
| **Refresh Rate** | ~60 Hz typical              |

### Pin Configuration

```cpp
PIN_SCK     = 2   // SPI Clock
PIN_MOSI    = 3   // SPI Data Out
PIN_MISO    = 19  // SPI Data In (SD card only)
PIN_BL      = 17  // Backlight (LITE)
PIN_TFT_CS  = 20  // Display Chip Select
PIN_CARD_CS = 21  // SD Card Chip Select
PIN_DC      = 22  // Data/Command
PIN_RESET   = 26  // Display Reset
```

**SPI Bus Sharing:**
- Display and SD card share SCK, MOSI, MISO
- Each has separate CS pin
- Only one device selected at a time

---

## RGB565 Color Format

### Bit Layout

```
15 14 13 12 11 | 10  9  8  7  6  5 |  4  3  2  1  0
R  R  R  R  R  |  G  G  G  G  G  G |  B  B  B  B  B
   5 bits RED  |     6 bits GREEN  |    5 bits BLUE
```

### Color Depth

| Channel | Bits | Values | Range |
|---------|------|--------|-------|
| **Red** | 5 | 2⁵ = 32 | 0-31 |
| **Green** | 6 | 2⁶ = 64 | 0-63 |
| **Blue** | 5 | 2⁵ = 32 | 0-31 |
| **Total** | 16 | 65,536 | - |

**Why 6 bits for green?**
Human eyes are most sensitive to green wavelengths. The extra bit provides better perceived color quality.

### RGB888 → RGB565 Conversion

```cpp
// Input: RGB888 (r, g, b each 0-255)
// Output: RGB565 (16-bit value)

uint16_t rgb565 = ((r & 0xF8) << 8) |   // Red: top 5 bits
                  ((g & 0xFC) << 3) |   // Green: top 6 bits
                  ((b & 0xF8) >> 3);    // Blue: top 5 bits

// Example: RGB(255, 128, 64)
// r: 255 & 0xF8 = 248 (11111000) → shift left 8
// g: 128 & 0xFC = 128 (10000000) → shift left 3
// b:  64 & 0xF8 =  64 (01000000) → shift right 3
// Result: 0xFC47
```

### Common Colors

```cpp
Red:     0xF800  // (11111 000000 00000)
Green:   0x07E0  // (00000 111111 00000)
Blue:    0x001F  // (00000 000000 11111)
White:   0xFFFF  // (11111 111111 11111)
Black:   0x0000  // (00000 000000 00000)
Yellow:  0xFFE0  // (11111 111111 00000)
Cyan:    0x07FF  // (00000 111111 11111)
Magenta: 0xF81F  // (11111 000000 11111)
```

### Alternative Formats (Why We Don't Use Them)

| Format | Bits/Pixel | Colors | Framebuffer | Verdict |
|--------|-----------|--------|-------------|---------|
| RGB444 | 12-bit | 4,096 | 30 KB | Poor quality |
| **RGB565** | **16-bit** | **65,536** | **40 KB** | **Optimal** |
| RGB666 | 18-bit | 262,144 | 60 KB | Too much RAM |
| RGB888 | 24-bit | 16.7M | 60 KB | Overkill, wastes RAM |

---

## Framebuffer Memory

### Size Calculation

```
Display Resolution: 128 × 160 pixels
Color Format: RGB565 = 2 bytes per pixel

Framebuffer Size:
= 128 × 160 pixels × 2 bytes/pixel
= 20,480 × 2
= 40,960 bytes
= 40 KB

Percentage of Total RAM:
= 40,960 / 270,336
= 15.1% of 264 KB RAM
```

### Memory Layout

```
Framebuffer array in RAM:
uint16_t framebuffer[128 * 160];  // 40,960 bytes

Memory address layout:
[0]      → pixel (0,0)   - top-left
[1]      → pixel (1,0)
[127]    → pixel (127,0) - top-right
[128]    → pixel (0,1)   - second row
...
[20,479] → pixel (127,159) - bottom-right

Index formula: framebuffer[y * SCREEN_WIDTH + x]
```

---

## SPI Communication

### SPI Configuration

```cpp
// Initialize SPI at 62.5 MHz
spi_init(spi0, 62500000);
spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
```

| Parameter | Value | Description |
|-----------|-------|-------------|
| **Clock Speed** | 62.5 MHz | 62,500,000 Hz |
| **Bits per Transfer** | 8 bits | 1 byte at a time |
| **Mode** | Mode 0 | CPOL=0, CPHA=0 |
| **Bit Order** | MSB First | Most significant bit first |

### Clock Speed Derivation

```
System Clock: 133 MHz (RP2040 default)
SPI Clock: 62.5 MHz

Actually derived from 125 MHz peripheral clock:
125 MHz ÷ 2 = 62.5 MHz (clean division)

Percentage of system speed:
62.5 / 133 = 47% of CPU clock
```

### Transfer Rates

```
Theoretical Maximum:
62.5 MHz ÷ 8 bits/byte = 7,812,500 bytes/second
= 7.8125 MB/sec

Practical Throughput (with overhead):
≈ 6-7 MB/sec
```

### Framebuffer Transfer Time

```
Full Framebuffer: 40,960 bytes

At 7 MB/sec:
Transfer time = 40,960 ÷ 7,000,000
              = 0.00585 seconds
              = 5.85 milliseconds
              = 5,850 microseconds

At 60 FPS:
Frame budget = 16.67 ms
Transfer uses = 5.85 ms (35% of frame time)
Remaining = 10.82 ms for rendering
```

---

## Performance & Timing

### 60 FPS Target

```
60 FPS = 60 frames per second

Frame Period:
= 1,000 ms ÷ 60 frames
= 16.67 milliseconds per frame
= 16,667 microseconds per frame

CPU Cycles per Frame:
= 133,000,000 cycles/sec ÷ 60 frames/sec
= 2,216,667 cycles per frame
```

### Frame Time Budget

```
Total Budget: 16.67 ms per frame

Breakdown:
├─ Game Logic:        3.0 ms  (18%)
├─ Rendering:         6.0 ms  (36%)
├─ SPI Transfer:      5.9 ms  (35%)
├─ Input Processing:  1.0 ms  (6%)
└─ Reserve:           0.8 ms  (5%)
────────────────────────────────
Total:                16.67 ms (100%)
```

### Other Frame Rates

```
30 FPS = 33.33 ms per frame
60 FPS = 16.67 ms per frame
120 FPS = 8.33 ms per frame (theoretical max)
```

---

## Memory Architecture

### RAM (264 KB)

**Characteristics:**

| Property | Value |
|----------|-------|
| **Type** | SRAM (Static RAM) |
| **Size** | 264 KB (270,336 bytes) |
| **Volatility** | Volatile (loses data on power-off) |
| **Read Speed** | <1 nanosecond per byte |
| **Write Speed** | <1 nanosecond per byte |
| **Write Cycles** | Unlimited |

**RAM Budget:**

```
RAM (264 KB total):
├─ Framebuffer:         40 KB   (15%)   [mandatory - display output]
├─ Tile cache:          40 KB   (15%)   [loaded from SD card]
├─ Sprite cache:        60 KB   (23%)   [loaded from SD card]
├─ Entities (20):       20 KB   (7.5%)  [game objects]
├─ Game state:          40 KB   (15%)   [player, inventory, variables]
├─ Pico SDK:            20 KB   (7.5%)  [system overhead]
├─ Stack:                8 KB   (3%)    [function calls]
└─ Free/Reserve:        36 KB   (14%)   [safety margin]
────────────────────────────────────────
Total Used:            228 KB   (86%)
```

**Architecture: Assets Loaded from SD Card to RAM Cache**

RAM stores the **active game assets** loaded from SD card at runtime.

**When to Use RAM:**
- ✅ Framebuffer (mandatory - updated every frame)
- ✅ Tile cache (loaded from SD card per level)
- ✅ Sprite cache (loaded from SD card per game)
- ✅ Active game state (player position, health, score)
- ✅ Entity arrays (enemies, items, projectiles)
- ✅ Current level data (loaded from SD card)
- ❌ Engine code (use Flash)
- ❌ All game assets at once (use cache, swap as needed)

### Flash (2 MB)

**Characteristics:**

| Property | Value |
|----------|-------|
| **Type** | External Flash chip |
| **Size** | 2 MB (2,097,152 bytes) |
| **Volatility** | Non-volatile (keeps data when off) |
| **Read Speed** | 1-3 microseconds per byte |
| **Write Speed** | Milliseconds (very slow) |
| **Write Cycles** | ~100,000 times (limited) |
| **Access Method** | XIP (Execute In Place) |

**Flash Budget:**

```
Flash (2 MB total):
├─ Engine code:        100 KB   (5%)    [renderer, physics, game loop]
├─ Pico SDK:            50 KB   (2.5%)  [system libraries]
├─ Display drivers:     20 KB   (1%)    [ST7735, SPI drivers]
├─ Game loader:         30 KB   (1.5%)  [SD card, menu, loader]
├─ Boot menu UI:        10 KB   (0.5%)  [game selection]
└─ Available:        1,887 KB   (92%)   [future engine features]
──────────────────────────────────────────
Total Used:            210 KB   (10%)
```

**Architecture: Engine in Flash, Games on SD Card**

Flash stores only the **engine** - game data lives on SD card and is loaded to RAM at runtime.

**When to Use Flash:**
- ✅ Engine code (renderer, physics, game loop)
- ✅ Display drivers (ST7735, SPI communication)
- ✅ System libraries (Pico SDK)
- ✅ Game loader and boot menu
- ❌ Game sprites/tiles (use SD card → RAM cache)
- ❌ Level data (use SD card → RAM)
- ❌ Game-specific assets (use SD card)

### SD Card (Optional, Recommended)

**Characteristics:**

| Property | Value |
|----------|-------|
| **Type** | MicroSD card (removable) |
| **Size** | GB range (user-provided) |
| **Volatility** | Non-volatile (keeps data when off) |
| **Read Speed** | 50-150 microseconds per byte |
| **Write Speed** | 100-200 microseconds per byte |
| **Write Cycles** | ~10,000-100,000 (varies by card) |
| **Access Method** | SPI + FatFS filesystem |

**SD Card Structure:**

```
SD Card (/):
├─ games/
│  ├─ platformer/
│  │  ├─ manifest.dat    (game metadata)
│  │  ├─ sprites.dat     (RGB565 sprite data)
│  │  ├─ tiles.dat       (RGB565 tile data)
│  │  ├─ level1.dat      (tilemap layout)
│  │  ├─ level2.dat
│  │  └─ saves/          (save game data)
│  ├─ rpg/
│  │  ├─ manifest.dat
│  │  ├─ sprites.dat
│  │  └─ ...
│  └─ puzzle/
│     └─ ...
└─ music/
   ├─ track1.wav
   └─ track2.wav
```

**Typical Game Size:**

```
Single game on SD card:
├─ Sprites (50):        100 KB  (50 × 2KB each)
├─ Tiles (30):           30 KB  (30 × 1KB each)
├─ Levels (20):          80 KB  (20 × 4KB each)
├─ Manifest:              1 KB  (metadata)
└─ Save data:            10 KB  (player progress)
───────────────────────────────
Total per game:         ~221 KB

10 games fit in:        ~2.2 MB
Recommended SD card:    1 GB or larger
```

**When to Use SD Card:**
- ✅ Game sprites and tiles (all games)
- ✅ Level data (tilemaps, entity layouts)
- ✅ Save games (player progress)
- ✅ Music files (if implementing audio)
- ✅ User-generated content
- ✅ Development workflow (no recompilation needed)
- ❌ Engine code (use Flash)
- ❌ Real-time rendering (too slow - load to RAM first)

**Loading Strategy:**

```
Boot sequence:
1. Load engine from Flash (instant - XIP)
2. Display boot menu (read game list from SD card)
3. User selects game
4. Load game assets to RAM cache (50-100ms)
5. Start game loop (runs at 60 FPS from RAM)

Level transition:
1. Unload current level assets from RAM
2. Load new level from SD card to RAM (30-50ms)
3. Resume game loop
```

### Memory Speed Comparison

**Reading 1 KB of Data:**

```
From RAM:       ~1 microsecond      (fastest)
From Flash:     ~3 microseconds     (3× slower than RAM)
From SD Card:   ~150 microseconds   (50× slower than Flash)

Conclusion: Flash is fast enough for 60 FPS rendering!
```

**Why This Matters:**

```
At 60 FPS, we have 16.67 ms per frame

Reading 10 tiles from Flash:
= 10 tiles × 1 KB × 3 µs
= 30 microseconds
= 0.03 milliseconds
= 0.2% of frame budget

This is why we store sprites in Flash, not SD card.
```

---

## Isometric Rendering

### Tile Specifications

```
Tile Shape: Diamond (isometric)
Tile Size: 32 × 16 pixels
Bytes per Tile: 32 × 16 × 2 = 1,024 bytes = 1 KB
```

### Visible Tile Count

```
Screen: 128×160 pixels
Tile: 32×16 pixels

Simple calculation:
Horizontal: 128 ÷ 32 = 4 tiles
Vertical: 160 ÷ 16 = 10 tiles

Isometric layout (diamond arrangement):
Typical visible area: 10×10 tiles = 100 tiles on screen
```

### Coordinate Conversion

```cpp
// World coordinates (grid) → Screen coordinates (pixels)
screen_x = (world_x - world_y) * (tile_width / 2);
screen_y = (world_x + world_y) * (tile_height / 2);

// With our tile size (32×16):
screen_x = (world_x - world_y) * 16;
screen_y = (world_x + world_y) * 8;
```

### Render Order & Depth Sorting

```
Render order: Back-to-front (painter's algorithm)
Sort key: (world_x + world_y)

Example:
Tile (0,0) → sort key = 0  (render first)
Tile (1,0) → sort key = 1
Tile (0,1) → sort key = 1
Tile (5,5) → sort key = 10 (render last)
```

### Memory for Tile Assets

```
Storage locations:

Tiles (on SD card):
20 unique tiles = 20 × 1,024 bytes = 20 KB on SD card
Loaded to RAM cache when game starts

Tilemap Layout (on SD card):
64×64 tilemap = 4,096 tile IDs = 4 KB per level
Loaded to RAM when level starts

Multiple levels (on SD card):
20 levels = 20 × 4 KB = 80 KB per game on SD card

Multiple games (on SD card):
10 games with 20 levels each = 800 KB total on SD card
```

**Data Flow:**
```
SD Card → RAM Cache → Display

1. Level load: Read tiles from SD → 40 KB tile cache in RAM
2. Rendering: Read from RAM cache (instant) → framebuffer
3. SPI transfer: Framebuffer → Display

SD card only accessed during level transitions (50-100ms)
Gameplay runs from RAM at 60 FPS
```

---

## Unit Conversions

### Storage Units

```
Bits to Bytes:
1 Byte = 8 bits
2 Bytes = 16 bits (RGB565 pixel)
4 Bytes = 32 bits (int/float)

Bytes to Kilobytes:
1 KB = 1,024 bytes
10 KB = 10,240 bytes
40 KB = 40,960 bytes (framebuffer)
264 KB = 270,336 bytes (RAM)

Kilobytes to Megabytes:
1 MB = 1,024 KB
1 MB = 1,048,576 bytes
2 MB = 2,097,152 bytes (Flash)
```

### Time Units

```
1 second = 1,000 milliseconds (ms)
1 millisecond = 1,000 microseconds (µs)
1 microsecond = 1,000 nanoseconds (ns)

Common conversions:
60 FPS = 16.67 ms per frame
1 ms = 1,000 µs
16.67 ms = 16,667 µs
```

### Frequency (Clock Speed)

```
1 Hz = 1 cycle per second
1 kHz = 1,000 Hz = 1,000 cycles/sec
1 MHz = 1,000,000 Hz = 1 million cycles/sec

Our hardware:
133 MHz CPU = 133,000,000 cycles/sec
62.5 MHz SPI = 62,500,000 cycles/sec
```

### Color Formats

```
1 pixel RGB565 = 16 bits = 2 bytes
1 pixel RGB888 = 24 bits = 3 bytes
1 pixel RGB444 = 12 bits = 1.5 bytes

128×160 framebuffer:
RGB565: 20,480 × 2 = 40,960 bytes = 40 KB ✅
RGB888: 20,480 × 3 = 61,440 bytes = 60 KB ❌ (too large)
RGB444: 20,480 × 1.5 = 30,720 bytes = 30 KB (poor quality)
```

---

## Design Rationale

### Why RGB565?

**Decision:** Use 16-bit RGB565 color format

**Reasons:**
- ✅ 65,536 colors (sufficient for pixel art games)
- ✅ 40 KB framebuffer (fits comfortably in 264 KB RAM)
- ✅ 2 bytes per pixel = fast SPI transfers
- ✅ Direct hardware support in ST7735
- ❌ RGB888 would need 60 KB (23% of RAM - too much)
- ❌ RGB444 only 4,096 colors (poor quality)

### Why 62.5 MHz SPI?

**Decision:** Run SPI at 62.5 MHz

**Reasons:**
- ✅ Fast enough for 60 FPS (5.85 ms transfer time)
- ✅ Clean division of 125 MHz peripheral clock
- ✅ Within ST7735 reliable operating range
- ✅ Good signal integrity on PCB traces
- ❌ Higher speeds risk data corruption
- ❌ Lower speeds can't achieve 60 FPS target

**Math:**
```
Frame transfer: 40 KB ÷ 7 MB/s = 5.85 ms
Frame budget at 60 FPS: 16.67 ms
Percentage used: 35% (leaves 65% for rendering)
```

### Why Engine in Flash, Games on SD Card?

**Decision:** Cartridge-style architecture (engine in Flash, games on SD card)

**Engine in Flash:**
- ✅ Fast execution (XIP - Execute In Place)
- ✅ Always available (can't lose/corrupt engine)
- ✅ Single engine benefits all games
- ✅ No loading time for engine code
- ✅ 2 MB is plenty for engine (only using ~10%)

**Games on SD Card:**
- ✅ Multiple games without reflashing
- ✅ Fast development (edit data, no recompilation)
- ✅ User can add new games easily
- ✅ Swappable content (like console cartridges)
- ✅ Save games on same SD card
- ✅ Unlimited storage (GB vs 2MB Flash)

**Why Not Store Games in Flash?**
- ❌ Flash read-only at runtime (can't add games)
- ❌ Only 2 MB (too small for many games)
- ❌ Requires reflashing to change games
- ❌ Slow development workflow

**Performance Strategy:**
```
SD card is too slow for real-time rendering:
- SD read: 150 µs/byte (50× slower than Flash)
- 10 tiles from SD: 1,500 µs (9% of frame budget)

Solution: Load from SD to RAM cache
1. Load assets from SD card during level load (50-100ms)
2. Cache in RAM (40KB tiles + 60KB sprites)
3. Render from RAM at 60 FPS (instant access)

Level transitions have loading time, but gameplay is smooth!
```

### Why 264 KB RAM Layout?

**Decision:** Single framebuffer (40 KB) + asset cache (100 KB) loaded from SD card

**Reasons:**
- ✅ Single buffer saves 40 KB for asset cache
- ✅ 62.5 MHz SPI fast enough (minimal tearing)
- ✅ 100 KB cache holds active game assets from SD card
- ✅ Fast rendering from RAM (not SD card)
- ❌ Double buffering would use 80 KB (no room for cache)
- ❌ Must cache assets in RAM (SD too slow for 60 FPS)

**Budget:**
```
Framebuffer: 40 KB (15%)  [display output]
Tile cache: 40 KB (15%)   [loaded from SD]
Sprite cache: 60 KB (23%) [loaded from SD]
Game state: 60 KB (23%)   [entities, variables]
System: 28 KB (11%)       [SDK, stack]
Free: 36 KB (14%)         [reserve]
```

**Workflow:**
```
Level load (one-time):
1. Read tiles from SD card → tile cache (40 KB)
2. Read sprites from SD card → sprite cache (60 KB)
3. Load level data → game state (20 KB)
Total load time: 50-100ms

Game loop (60 FPS):
1. Render from RAM cache (instant)
2. Update game state in RAM
3. No SD card access during gameplay
```

### Why 128×160 Display?

**Current Hardware:** ST7735 128×160

**Future Upgrade Path:**
- ST7789: 240×240 pixels
- ILI9341: 320×240 pixels

**Design Principle:** Display-agnostic architecture
- Never hardcode resolution (use SCREEN_WIDTH/SCREEN_HEIGHT)
- Abstract display driver interface
- Swappable drivers for different displays

### Why 60 FPS Target?

**Decision:** Target 60 frames per second

**Reasons:**
- ✅ Smooth gameplay (16.67 ms per frame)
- ✅ Achievable with current hardware
- ✅ Standard for action games
- ✅ Input latency <16 ms (responsive)
- ❌ 30 FPS feels sluggish for action
- ❌ 120 FPS not achievable with SPI transfer overhead

---

## Quick Reference Card

### Key Numbers

```
CPU Clock:        133 MHz
SPI Clock:        62.5 MHz
RAM:              264 KB (270,336 bytes)
Flash:            2 MB (2,097,152 bytes) - Engine only
SD Card:          1 GB+ - All games
Display:          128 × 160 pixels
Framebuffer:      40 KB (40,960 bytes)
Tile Cache:       40 KB (loaded from SD)
Sprite Cache:     60 KB (loaded from SD)
Color Format:     RGB565 (16-bit, 65,536 colors)
Target FPS:       60 FPS (16.67 ms/frame)
Transfer Time:    5.85 ms per frame
Tile Size:        32 × 16 pixels (1 KB each)
```

### Architecture

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   Flash     │      │   SD Card   │      │     RAM     │
│   (2 MB)    │      │   (1 GB+)   │      │   (264 KB)  │
├─────────────┤      ├─────────────┤      ├─────────────┤
│ Engine      │      │ Game 1      │  →→  │ Framebuffer │
│ Renderer    │      │ Game 2      │  →→  │ Tile cache  │
│ Physics     │      │ Game 3      │  →→  │ Sprite cache│
│ Game Loader │      │ ...         │      │ Game state  │
│ Boot Menu   │      │ Saves       │      │             │
└─────────────┘      └─────────────┘      └─────────────┘
     XIP                  Load                 Render
   (instant)           (50-100ms)             (60 FPS)
```

### Critical Calculations

```
Framebuffer size:
= 128 × 160 × 2 bytes
= 40,960 bytes = 40 KB

Frame budget:
= 1000 ms ÷ 60 FPS
= 16.67 ms per frame

SPI transfer time:
= 40,960 bytes ÷ 7,000,000 bytes/sec
= 5.85 ms (35% of frame)

Memory speeds:
RAM:     <1 ns/byte
Flash:   ~3 µs/byte (3,000× slower)
SD Card: ~150 µs/byte (50× slower than Flash)
```

---

## See Also

- **[DESIGN_DECISIONS.md](DESIGN_DECISIONS.md)** - Architectural rationale
- **[hardware/rgb565.md](hardware/rgb565.md)** - Color format details
- **[hardware/spi.md](hardware/spi.md)** - SPI communication
- **[memory/ram.md](memory/ram.md)** - RAM usage guide
- **[memory/flash.md](memory/flash.md)** - Flash storage guide
- **[CLAUDE.md](../CLAUDE.md)** - Project constraints
