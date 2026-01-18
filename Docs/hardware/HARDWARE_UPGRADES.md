# Hardware Upgrade Plan

## Overview
This document outlines potential hardware upgrades for the TriggEngine console to improve performance, usability, and user experience.

---

## 1. Microcontroller Upgrade

### Current: Raspberry Pi Pico (RP2040)
- Dual-core ARM Cortex-M0+ @ 133MHz
- 264KB SRAM
- 2MB Flash

### Chosen: STM32H743VIT6

**Selected for the upgraded console (see DESIGN_DECISIONS.md Section 7 for full rationale)**

| Spec | Value |
|------|-------|
| CPU | ARM Cortex-M7 @ 480 MHz |
| RAM | 1 MB |
| Flash | 2 MB internal + 8 MB QSPI |
| FPU | Full hardware floating-point |
| DMA2D | 2D graphics acceleration (Chrom-ART) |
| DSP | SIMD instructions |

**Why STM32H743:**
- ✅ 1 MB RAM fits 4" display with double buffering (614 KB needed)
- ✅ Hardware FPU for raycasting engine (10-50× faster than software)
- ✅ DMA2D accelerates sprite blending in hardware
- ✅ 480 MHz single-core outperforms dual 240 MHz cores for game loops
- ✅ Extensive HAL libraries and documentation

**Development Board Specs:**
- STM32H743VIT6 480MHz, 2048KB ROM, 1MB RAM
- 8MB SPI Flash, 8MB QSPI Flash (executable)
- USB-C interface with ESD protection
- MicroSD card slot (on-board)
- DCMI camera interface (optional use)
- 2×22 Pin 2.54mm I/O headers

### Alternatives Considered (Not Chosen)

#### ESP32-S3 N16R8
- ❌ 512 KB RAM insufficient for 4" double buffering
- ❌ No hardware FPU for floating-point trig
- ✅ Has WiFi (can add as co-processor later)

#### Raspberry Pi Pico 2 (RP2350)
- ❌ 520 KB RAM tight for 4" double buffering
- ❌ 150 MHz slowest option
- ✅ Cheapest, good for smaller displays

#### Compute Module (RPi CM4)
- ❌ Overkill for 2D retro games
- ❌ 10-30 second boot time
- ❌ Higher power consumption (2-5W)
- ✅ Only option for true 3D GPU graphics

---

## 2. Display Upgrade

### Current: ST7735 128x160 SPI
- 1.8" TFT
- 128x160 resolution
- Standard TN panel

### Chosen: ST7796S 4" IPS with FT6336U Capacitive Touch

**Selected display module: MSP4031 (see DESIGN_DECISIONS.md Section 7 for full rationale)**

| Spec | Value |
|------|-------|
| Screen Size | 4.0" diagonal |
| Screen Type | IPS |
| Resolution | 480×320 pixels |
| PPI | ~144 (similar to PSP) |
| Color Depth | 262K (18-bit) |
| Driver IC | ST7796S |
| Interface | 4-line SPI |
| Brightness | 300 cd/m² |
| Backlight | White LED × 8 |

**Touch Screen Specs:**

| Spec | Value |
|------|-------|
| Type | Capacitive |
| Driver IC | FT6336U |
| Interface | **I2C** (separate from SPI!) |
| Resolution | 320×480 |

**Why ST7796S:**
- ✅ SPI MISO tristates properly (can share bus with SD card)
- ✅ Best overclock headroom (datasheet: 15 MHz, practical: 40-80 MHz)
- ✅ IPS panel with good viewing angles
- ✅ Confirmed chip (not mislabeled like many ILI9488)
- ✅ Native RGB565 support

**Why FT6336U Capacitive Touch:**
- ✅ **I2C interface** - completely separate from SPI bus
- ✅ No bus contention with display or SD card
- ✅ Better touch feel than resistive (XPT2046)
- ✅ Essential for inventory management in RPG games

**Physical Dimensions:**
- Module: 60.88mm × 108.0mm × 14.80mm (with touch)
- Active Area: 55.68mm × 83.52mm
- Connector: 14-pin 2.54mm header (jumper wire friendly)

### Display Controller Comparison

| Controller | ST7796S (Chosen) | ILI9488 | ILI9486 |
|------------|------------------|---------|---------|
| SPI Speed (datasheet) | 15 MHz | 20 MHz | 20 MHz |
| SPI Speed (practical) | **40-80 MHz** | 20-40 MHz | 25-32 MHz |
| Color Depth | 262K | 16.7M | 262K |
| MISO Tristate | ✅ Yes | ❌ **No** | ⚠️ Needs fix |
| SD Card Sharing | ✅ Works | ❌ Broken | ⚠️ Possible |

