# STM32H743 + 4" ST7796S Display Wiring Guide

## Overview

This document describes the DuPont wire connections between the **WeAct STM32H743VIT6** development board and the **4" ST7796S IPS display** with **FT6336U capacitive touch**.

**Date**: 2026-01-30
**Hardware Version**: WeAct STM32H7XX Board V1.2

---

## Hardware Components

| Component | Model | Interface |
|-----------|-------|-----------|
| MCU Board | WeAct STM32H743VIT6 | - |
| Display | ST7796S 4" IPS 480x320 | SPI |
| Touch Controller | FT6336U | I2C |
| SD Card | On display module | SPI (shared) |

---

## Display Module Pinout (14-pin header)

Viewing display from the back, pins top to bottom:

| Pin # | Label | Function | Direction |
|-------|-------|----------|-----------|
| 1 | SD_CS | SD card chip select | Output |
| 2 | CTP_INT | Touch interrupt | Input |
| 3 | CTP_SDA | Touch I2C data | Bidirectional |
| 4 | CTP_RST | Touch reset | Output |
| 5 | CTP_SCL | Touch I2C clock | Output |
| 6 | SDO (MISO) | SPI data out | Input |
| 7 | LED | Backlight control | Output |
| 8 | SCK | SPI clock | Output |
| 9 | SDI (MOSI) | SPI data in | Output |
| 10 | LCD_RS | Data/Command (DC) | Output |
| 11 | LCD_RST | LCD reset | Output |
| 12 | LCD_CS | LCD chip select | Output |
| 13 | GND | Ground | Power |
| 14 | VCC | 3.3V supply | Power |

---

## STM32H743 Pin Assignment

### Complete Wiring Table

| Display Pin | Function | STM32 Pin | GPIO Port | Peripheral | Header Position |
|-------------|----------|-----------|-----------|------------|-----------------|
| VCC | 5V Power (recommended) | 5V | - | Power | Right, Row 20 |
| GND | Ground | GND | - | Power | Right, Row 21 |
| LCD_CS | LCD Chip Select | PE11 | GPIOE | GPIO Output | Right, Row 16 |
| LCD_RST | LCD Reset | PE10 | GPIOE | GPIO Output | Right, Row 16 |
| LCD_RS | Data/Command | PE9 | GPIOE | GPIO Output | Right, Row 15 |
| SDI (MOSI) | SPI Data In | PB5 | GPIOB | SPI3_MOSI | Left, Row 4 |
| SCK | SPI Clock | PB3 | GPIOB | SPI3_SCK | Left, Row 5 |
| SDO (MISO) | SPI Data Out | PB4 | GPIOB | SPI3_MISO | Left, Row 4 |
| LED | Backlight | PE8 | GPIOE | TIM1_CH1N / GPIO | Right, Row 15 |
| CTP_SCL | Touch I2C Clock | PB8 | GPIOB | I2C1_SCL | Left, Row 2 |
| CTP_SDA | Touch I2C Data | PB9 | GPIOB | I2C1_SDA | Left, Row 2 |
| CTP_RST | Touch Reset | PE7 | GPIOE | GPIO Output | Right, Row 14 |
| CTP_INT | Touch Interrupt | PE6 | GPIOE | EXTI6 | Right, Row 3 |
| SD_CS | SD Card CS | PE5 | GPIOE | GPIO Output | Right, Row 2 |

### Peripheral Summary

| Peripheral | Function | Pins Used |
|------------|----------|-----------|
| SPI3 | Display + SD Card | PB3 (SCK), PB4 (MISO), PB5 (MOSI) |
| I2C1 | Touch Controller | PB8 (SCL), PB9 (SDA) |
| TIM1_CH1N | Backlight PWM | PE8 (optional, can use GPIO) |
| EXTI6 | Touch Interrupt | PE6 |

---

## Wire Color Coding

Using 10 available colors for 14 connections:

| Color | Wire 1 | Wire 2 (if repeated) |
|-------|--------|----------------------|
| Red | VCC (3.3V) | - |
| Black | GND | - |
| Purple | SCK | - |
| Blue | SDI (MOSI) | - |
| Grey | SDO (MISO) | - |
| White | LED | - |
| Brown | CTP_SCL | CTP_SDA (I2C pair) |
| Orange | LCD_CS | SD_CS (chip selects) |
| Yellow | LCD_RST | CTP_RST (reset lines) |
| Green | LCD_RS | CTP_INT (control lines) |

### Color Logic for Repeated Wires

- **Brown x2**: I2C bus pair (SCL + SDA always together)
- **Orange x2**: Both are chip select signals
- **Yellow x2**: Both are reset signals
- **Green x2**: LCD control vs Touch control (different areas)

---

## Physical Wiring Diagram

### STM32H743 Board Back View (Pin Labels)

