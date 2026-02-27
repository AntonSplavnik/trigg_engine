# DMA (Direct Memory Access)

**What is DMA?**

DMA is a hardware feature that transfers data between memory and peripherals **without CPU involvement**. Think of it as a dedicated delivery truck that moves data around while the CPU (the manager) is free to do other work.

The CPU only needs to set up the transfer (source, destination, size), then the DMA controller handles everything automatically.

---

## Why DMA Matters

### Without DMA (CPU does everything)

```
CPU: Read byte → Store → Read byte → Store → Read byte → Store...
     ████████████████████████████████████████████████████████████
     (CPU completely blocked during transfer)
```

The CPU must:
1. Read each byte from source
2. Write each byte to destination
3. Repeat for every single byte
4. Cannot do anything else during transfer

### With DMA (hardware handles transfer)

```
CPU:  Setup DMA → Do other work............... → Done interrupt
      ██         ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ██

DMA:       ████████████████████████████████████
           (transfers data in background)
```

The CPU only:
1. Configures DMA (source, destination, size)
2. Starts transfer
3. Does other work (rendering, game logic, etc.)
4. Gets notified when complete

---

## DMA Transfer Types

### Memory-to-Peripheral (M2P)

Send data from RAM to a hardware device.

```
┌─────────┐         ┌─────────┐
│   RAM   │ ──DMA─► │   SPI   │ ──► Display
│ (buffer)│         │(TX reg) │
└─────────┘         └─────────┘
```

**Use case:** Sending framebuffer to display via SPI

### Peripheral-to-Memory (P2M)

Receive data from hardware device into RAM.

```
┌─────────┐         ┌─────────┐
│ SD Card │ ──► │   SPI   │ ──DMA─► │   RAM   │
│         │     │(RX reg) │         │ (buffer)│
└─────────┘     └─────────┘         └─────────┘
```

**Use case:** Loading game assets from SD card

### Memory-to-Memory (M2M)

Copy data between RAM regions.

```
┌─────────┐         ┌─────────┐
│  RAM A  │ ──DMA─► │  RAM B  │
│ (source)│         │ (dest)  │
└─────────┘         └─────────┘
```

**Use case:** Copying sprite data, clearing buffers

---

## Practical Example: Display Without DMA

```cpp
// WITHOUT DMA - CPU blocked for entire transfer
void send_framebuffer_blocking(uint16_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        // Wait for SPI ready
        while (!(SPI->SR & SPI_SR_TXE));
        // Send byte - CPU stuck in this loop!
        SPI->DR = buffer[i];
    }
    // Wait for transfer complete
    while (SPI->SR & SPI_SR_BSY);
}

// Game loop - SLOW
void game_loop() {
    while (true) {
        render_frame(back_buffer);      // ~8ms
        send_framebuffer_blocking();    // ~5ms (CPU waiting!)
        // Total: 13ms, but CPU idle for 5ms
    }
}
```

**Problem:** CPU wastes 5ms per frame doing nothing but waiting.

---

## Practical Example: Display With DMA

```cpp
// WITH DMA - CPU free during transfer
volatile bool dma_busy = false;

void DMA_IRQHandler() {
    if (DMA->ISR & DMA_ISR_TCIF) {  // Transfer complete
        DMA->IFCR = DMA_IFCR_CTCIF; // Clear flag
        dma_busy = false;
    }
}

void send_framebuffer_dma(uint16_t* buffer, size_t size) {
    dma_busy = true;

    DMA->CMAR = (uint32_t)buffer;      // Source: RAM buffer
    DMA->CPAR = (uint32_t)&SPI->DR;    // Dest: SPI data register
    DMA->CNDTR = size;                  // Number of transfers
    DMA->CCR |= DMA_CCR_EN;            // Start transfer

    // Returns immediately - DMA runs in background
}

// Game loop - FAST (double buffering)
void game_loop() {
    while (true) {
        // Wait for previous DMA to finish (if needed)
        while (dma_busy);

        swap_buffers();                     // Instant pointer swap
        send_framebuffer_dma(front_buffer); // Start DMA (non-blocking)
        render_frame(back_buffer);          // Render WHILE DMA sends
        // CPU utilization: ~100%
    }
}
```

**Benefit:** CPU renders next frame while DMA sends current frame.

---

## Double Buffering with DMA

This is the key technique for smooth 60 FPS rendering:

