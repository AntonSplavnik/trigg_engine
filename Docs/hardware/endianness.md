# Endianness in Embedded Systems

## What is Endianness?

**Endianness** defines the byte order for storing multi-byte data types in memory.

### Example: 16-bit value `0x1234`

**Big-Endian** (most significant byte first):
```
Address:  0x1000    0x1001
Value:    0x12      0x34
          ^^ high   ^^ low
```

**Little-Endian** (least significant byte first):
```
Address:  0x1000    0x1001
Value:    0x34      0x12
          ^^ low    ^^ high
```

## Why It Matters in PocketGateEngine

### 1. RGB565 Format
When sending 16-bit RGB565 colors to the ST7735 display:

```cpp
uint16_t color = 0xF800;  // Red: 0b1111100000000000
```

**Memory layout depends on endianness:**
- **Little-Endian (Pico)**: `[0x00, 0xF8]` in memory
- **Big-Endian (Network/Display)**: `[0xF8, 0x00]` in memory

### 2. SPI Communication
The ST7735 display expects **big-endian** byte order (MSB first) over SPI.

**Problem**: Raspberry Pi Pico is **little-endian**.

**Solution**:
```cpp
// Option 1: Byte swap before sending
uint16_t color = 0xF800;
uint8_t high = (color >> 8) & 0xFF;    // 0xF8
uint8_t low = color & 0xFF;            // 0x00
spi_write_blocking(spi, &high, 1);
spi_write_blocking(spi, &low, 1);

// Option 2: Use __builtin_bswap16()
uint16_t swapped = __builtin_bswap16(color);
spi_write_blocking(spi, (uint8_t*)&swapped, 2);
```

### 3. Framebuffer Operations
When writing directly to framebuffer (before SPI send):

```cpp
// Store in native (little-endian) format for CPU efficiency
framebuffer[y * SCREEN_WIDTH + x] = color;  // uint16_t array

// Convert to big-endian only when sending to display
void send_framebuffer() {
    for (uint32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        uint16_t pixel = framebuffer[i];
        uint8_t msb = pixel >> 8;
        uint8_t lsb = pixel & 0xFF;
        // Send MSB first for display
        spi_write_blocking(spi, &msb, 1);
        spi_write_blocking(spi, &lsb, 1);
    }
}
```

## Common Pitfalls

### Wrong Color Output
```cpp
// WRONG: Sending little-endian directly
uint16_t red = 0xF800;
spi_write_blocking(spi, (uint8_t*)&red, 2);  // Sends 0x00, 0xF8 → Blue!

// CORRECT: Swap bytes first
uint8_t data[2] = {(red >> 8), red & 0xFF};
spi_write_blocking(spi, data, 2);  // Sends 0xF8, 0x00 → Red
```

### DMA Transfers
When using DMA for bulk framebuffer transfers:
- Pre-swap entire framebuffer to big-endian
- Or configure DMA byte-swap if hardware supports it
- Pico DMA doesn't auto-swap, so pre-process required

## Reference

### Pico Architecture
- **CPU**: ARM Cortex-M0+ (little-endian)
- **Memory**: Little-endian byte order
- **SPI**: Sends bytes in order (doesn't auto-swap)

### Display Expectations
- **ST7735**: Expects MSB first (big-endian) for 16-bit colors
- **Command bytes**: Single byte, no endianness issue
- **Data parameters**: Multi-byte coordinates/colors need MSB first

## Best Practices

1. **Store native**: Keep framebuffer in little-endian (CPU native)
2. **Convert on send**: Byte-swap only when transmitting via SPI
3. **Batch processing**: Swap entire framebuffer before DMA transfer
4. **Use helpers**: Create `to_big_endian()` utility function
5. **Profile**: Batch swapping is faster than per-pixel conversion

## Quick Reference

| Component          | Endianness    | Notes               |
|--------------------|---------------|---------------------|
| Pico CPU           | Little-endian | Native format       |
| Framebuffer        | Little-endian | Optimize for CPU    |
| SPI Transmission   | Big-endian    | Display requirement |
| RGB565 storage     | Little-endian | In memory/Flash     |
| RGB565 wire format | Big-endian    | Over SPI            |