```
              LEFT HEADER (P1)              RIGHT HEADER (P2)
             ┌─────┬─────┐                 ┌─────┬─────┐
     Row 1   │ E1  │ E0  │                 │ E2  │ E3  │   Row 1
     Row 2   │ B9  │ B8  │ ◄─ I2C         │ E4  │ E5  │   Row 2  ◄─ SD_CS
     Row 3   │ B7  │ B6  │                 │ E6  │ VB  │   Row 3  ◄─ CTP_INT
     Row 4   │ B5  │ B4  │ ◄─ SPI         │ C13 │ NR  │   Row 4
     Row 5   │ B3  │ D7  │ ◄─ SCK         │ C0  │ C1  │   Row 5
     Row 6   │ D6  │ D5  │                 │ C2  │ C3  │   Row 6
     Row 7   │ D4  │ D3  │                 │ GND │ V+  │   Row 7
     Row 8   │ D2  │ D1  │                 │ A0  │ A1  │   Row 8
     Row 9   │ D0  │ C12 │                 │ A2  │ A3  │   Row 9
     Row 10  │ C11 │ C10 │                 │ A4  │ A5  │   Row 10
     Row 11  │ A15 │ A12 │                 │ A6  │ A7  │   Row 11
     Row 12  │ A11 │ A10 │                 │ C4  │ C5  │   Row 12
     Row 13  │ A9  │ A8  │                 │ B0  │ B1  │   Row 13
     Row 14  │ C9  │ C8  │                 │ B2  │ E7  │   Row 14 ◄─ CTP_RST
     Row 15  │ C7  │ C6  │                 │ E8  │ E9  │   Row 15 ◄─ LED, LCD_RS
     Row 16  │ D15 │ D14 │                 │ E10 │ E11 │   Row 16 ◄─ LCD_RST, LCD_CS
     Row 17  │ D13 │ D12 │                 │ E12 │ E13 │   Row 17
     Row 18  │ D11 │ D10 │                 │ E14 │ E15 │   Row 18
     Row 19  │ D9  │ D8  │                 │ B10 │ B11 │   Row 19
     Row 20  │ B15 │ B14 │                 │ 3V3 │ 5V  │   Row 20 ◄─ POWER
     Row 21  │ B13 │ B12 │                 │ GND │ GND │   Row 21 ◄─ GROUND
             └─────┴─────┘                 └─────┴─────┘
```

### Connection Diagram

```
    DISPLAY MODULE                           STM32H743 BOARD
    ┌────────────────┐
    │                │                       LEFT        RIGHT
    │   4" IPS LCD   │                      HEADER      HEADER
    │   480 x 320    │                     ┌──────┐    ┌──────┐
    │                │                     │      │    │      │
    │                │                     │ B9 ◄─┼────┼──────┤ Brown  (CTP_SDA)
    │                │                     │ B8 ◄─┼────┼──────┤ Brown  (CTP_SCL)
    │                │                     │      │    │      │
    │                │                     │ B5 ◄─┼────┼──────┤ Blue   (MOSI)
    │                │                     │ B4 ◄─┼────┼──────┤ Grey   (MISO)
    │                │                     │ B3 ◄─┼────┼──────┤ Purple (SCK)
    └────────────────┘                     │      │    │      │
    ┌────────────────┐                     │      │    │ E5 ◄─┤ Orange (SD_CS)
    │ 14-Pin Header  │                     │      │    │ E6 ◄─┤ Green  (CTP_INT)
    ├────────────────┤                     │      │    │ E7 ◄─┤ Yellow (CTP_RST)
    │ 1  SD_CS    ───┼─── Orange ──────────┼──────┼───►│ E8 ◄─┤ White  (LED)
    │ 2  CTP_INT  ───┼─── Green ───────────┼──────┼───►│ E9 ◄─┤ Green  (LCD_RS)
    │ 3  CTP_SDA  ───┼─── Brown ───────────┼─►    │    │ E10◄─┤ Yellow (LCD_RST)
    │ 4  CTP_RST  ───┼─── Yellow ──────────┼──────┼───►│ E11◄─┤ Orange (LCD_CS)
    │ 5  CTP_SCL  ───┼─── Brown ───────────┼─►    │    │      │
    │ 6  SDO/MISO ───┼─── Grey ────────────┼─►    │    │      │
    │ 7  LED      ───┼─── White ───────────┼──────┼───►│      │
    │ 8  SCK      ───┼─── Purple ──────────┼─►    │    │      │
    │ 9  SDI/MOSI ───┼─── Blue ────────────┼─►    │    │      │
    │ 10 LCD_RS   ───┼─── Green ───────────┼──────┼───►│      │
    │ 11 LCD_RST  ───┼─── Yellow ──────────┼──────┼───►│      │
    │ 12 LCD_CS   ───┼─── Orange ──────────┼──────┼───►│      │
    │ 13 GND      ───┼─── Black ───────────┼──────┼───►│ GND  │
    │ 14 VCC      ───┼─── Red ─────────────┼──────┼───►│ 3V3  │
    └────────────────┘                     └──────┘    └──────┘
```

---

## Wiring Checklist

