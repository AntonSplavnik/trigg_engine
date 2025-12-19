# SPI (Serial Peripheral Interface)

**What is SPI?**

SPI is a communication protocol. Think of it like a one-way phone line where the Pico talks to the display very fast. The Pico is the **Master** (controls the conversation) and the display is the **Slave** (listens and responds).

**How it works:**
- Data is sent one bit at a time (serial)
- The clock signal synchronizes timing
- Very fast: 62.5 MHz = 62.5 million bits per second!

---

## Communication Pattern

### Sending a Command:
```
1. Set CS LOW       (select display)
2. Set DC LOW       (command mode)
3. Send command byte via SPI
4. Set CS HIGH      (deselect display)
```

### Sending Data:
```
1. Set CS LOW       (select display)
2. Set DC HIGH      (data mode)
3. Send data bytes via SPI
4. Set CS HIGH      (deselect display)
```

---

## Sharing the SPI Bus

Multiple devices (display and SD card) can share the same SPI bus:
- Both use the same SCK, MOSI, MISO pins
- Each device has its own CS (Chip Select) pin
- Only the device with CS LOW will respond
- **Never set both CS pins LOW simultaneously**

---

## Clock Speed and Timing

### What is Frequency?

**Frequency = how many times per second something happens**

- Measured in **Hertz (Hz)** = cycles per second
- 62.5 MHz = 62,500,000 cycles per second
- Each cycle = one "tick" of the clock signal

### The SPI Clock Signal

SPI uses a **clock signal** to synchronize data transfer:

```
Clock:  ___╱¯¯¯╲___╱¯¯¯╲___╱¯¯¯╲___╱¯¯¯╲___
Data:   ___[bit0][bit1][bit2][bit3]___

Each clock pulse = transfer 1 bit of data
```

**At 62.5 MHz:**
- 62.5 million clock pulses per second
- = 62.5 million bits per second
- = ~7.8 million bytes per second (÷8 bits/byte)

### System Clock vs SPI Clock

The **Raspberry Pi Pico system clock** runs at **125 MHz** - this is the CPU's base speed.

All peripherals (SPI, I2C, UART, etc.) derive their clocks from the system clock:

```
System Clock (125 MHz)
        ↓
    [Divider ÷2]
        ↓
   SPI Clock (62.5 MHz)
```

### Why 62.5 MHz?

The frequency is chosen for **maximum reliable performance**:

1. **Clean Division**: 125 MHz ÷ 2 = 62.5 MHz (perfect divisor)
2. **Hardware Limit**: ST7735 display can typically handle 15-30 MHz maximum
3. **Signal Integrity**: Higher speeds may cause data corruption on wires
4. **Performance**: Fast enough for 60 FPS rendering

**Why not higher?**
- Display controller may not keep up
- Signal quality degrades at very high frequencies
- PCB traces and breadboard connections have limits

**Why not lower?**
- Slower display updates
- Won't achieve 60 FPS target
- Wastes available bandwidth

### Transfer Speed Calculations

**Example: Sending one full frame to display**

Framebuffer size: 40KB (128 × 160 × 2 bytes RGB565)

At 62.5 MHz SPI:
- Theoretical: 7,800,000 bytes/second
- Time per frame: 40,000 ÷ 7,800,000 ≈ **5ms**
- Maximum possible: ~200 FPS

Real-world performance is lower due to:
- CPU processing time
- Command overhead (not just pixel data)
- Memory access latency
- DMA setup time

**Realistic target: 60 FPS** (16.7ms per frame)

### Code Reference

See `drivers/display.cpp:9` for SPI initialization:
```cpp
spi_init(spi0, 62500000);  // 62.5 MHz
```