```
Frame 1:
┌──────────────────────────────────────────────────────┐
│ CPU renders to Buffer A │ DMA sends Buffer B         │
└──────────────────────────────────────────────────────┘
                          ↓ swap pointers
Frame 2:
┌──────────────────────────────────────────────────────┐
│ CPU renders to Buffer B │ DMA sends Buffer A         │
└──────────────────────────────────────────────────────┘
                          ↓ swap pointers
Frame 3:
┌──────────────────────────────────────────────────────┐
│ CPU renders to Buffer A │ DMA sends Buffer B         │
└──────────────────────────────────────────────────────┘
```

**Without DMA:** Render → Wait → Send → Wait → Render (serial)
**With DMA:** Render + Send happen simultaneously (parallel)

---

## DMA on Different Platforms

### Raspberry Pi Pico (RP2040)

12 DMA channels, all memory regions accessible.

```cpp
#include "hardware/dma.h"

int dma_chan = dma_claim_unused_channel(true);

dma_channel_config c = dma_channel_get_default_config(dma_chan);
channel_config_set_transfer_data_size(&c, DMA_SIZE_16);  // 16-bit
channel_config_set_dreq(&c, spi_get_dreq(spi0, true));   // SPI TX

dma_channel_configure(
    dma_chan,
    &c,
    &spi_get_hw(spi0)->dr,  // Destination: SPI data register
    buffer,                  // Source: framebuffer
    count,                   // Number of transfers
    true                     // Start immediately
);
```

### STM32H743

Multiple DMA controllers with domain restrictions:

| Controller | Access                | Best For               |
|------------|-----------------------|------------------------|
| DMA1/DMA2  | D2 domain (SRAM1/2/3) | Peripheral transfers   |
| MDMA.      | All domains           | Cross-domain transfers |
| BDMA       | D3 domain (SRAM4)     | Low-power operations   |

```cpp
// STM32 HAL example
HAL_SPI_Transmit_DMA(&hspi1, buffer, size);

// Callback when complete
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    dma_complete = true;
}
```

**Important:** On STM32H743, DMA1/DMA2 cannot access AXI SRAM (0x24000000) or DTCM (0x20000000). Place DMA buffers in SRAM1/2/3 (0x30000000).

---

## DMA Memory Placement (STM32H743)

```c
// WRONG - DMA cannot access DTCM
uint16_t framebuffer[128*160];  // Placed in DTCM by default

// CORRECT - Place in D2 SRAM for DMA access
__attribute__((section(".sram1")))
uint16_t framebuffer[128*160];

// Linker script section
SECTIONS {
    .sram1 (NOLOAD) : {
        *(.sram1)
    } > SRAM1
}
```

---

## DMA vs CPU: Performance Comparison

### Sending 40KB Framebuffer

| Method                 | CPU Time | Transfer Time | CPU Available |
|------------------------|----------|---------------|---------------|
| CPU polling            | 5ms      | 5ms           | 0%            |
| CPU interrupt per byte | 5ms.     | 5ms           | ~10%          |
| DMA                    | 0.01ms   | 5ms           | 99.8%         |

### Frame Budget at 60 FPS (16.67ms)

```
Without DMA:
├─ Render: 8ms ─┤├─ Send: 5ms (blocked) ─┤
Total: 13ms, but only 8ms productive

With DMA:
├─ Render: 8ms ────────────────┤
├─ Send (DMA): 5ms ─┤ (parallel)
Total: 8ms, all productive
```

---

## DMA Considerations

### Advantages
- CPU free during transfers
- Higher throughput (dedicated hardware)
- Enables double buffering
- Consistent timing (no CPU jitter)

### Limitations
- Setup overhead (~1μs) - not worth it for small transfers (<100 bytes)
- Memory alignment requirements (often 4-byte aligned)
- Memory region restrictions (STM32H7 domains)
- Limited channels (may need to share/prioritize)

### When to Use DMA

**Use DMA for:**
- Framebuffer transfers (40KB+)
- Audio streaming
- SD card reads/writes
- Any transfer > 100 bytes

**Don't use DMA for:**
- Single register writes
- Small configuration data
- One-time initialization

---

## DMA Interrupts

DMA can trigger interrupts at different points:

```cpp
// Transfer complete - all data sent
void DMA_TC_Handler() {
    // Safe to reuse buffer
    // Start next operation
}

// Half transfer - half the data sent
void DMA_HT_Handler() {
    // Useful for circular buffers (audio)
    // Fill first half while second half sends
}

// Transfer error
void DMA_TE_Handler() {
    // Handle bus errors, alignment issues
}
```