**Critical**: ILI9488 MISO never tristates - cannot share SPI bus with SD card. This was the deciding factor.

### Framebuffer Memory Impact

```
480×320 × 2 bytes (RGB565) = 307,200 bytes per buffer
Double buffering = 614,400 bytes (614 KB)

STM32H743 has 1 MB RAM → ✅ Fits with 386 KB spare
ESP32-S3 has 512 KB RAM → ❌ Cannot fit
Pico 2 has 520 KB RAM → ❌ Cannot fit
```

### 2.1 Understanding Display Resolution & PPI

#### What is PPI?
**PPI (Pixels Per Inch)** is the relationship between resolution and physical display size:
```
PPI = diagonal_in_pixels / diagonal_in_inches
```

- **Higher PPI** = More pixels packed per inch = Smaller individual pixels
- **Lower PPI** = Fewer pixels per inch = Larger individual pixels
- **PPI varies between display models** - it's NOT standardized

#### Current Display Analysis
**ST7735 128×160 (1.8" diagonal)**:
- Resolution: 128×160 pixels
- PPI ≈ 114
- Framebuffer: 40KB (128×160×2 bytes)
- Physical pixel size: ~0.22mm

#### Common Upgrade Options with PPI Analysis

| Display | Size | Resolution | PPI | Framebuffer | Physical Pixel Size |
|---------|------|------------|-----|-------------|-------------------|
| ST7735 (current) | 1.8" | 128×160 | ~114 | 40KB | ~0.22mm |
| ST7789 | 2.4" | 240×240 | ~141 | 115KB | ~0.18mm |
| ST7789 | 1.54" | 240×240 | ~220 | 115KB | ~0.12mm |
| ILI9341 | 2.4" | 240×320 | ~167 | 154KB | ~0.15mm |
| ILI9341 | 2.8" | 240×320 | ~143 | 154KB | ~0.18mm |
| ST7789 | 4.0" | 320×240 | ~100 | 154KB | ~0.25mm |
| Higher-res | 4.0" | 480×320 | ~144 | 307KB | ~0.18mm |

#### How Display Upgrades Affect Sprites

When upgrading displays, **two independent factors** affect how sprites appear:

**Factor 1: Resolution Change (View Area)**
- Higher resolution = More pixels = Larger view area
- A 16×16 pixel sprite remains 16×16 pixels
- You see **more of the game world** (camera shows larger area)

**Factor 2: Physical Size Change (Sprite Physical Size)**
- Larger physical display = Bigger pixels (usually)
- Same 16×16 sprite appears **physically larger or smaller** depending on PPI
- Lower PPI = sprite is physically bigger and easier to see
- Higher PPI = sprite is physically smaller but sharper

#### Practical Examples

**Example 1: Upgrade to 4" Display (320×240)**
- Resolution increases: 128×160 → 320×240 (2.5× width, 1.5× height)
- View area: See ~2.5× more horizontally, 1.5× more vertically
- PPI decreases: ~114 → ~100 (pixels get bigger)
- **Result**: More view area + sprites are physically comfortable to see

**Example 2: Upgrade to 2.4" Display (240×320)**
- Resolution increases: 128×160 → 240×320 (1.9× both dimensions)
- View area: See ~1.9× more of the game world
- PPI increases: ~114 → ~167 (pixels get smaller)
- **Result**: More view area but sprites are physically smaller (sharper, more detailed)

#### Choosing the Right Display

**For comfortable gameplay without scaling sprites:**
- Target PPI: 80-130 (similar to current display)
- Recommended: 2.4"-4.0" displays with 240×240 or 320×240 resolution
- These provide more view area while keeping sprites physically visible

**For maximum detail (may need larger sprites):**
- Higher PPI displays (150+) provide sharper graphics
- May require upscaling sprites or creating higher-resolution assets
- Better for displays with more RAM budget

#### Memory Constraints

**Framebuffer Memory = Width × Height × 2 bytes (RGB565)**

On Raspberry Pi Pico (264KB RAM):
- 128×160: 40KB (double buffered: 80KB) ✓ Current
- 240×240: 115KB (double buffered: 230KB) ⚠️ Tight on Pico 1
- 320×240: 154KB (double buffered: 308KB) ✗ Exceeds Pico 1 RAM

On Raspberry Pi Pico 2 (520KB RAM):
- 240×320: 154KB (double buffered: 308KB) ✓ Comfortable
- 480×320: 307KB (double buffered: 614KB) ✗ Too large even for Pico 2

**Practical Limit**: 320×240 is near maximum for double buffering on Pico 2

---

## 3. Input System Upgrades

### Current Configuration
- Individual tactile buttons
- PSP-like horizontal layout
- No hardware debouncing

### Proposed Changes

#### 3.1 D-Pad (Cross/Directional Pad)
**Replace left-side discrete buttons with proper D-pad**
- Better tactile feedback for directional input
- More ergonomic for isometric movement
- Standard gaming control scheme
- Options:
  - Single membrane D-pad
  - Four mechanical switches in cross formation
  - Alps SKQE series low-profile tactile switches

#### 3.2 Hardware Debouncing
**Add capacitors to all button inputs**
- 100nF ceramic capacitor per button (between GPIO and GND)
- Optional: RC network (10kΩ + 100nF) for cleaner signals
- Reduces need for software debouncing
- Improves input reliability
- Faster input response

#### 3.3 Additional Buttons
**Add standard console buttons**:
- **START button**: Pause/menu access
- **SELECT button**: Secondary menu/options
- **L/R shoulder buttons**: Optional, for additional actions

#### 3.4 Button Layout
```
Current (PSP-style):
[D-Pad Area]  [SCREEN]  [Action Buttons]
    ↕↔                      A B
                            X Y

Proposed (Game Boy Color style - vertical):
     [SCREEN]

     [D-Pad]  [A]
              [B]

  [SELECT] [START]
```

---

## 4. Form Factor & Orientation

### Current: PSP-style (Horizontal/Landscape)
- Wide horizontal layout
- Side-by-side controls

### Proposed: Game Boy Color Style (Vertical/Portrait)

#### Option A: Portrait Orientation (Recommended)
**Advantages**:
- More compact and portable
- Better one-handed grip potential
- Familiar retro gaming form factor
- Natural fit for 240x320 display (vertical)
- Easier to pocket

**Considerations**:
- May need to adjust isometric camera for portrait aspect
- UI elements need portrait-friendly layout

#### Option B: Keep Landscape
**Advantages**:
- Wider field of view for isometric games
- More traditional console feel
- No code changes for aspect ratio

**Decision Point**: Test both with cardboard prototype before finalizing

---

## 5. Audio Upgrade

### Current: Single Speaker
- Mono audio output

### Proposed: Enhanced Audio System

#### Option A: Stereo Speakers
- Two small speakers (left/right)
- Stereo sound effects and music
- More immersive experience
- Requires second PWM channel

#### Option B: Better Single Speaker + 3.5mm Jack
- Larger/better quality speaker
- Headphone jack for private gaming
- Simpler implementation

**Recommendation**: Start with Option B (headphone jack), add stereo in future revision

---

## 6. Power System Upgrade

### Current: AAA Batteries
- 2-3x AAA batteries
- Frequent replacement needed
- Environmental waste
- Variable voltage as batteries drain

### Proposed: Rechargeable Lithium Battery

#### Recommended Solution: Li-Po/Li-Ion Battery
**Battery Options**:
- 18650 Li-Ion cell (3.7V, 2000-3500mAh)
- Li-Po pouch cell (3.7V, 1000-2000mAh)
- Custom capacity based on form factor

**Required Components**:
- **Battery Management System (BMS)**:
  - TP4056 charging module (simple, cheap)
  - MCP73831 single-cell charger IC
  - Overcharge/overdischarge protection

- **Power Regulation**:
  - 3.3V buck converter (TPS63000 or similar)
  - Efficient switching regulator for battery life

- **Charging**:
  - USB-C charging port
  - LED charging indicator (red=charging, green=full)
  - Charge current: 500mA-1A

- **Battery Monitoring**:
  - Voltage divider to ADC for battery level
  - Low battery warning system
  - Fuel gauge IC (optional): MAX17048 for accurate %

**Benefits**:
- Longer play time (3-6 hours depending on capacity)
- Rechargeable via USB
- Cost savings over time
- Consistent voltage/performance
- Battery level indicator possible

**Safety Considerations**:
- Proper BMS with protection circuits
- Quality battery cells only
- Temperature monitoring
- Proper enclosure design

---

## 7. Additional Potential Upgrades

### 7.1 Haptic Feedback
- Small vibration motor
- Tactile feedback for game events
- Low power consumption (50-100mA peak)

### 7.2 MicroSD Card Slot
- Expandable storage for game saves
- Load custom games/levels
- Asset streaming for large games
- FAT32 filesystem support

### 7.3 RGB Status LED
- WS2812B or similar
- Show battery level, game state, notifications
- Low cost, high visibility

### 7.4 External GPIO Header
- Expansion port for accessories
- I2C/SPI/UART breakout
- Debugging interface
- Future peripheral support

---

## 8. Implementation Priority

### Phase 1: Core Hardware (Current Focus)
1. **Microcontroller**: STM32H743VIT6 dev board ✅ Selected
2. **Display**: ST7796S 4" IPS with FT6336U touch (MSP4031) ✅ Selected
3. **Input**: Joystick + button kit ✅ Selected
4. **Connectivity**: Dupont wires 20cm (F-F, M-F) for prototyping

### Phase 2: Software Adaptation
5. **Port engine**: Adapt framebuffer code for STM32 HAL
6. **Display driver**: Write ST7796S driver
7. **Touch driver**: Write FT6336U I2C driver
8. **DMA2D integration**: Hardware-accelerated sprite blending

### Phase 3: Ergonomics & Power
9. **Power System**: Li-Po battery + USB-C charging
10. **D-Pad**: Replace discrete buttons with proper D-pad
11. **Form Factor**: Design console layout

### Phase 4: Polish & Features
12. **Audio**: I2S DAC (MAX98357A) + headphone jack
13. **Battery Monitor**: Fuel gauge/indicator
14. **Custom PCB**: Integrate all components
15. **Enclosure**: 3D printed or manufactured case

---

## 9. Estimated Costs (USD)

### Selected Hardware (Phase 1)

| Component | Model | Price |
|-----------|-------|-------|
| MCU | STM32H743VIT6 dev board | ~$20 |
| Display | ST7796S 4" IPS + touch (MSP4031) | ~$15 |
| Input | Joystick + button kit | ~$5 |
| Wiring | Dupont wires 20cm (assorted) | ~$3 |
| Breadboards | 2× 830-point (optional) | ~$5 |
| **Phase 1 Total** | | **~$48** |

### Future Phases

| Component | Model | Price |
|-----------|-------|-------|
| Battery | Li-Po 2000mAh + TP4056 + regulator | ~$8 |
| Audio | MAX98357A I2S DAC + speaker | ~$5 |
| D-Pad | Mechanical D-pad assembly | ~$3 |
| Custom PCB | JLCPCB 5-board order | ~$10 |
| Enclosure | 3D printed or manufactured | ~$15 |
| **Future Total** | | **~$41** |

### Complete Console Estimate

| Stage | Cost |
|-------|------|
| Development prototype | ~$48 |
| Final console (additional) | ~$41 |
| **Total project** | **~$89** |

*Note: Prices are estimates for hobbyist quantities from AliExpress/JLCPCB. Prices vary by seller and region.*

---

## 10. Software Implications

### Engine Changes Required

**Platform Migration (Pico → STM32H743):**
- [ ] Port to STM32 HAL (replace Pico SDK calls)
- [ ] Update SCREEN_WIDTH=480, SCREEN_HEIGHT=320
- [ ] Increase framebuffer allocation (307 KB × 2)
- [ ] Implement DMA-based SPI transfers
- [ ] Add DMA2D hardware acceleration for sprites

**New Drivers Required:**
- [ ] ST7796S display driver (SPI, based on existing ST7735)
- [ ] FT6336U touch driver (I2C, new)
- [ ] Joystick ADC driver (analog input)

**DMA2D Integration:**
- [ ] Hardware rectangle fill (clear screen)
- [ ] Hardware sprite copy (opaque blitting)
- [ ] Hardware alpha blending (transparent sprites)
- [ ] Hardware color conversion (if needed)

**Touch/Inventory System:**
- [ ] Touch event handling (tap, drag)
- [ ] Inventory UI with touch support
- [ ] Calibration routine

### Asset Pipeline
- [ ] Update sprite converter for 480×320 target
- [ ] Consider asset compression (LZ4) for larger sprites
- [ ] Streaming system for large tilesets from SD card

---

## 11. Testing Plan

### Hardware Prototyping
1. **Breadboard Phase**: Test each component individually
   - Pico 2 with new display
   - Battery charging circuit
   - Button debouncing effectiveness

2. **Form Factor Mockup**: Cardboard/3D printed prototype
   - Test portrait vs landscape
   - Ergonomics testing
   - Button placement validation

3. **PCB Design**: Custom board layout
   - Integrate all components
   - Optimize routing for signal integrity
   - Power distribution design

### Software Testing
1. Test display driver with ST7789
2. Benchmark frame rates with larger resolution
3. Verify battery monitoring accuracy
4. Input latency testing with debouncing

---

## Notes
- Maintain backward compatibility where possible
- Design modular: allow incremental upgrades
- Document all pin assignments
- Keep schematics updated
- Consider manufacturing if design proves popular

---

**Last Updated**: 2026-01-18
**Status**: Hardware Selected - Ready for Purchase