### Left Header Connections (5 wires)
- [ ] Brown → PB9 (Row 2, left pin) - CTP_SDA
- [ ] Brown → PB8 (Row 2, right pin) - CTP_SCL
- [ ] Blue → PB5 (Row 4, left pin) - MOSI
- [ ] Grey → PB4 (Row 4, right pin) - MISO
- [ ] Purple → PB3 (Row 5, left pin) - SCK

### Right Header Connections (9 wires)
- [ ] Orange → PE5 (Row 2, left pin) - SD_CS
- [ ] Green → PE6 (Row 3, left pin) - CTP_INT
- [ ] Yellow → PE7 (Row 14, right pin) - CTP_RST
- [ ] White → PE8 (Row 15, left pin) - LED
- [ ] Green → PE9 (Row 15, right pin) - LCD_RS
- [ ] Yellow → PE10 (Row 16, left pin) - LCD_RST
- [ ] Orange → PE11 (Row 16, right pin) - LCD_CS
- [ ] Red → 3V3 (Row 20, left pin) - Power
- [ ] Black → GND (Row 21, any) - Ground

---

## Software Configuration

### STM32CubeMX Settings

#### SPI3 Configuration
```
Mode: Full-Duplex Master
Hardware NSS: Disable
Prescaler: Start with 32 (7.5 MHz), reduce for higher speed
CPOL: Low
CPHA: 1 Edge
Data Size: 8 Bits
First Bit: MSB
```

#### I2C1 Configuration
```
Mode: I2C
Speed Mode: Fast Mode (400 kHz)
Clock Frequency: 400000
Address Length: 7-bit
```

#### GPIO Configuration
```
PE5  - GPIO_Output - SD_CS      - High (deselected)
PE6  - GPIO_EXTI6  - CTP_INT    - Falling edge trigger
PE7  - GPIO_Output - CTP_RST    - High (not reset)
PE8  - GPIO_Output - LED        - High (backlight on)
PE9  - GPIO_Output - LCD_RS     - Data/Command select
PE10 - GPIO_Output - LCD_RST    - High (not reset)
PE11 - GPIO_Output - LCD_CS     - High (deselected)
```

### Pin Definitions (C Header)

```c
// Display SPI Pins (directly on SPI3 peripheral)
#define LCD_SPI                 SPI3

// Display Control Pins
#define LCD_CS_PIN              GPIO_PIN_11
#define LCD_CS_PORT             GPIOE
#define LCD_RST_PIN             GPIO_PIN_10
#define LCD_RST_PORT            GPIOE
#define LCD_DC_PIN              GPIO_PIN_9      // LCD_RS = Data/Command
#define LCD_DC_PORT             GPIOE
#define LCD_LED_PIN             GPIO_PIN_8
#define LCD_LED_PORT            GPIOE

// Touch Controller Pins
#define CTP_I2C                 I2C1
#define CTP_RST_PIN             GPIO_PIN_7
#define CTP_RST_PORT            GPIOE
#define CTP_INT_PIN             GPIO_PIN_6
#define CTP_INT_PORT            GPIOE
#define CTP_I2C_ADDR            0x38            // FT6336U default address

// SD Card Pin
#define SD_CS_PIN               GPIO_PIN_5
#define SD_CS_PORT              GPIOE

// Macros
#define LCD_CS_LOW()            HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()           HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_DC_DATA()           HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)
#define LCD_DC_CMD()            HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define LCD_RST_LOW()           HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define LCD_RST_HIGH()          HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET)
#define LCD_LED_ON()            HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_SET)
#define LCD_LED_OFF()           HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_RESET)
```

---

## Electrical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| VCC | 5V (recommended) or 3.3V | From STM32 5V pin |
| Logic Level | 3.3V | STM32H7 native level |
| SPI Clock | 10-80 MHz | Start low, increase after testing |
| I2C Clock | 400 kHz | Fast Mode |
| Backlight Current | ~40mA | Via LED pin, can use PWM |

---

## Troubleshooting

### Display Not Working
1. Check VCC and GND connections
2. Verify LCD_CS is going LOW during transfers
3. Check SPI clock with oscilloscope (start at 1 MHz)
4. Ensure LCD_RST pulse at startup (LOW 10ms, then HIGH)

### Touch Not Responding
1. Check I2C pull-ups (4.7K on board)
2. Verify CTP_RST is HIGH
3. Scan I2C bus for device (address 0x38)
4. Check CTP_INT signal on touch

### White/Black Screen
1. Backlight issue: Check LED pin
2. Wrong initialization sequence
3. SPI speed too high - reduce prescaler

### Garbled Display
1. SPI mode wrong (try CPOL=0, CPHA=0)
2. Data/Command (LCD_RS) timing issue
3. SPI speed too high for wiring

---

## References

- WeAct STM32H7XX Schematic V1.2
- ST7796S Datasheet
- FT6336U Datasheet
- STM32H743 Reference Manual

---

**Last Updated**: 2026-01-30
**Author**: PocketGateEngine Project