---

## Circular DMA (Audio Example)

For continuous streaming (audio playback):

```cpp
// Audio buffer - DMA loops continuously
uint16_t audio_buffer[2048];  // Two halves: 0-1023, 1024-2047

void setup_circular_dma() {
    // Enable circular mode
    DMA->CCR |= DMA_CCR_CIRC;  // Circular mode
    DMA->CCR |= DMA_CCR_HTIE;  // Half-transfer interrupt
    DMA->CCR |= DMA_CCR_TCIE;  // Transfer complete interrupt
}

void DMA_HT_Handler() {
    // First half sent, fill it with new data
    fill_audio(&audio_buffer[0], 1024);
}

void DMA_TC_Handler() {
    // Second half sent, fill it with new data
    fill_audio(&audio_buffer[1024], 1024);
}
```

```
Buffer: [====HALF A====][====HALF B====]
         ↑ fill here     ↑ DMA sending

        [====HALF A====][====HALF B====]
         ↑ DMA sending   ↑ fill here
```

---

## PocketGateEngine DMA Usage

### Display Transfer

For sending framebuffer to ST7735/ST7789 display:

```cpp
// Initialize DMA for SPI display
void display_init_dma() {
    dma_channel = dma_claim_unused_channel(true);

    dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, spi_get_dreq(SPI_PORT, true));
}

// Send framebuffer (non-blocking)
void display_send_buffer_dma(uint16_t* buffer) {
    // Set display window (blocking - small command)
    display_set_window(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Start DMA transfer (non-blocking - large data)
    dma_channel_configure(
        dma_channel,
        &dma_config,
        &spi_get_hw(SPI_PORT)->dr,
        buffer,
        SCREEN_WIDTH * SCREEN_HEIGHT,
        true  // Start immediately
    );
}

// Check if DMA finished
bool display_dma_busy() {
    return dma_channel_is_busy(dma_channel);
}
```

### Game Loop Integration

```cpp
uint16_t* front_buffer;
uint16_t* back_buffer;

void game_loop() {
    while (true) {
        // 1. Wait for previous frame's DMA (if still running)
        while (display_dma_busy());

        // 2. Swap buffers
        std::swap(front_buffer, back_buffer);

        // 3. Start sending front buffer via DMA
        display_send_buffer_dma(front_buffer);

        // 4. Render next frame to back buffer (while DMA runs)
        clear_buffer(back_buffer);
        render_sprites(back_buffer);
        render_ui(back_buffer);

        // 5. Process input and game logic
        update_game();
    }
}
```

---

## Common DMA Mistakes

### 1. Buffer Modified During Transfer

```cpp
// WRONG - modifying buffer while DMA is using it
send_buffer_dma(buffer);
buffer[0] = 0xFF;  // Corruption! DMA might read old or new value

// CORRECT - wait for completion or use double buffering
send_buffer_dma(buffer);
while (dma_busy());  // Wait
buffer[0] = 0xFF;    // Safe now
```

### 2. Wrong Memory Region (STM32H7)

```cpp
// WRONG - DTCM not accessible by DMA1/DMA2
uint16_t buffer[1024];  // Goes to DTCM

// CORRECT - use D2 SRAM
__attribute__((section(".sram1"))) uint16_t buffer[1024];
```

### 3. Alignment Issues

```cpp
// WRONG - unaligned buffer for 32-bit DMA
uint8_t data[100];
uint32_t* ptr = (uint32_t*)&data[1];  // Misaligned!

// CORRECT - ensure alignment
__attribute__((aligned(4))) uint8_t data[100];
```

### 4. Forgetting Cache (Cortex-M7)

```cpp
// On STM32H7 with cache enabled:
// WRONG - cache may contain stale data
receive_dma(buffer);
while (dma_busy());
process(buffer);  // May read cached (old) data!

// CORRECT - invalidate cache after DMA write
receive_dma(buffer);
while (dma_busy());
SCB_InvalidateDCache_by_Addr(buffer, size);
process(buffer);  // Fresh data from RAM
```

---

## Reference

- Raspberry Pi Pico SDK: `hardware/dma.h`
- STM32H7 Reference Manual: DMA chapter
- See `Docs/hardware/spi.md` for SPI + DMA integration
