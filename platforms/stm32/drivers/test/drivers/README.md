# PocketGateEngine - STM32H743 Build

PlatformIO project for WeAct STM32H743VIT6 with 4" ST7796S display.

## Hardware

- **MCU**: STM32H743VIT6 (480 MHz, 1MB RAM)
- **Display**: ST7796S 4" IPS 480x320
- **Touch**: FT6336U Capacitive (I2C)
- **Board**: WeAct STM32H7XX V1.2

## Quick Start

### 1. Install PlatformIO

```bash
# VS Code extension (recommended)
# Or CLI:
pip install platformio
```

### 2. Build

```bash
cd stm32
pio run
```

### 3. Upload (via ST-Link)

Connect ST-Link to SWD header (GND, SWCLK, SWDIO, 3.3V):

```bash
pio run -t upload
```

### 4. Monitor (via SWO/ITM)

```bash
pio device monitor
```

## Project Structure

```
stm32/
├── platformio.ini          # PlatformIO configuration
├── include/
│   └── board_config.h      # Pin definitions, hardware config
├── src/
│   ├── main.cpp            # Entry point
│   └── drivers/
│       ├── st7796s_driver.h    # Display driver header
│       └── st7796s_driver.cpp  # Display driver implementation
└── README.md
```

## Pin Mapping

See `Docs/hardware_docs/stm32h743/STM32H743_4INCH_DISPLAY_WIRING.md` for complete wiring guide.

### Quick Reference

| Function  | STM32 Pin | Peripheral |
|-----------|-----------|------------|
| SPI3_SCK  | PB3       | SPI3       |
| SPI3_MISO | PB4       | SPI3       |
| SPI3_MOSI | PB5       | SPI3       |
| I2C1_SCL  | PB8       | I2C1       |
| I2C1_SDA  | PB9       | I2C1       |
| LCD_CS    | PE11      | GPIO       |
| LCD_RST   | PE10      | GPIO       |
| LCD_DC    | PE9       | GPIO       |
| LCD_LED   | PE8       | GPIO       |
| CTP_RST   | PE7       | GPIO       |
| CTP_INT   | PE6       | EXTI       |
| SD_CS     | PE5       | GPIO       |

## Configuration

### Clock Configuration

- HSE: 25 MHz (external crystal)
- SYSCLK: 480 MHz
- HCLK: 240 MHz
- APB1/APB2: 120 MHz

### SPI Speed

Edit `platformio.ini` or `board_config.h`:

```c
#define SPI_PRESCALER_SAFE  SPI_BAUDRATEPRESCALER_16  // 7.5 MHz (safe)
#define SPI_PRESCALER_FAST  SPI_BAUDRATEPRESCALER_4   // 30 MHz (tested)
#define SPI_PRESCALER_MAX   SPI_BAUDRATEPRESCALER_2   // 60 MHz (aggressive)
```

## Build Environments

| Environment | Description |
|-------------|-------------|
| `weact_h743` | Release build (-O2) |
| `weact_h743_debug` | Debug build (-O0, symbols) |

```bash
# Build specific environment
pio run -e weact_h743_debug
```

## Troubleshooting

### Upload fails

1. Check ST-Link connection (4 wires: GND, SWCLK, SWDIO, 3.3V)
2. Hold BOOT0 button while connecting power
3. Try `pio run -t upload --upload-port /dev/cu.usbmodem*`

### Display not working

1. Check wiring against documentation
2. Verify 3.3V power
3. Check SPI signals with oscilloscope
4. Reduce SPI speed (increase prescaler)

### No serial output

Printf goes to ITM/SWO, requires:
- SWO pin connected to debugger
- Debugger configured for SWO trace
- Or use UART for printf instead

## TODO

- [ ] DMA transfers for display
- [ ] FT6336U touch driver
- [ ] Framebuffer class (shared with Pico)
- [ ] DMA2D hardware acceleration
- [ ] SD card support
