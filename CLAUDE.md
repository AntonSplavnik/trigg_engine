# PocketGate Engine - Isometric Game Engine

## Role
You are a professional C++ developer. Primary standard is **C++11**, but **C++14/17/20** features may be used when necessary for clarity or performance.

## Project Overview
This is an isometric game engine for a physical handheld console:
- **Board**: WeAct Mini H743VITx (STM32H743VIT6)
- **Display**: ST7796S 4" TFT 320×480, SPI interface
- **Touch**: FT6336U capacitive touch controller (I2C)
- **Target**: Console-based isometric games
- **Standard**: C++11 (C++14/17/20 when beneficial)

## Working Principles
- Always inspect the codebase before providing answers
- Examine relevant files, classes, and functions related to each question
- Base all responses on the actual implementation in the codebase
- Provide concise, actionable answers (short to mid-length)

## Technical Constraints
- **MCU**: STM32H743VIT6 @ 480MHz (Cortex-M7)
- **Internal RAM**: ~1MB total (fragmented across domains)
  - DTCM: 128KB (fastest, no DMA)
  - AXI SRAM: 512KB (MDMA accessible)
  - SRAM1/2/3: 288KB (DMA1/2 accessible)
  - SRAM4: 64KB (low-power domain)
- **Display**: 320×480 pixels, RGB565 format
- **Framebuffer**: ~450KB each (320×480×3 bytes, 16-bit color + 8-bit alpha)
- **Double buffering**: Two framebuffers = ~900KB (requires external PSRAM)
- **External Memory**: PSRAM required for framebuffers
- **Flash**: 2MB internal + QSPI external for assets
- **SPI Speed**: Target max display SPI clock
- **Performance**: Target 60 FPS
- **Storage**: Use Flash/QSPI for assets, RAM for active data only

## Memory Architecture
```
Internal SRAM (~1MB):
├── DTCM (128KB)     → Stack, critical variables
├── AXI SRAM (512KB) → Large working buffers, single-buffer testing
├── SRAM1/2/3 (288KB)→ DMA buffers (SPI, audio)
└── SRAM4 (64KB)     → Low-power peripherals

External (planned):
├── PSRAM            → Framebuffers (2× ~450KB)
└── QSPI Flash       → Assets, sprites, tiles
```

## Code Standards
- Primary: C++11 features (auto, nullptr, range-based for, lambdas, smart pointers)
- Allowed: C++14/17/20 features when they improve code clarity or performance
- Optimize for embedded systems (minimize allocations, prefer stack over heap)
- Use const correctness
- Prefer compile-time constants over runtime where possible
- Use `constexpr` where applicable (C++14+)

## Hardware Interfaces
- **Display (ST7796S)**: SPI + GPIO control pins (DC, CS, Reset, Backlight)
- **Touch (FT6336U)**: I2C, interrupt-driven
- **Storage**: QSPI flash for assets
- **External RAM**: PSRAM via FMC or QSPI (for framebuffers)

## Project Structure
```
project/
├── platforms/
│   └── stm32/
│       └── drivers/    # STM32H7 specific drivers
├── engine/             # Core engine (sprite, tilemap, iso_renderer)
├── game/               # Game-specific logic
├── assets/             # Tile and sprite data (stored in Flash)
└── Docs/               # Documentation
```

## Game Architecture
**Sprite-based isometric rendering** (like Fallout 1, GBA LOTR: Two Towers):
- **NOT pure tile-based** - characters and objects are sprites with free movement
- Pre-rendered sprite sheets for characters/objects
- Background may use tiles internally for efficiency, but rendered seamlessly
- Camera/viewport system following player
- Layered sprite rendering with depth sorting

## Rendering System
- **Double buffering**: Two ~450KB framebuffers with pointer swapping (requires PSRAM)
- **Pixel format**: RGB565 (16-bit) + Alpha (8-bit) = 3 bytes per pixel
- **Sprite rendering**: Batch operations (memcpy, DMA, direct buffer writes)
- **Avoid individual pixel operations**: Never use set_pixel() for bulk drawing
- **Depth sorting**: Sort sprites by Y-position (back-to-front)
- **Coordinate system**: Isometric world coordinates → screen coordinates
- **Transparency**: Per-pixel alpha blending support
- **Camera**: Scrolling viewport with world-to-screen coordinate conversion
- **DMA**: Use DMA/MDMA for display transfers while CPU renders next frame

## Performance Targets
- **60 FPS sustained**
- 10-20 sprites on screen
- Double buffering: render to back buffer, swap, DMA send to display
- Frame budget: 16.67ms (clear + render + DMA send)
- Optimize batch operations over individual pixel writes
- Input latency < 16ms
- Touch response < 32ms

## Display Specifications
- **Controller**: ST7796S
- **Resolution**: 320×480
- **Interface**: SPI (4-wire)
- **Color depth**: RGB565 (65K colors)
- **Touch**: FT6336U capacitive (I2C address 0x38)
