# Memory Types (SRAM vs DRAM)

**What is RAM?**

RAM (Random Access Memory) means you can read or write any address directly - you don't have to access data sequentially. Both SRAM and DRAM are types of RAM.

---

## SRAM vs DRAM

```
                    RAM (Random Access Memory)
                              │
              ┌───────────────┴───────────────┐
              │                               │
           SRAM                            DRAM
      (Static RAM)                   (Dynamic RAM)
    Used in: MCUs, cache           Used in: PCs, phones
```

### Key Differences

| Feature | SRAM | DRAM |
|---------|------|------|
| **Speed** | Very fast (1-2 cycles) | Slower (needs refresh) |
| **Density** | Low (6 transistors/bit) | High (1 transistor/bit) |
| **Size** | Small (KB - few MB) | Large (GB) |
| **Cost** | Expensive (~$5000/GB) | Cheap (~$5/GB) |
| **Power** | Low idle, no refresh | Higher (constant refresh) |
| **Refresh** | Not needed | Every ~64ms or data lost |

---

## How They Work

### SRAM (Static RAM)

Uses 6 transistors arranged as a flip-flop circuit to store each bit.

```
        VDD
         │
    ┌────┴────┐
    │         │
   ─┤►  ◄├─   ← Cross-coupled inverters
    │         │     (flip-flop)
    └────┬────┘
         │
        GND

Data stays stable as long as power is on = "Static"
```

**Characteristics:**
- Data holds without refresh
- Very fast access (~1 CPU cycle)
- Large physical size (6 transistors per bit)
- Used where speed matters more than density

### DRAM (Dynamic RAM)

Uses 1 transistor + 1 capacitor to store each bit.

```
    Word Line
        │
        ├──┤ Transistor
        │
       ═╧═  Capacitor (stores charge = data)
        │
    Bit Line

Capacitor leaks charge → must refresh every 64ms = "Dynamic"
```

**Characteristics:**
- Capacitor charge leaks over time
- Must refresh all cells periodically
- Very dense (1 transistor per bit)
- Used where capacity matters more than speed

---

## Why Microcontrollers Use SRAM

### 1. No Refresh Circuitry

DRAM needs a memory controller that constantly refreshes cells:

```
DRAM refresh cycle (every 64ms):
┌─────────────────────────────────────────┐
│ Row 0: Read → Amplify → Write back      │
│ Row 1: Read → Amplify → Write back      │
│ Row 2: Read → Amplify → Write back      │
│ ...                                     │
│ Row N: Read → Amplify → Write back      │
└─────────────────────────────────────────┘
Memory unavailable during refresh!
```

SRAM has none of this complexity.

### 2. Deterministic Timing

For real-time systems, predictable timing is critical:

```
SRAM:  Request → Data (always ~1 cycle)
DRAM:  Request → Maybe wait for refresh → Data (variable)
```

### 3. Can Be On-Chip

SRAM integrates directly onto the MCU die:

```
┌─────────────────────────────────┐
│         MCU Chip                │
│  ┌─────┐  ┌─────┐  ┌─────────┐ │
│  │ CPU │──│ Bus │──│  SRAM   │ │  ← Same chip, fast
│  └─────┘  └─────┘  └─────────┘ │
└─────────────────────────────────┘

vs

┌─────────────┐      ┌─────────────┐
│  MCU Chip   │──────│  DRAM Chip  │  ← External, slower
│  (no RAM)   │ bus  │   (DDR4)    │
└─────────────┘      └─────────────┘
```

### 4. Lower Power at Idle

SRAM: Power only when accessed
DRAM: Power constantly for refresh

---

## Why PCs Use DRAM

### 1. Need Gigabytes of Memory

```
16GB SRAM = ~96 billion transistors just for memory
           = impossibly expensive and large

16GB DRAM = ~16 billion transistors
           = fits on a few small chips, affordable
```

### 2. Cost Per Gigabyte

| Amount | SRAM Cost | DRAM Cost |
|--------|-----------|-----------|
| 1 MB | ~$5 | ~$0.005 |
| 1 GB | ~$5,000 | ~$5 |
| 16 GB | ~$80,000 | ~$50 |

### 3. Refresh Overhead Acceptable

Desktop/laptop applications aren't hard real-time - occasional microsecond delays are fine.

---

## Memory Hierarchy

Modern systems use both types strategically:

```
┌─────────────────────────────────────────────────────┐
│                     CPU                             │
│  ┌─────────┐                                        │
│  │ Registers│  ← Fastest, ~10 bytes                 │
│  └────┬────┘                                        │
│       │                                             │
│  ┌────▼────┐                                        │
│  │ L1 Cache│  ← SRAM, 32-64KB, ~1 cycle            │
│  └────┬────┘                                        │
│       │                                             │
│  ┌────▼────┐                                        │
│  │ L2 Cache│  ← SRAM, 256KB-1MB, ~10 cycles        │
│  └────┬────┘                                        │
│       │                                             │
│  ┌────▼────┐                                        │
│  │ L3 Cache│  ← SRAM, 8-64MB, ~40 cycles           │
│  └────┬────┘                                        │
└───────┼─────────────────────────────────────────────┘
        │
   ┌────▼────┐
   │Main RAM │  ← DRAM (DDR4/DDR5), 8-128GB, ~100 cycles
   └────┬────┘
        │
   ┌────▼────┐
   │  SSD    │  ← Flash, TB scale, ~10,000 cycles
   └─────────┘
```

---

## STM32H743 Memory Map

The STM32H743 has multiple SRAM regions with different characteristics:

```
┌─────────────────────────────────────────────────────────────┐
│                    STM32H743 Memory                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  0x00000000 ┌─────────────────┐                            │
│             │   Flash (2MB)    │ ← Code, const data        │
│  0x08200000 └─────────────────┘                            │
│                                                             │
│  0x20000000 ┌─────────────────┐                            │
│             │   DTCM (128KB)   │ ← D1 domain, fastest      │
│  0x20020000 └─────────────────┘   CPU-only, no DMA!        │
│                                                             │
│  0x24000000 ┌─────────────────┐                            │
│             │  AXI SRAM (512KB)│ ← D1 domain, fast         │
│  0x24080000 └─────────────────┘   CPU + MDMA only          │
│                                                             │
│             ~~~~ 192MB gap (peripherals, reserved) ~~~~    │
│                                                             │
│  0x30000000 ┌─────────────────┐                            │
│             │   SRAM1 (128KB)  │ ← D2 domain               │
│  0x30020000 ├─────────────────┤   DMA accessible!          │
│             │   SRAM2 (128KB)  │   Contiguous with SRAM1   │
│  0x30040000 ├─────────────────┤                            │
│             │   SRAM3 (32KB)   │   Contiguous with SRAM2   │
│  0x30048000 └─────────────────┘                            │
│                                                             │
│  0x38000000 ┌─────────────────┐                            │
│             │   SRAM4 (64KB)   │ ← D3 domain, low-power    │
│  0x38010000 └─────────────────┘                            │
│                                                             │
│  0x38800000 ┌─────────────────┐                            │
│             │  Backup SRAM(4KB)│ ← Battery-backed          │
│  0x38801000 └─────────────────┘                            │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Memory Regions Summary

| Region | Size | Address | DMA | Best For |
|--------|------|---------|-----|----------|
| DTCM | 128KB | 0x20000000 | No | Stack, critical variables |
| AXI SRAM | 512KB | 0x24000000 | MDMA only | Large buffers, framebuffer |
| SRAM1 | 128KB | 0x30000000 | Yes | DMA buffers |
| SRAM2 | 128KB | 0x30020000 | Yes | DMA buffers |
| SRAM3 | 32KB | 0x30040000 | Yes | DMA buffers |
| SRAM4 | 64KB | 0x38000000 | BDMA | Low-power peripherals |
| Backup | 4KB | 0x38800000 | No | Save data (battery-backed) |

**Total SRAM: ~1MB**

---

## Power Domains

STM32H7 divides memory into power domains for efficiency:

```
┌─────────────────────────────────────────────────────────┐
│  D1 Domain (High Performance)                           │
│  ┌─────────┐  ┌─────────────┐  ┌───────────────────┐   │
│  │   CPU   │  │    DTCM     │  │     AXI SRAM      │   │
│  │(480MHz) │  │  (128KB)    │  │     (512KB)       │   │
│  └─────────┘  └─────────────┘  └───────────────────┘   │
└─────────────────────────────────────────────────────────┘
                         │
┌─────────────────────────────────────────────────────────┐
│  D2 Domain (Peripherals)                                │
│  ┌─────────────────────────────────────────────────┐   │
│  │         SRAM1 + SRAM2 + SRAM3 (288KB)           │   │
│  └─────────────────────────────────────────────────┘   │
│  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐              │
│  │  SPI  │ │  I2C  │ │ UART  │ │  DMA  │              │
│  └───────┘ └───────┘ └───────┘ └───────┘              │
└─────────────────────────────────────────────────────────┘
                         │
┌─────────────────────────────────────────────────────────┐
│  D3 Domain (Low Power) - Can stay on in sleep modes    │
│  ┌─────────────┐  ┌─────────────┐                      │
│  │ SRAM4 (64KB)│  │    BDMA     │                      │
│  └─────────────┘  └─────────────┘                      │
└─────────────────────────────────────────────────────────┘
```

---

## DMA Accessibility

**Critical:** Not all DMA controllers can access all memory!

```
                    DTCM    AXI     SRAM1/2/3   SRAM4
                   (D1)    (D1)      (D2)       (D3)
┌─────────────────┬───────┬───────┬───────────┬───────┐
│ CPU             │   ✓   │   ✓   │     ✓     │   ✓   │
│ DMA1/DMA2       │   ✗   │   ✗   │     ✓     │   ✗   │
│ MDMA            │   ✓   │   ✓   │     ✓     │   ✓   │
│ BDMA            │   ✗   │   ✗   │     ✗     │   ✓   │
└─────────────────┴───────┴───────┴───────────┴───────┘
```

### Placing Buffers for DMA

```c
// WRONG - Default placement may go to DTCM (no DMA access)
uint16_t framebuffer[128 * 160];

// CORRECT - Explicitly place in D2 SRAM for DMA1/DMA2
__attribute__((section(".sram1")))
uint16_t framebuffer[128 * 160];

// CORRECT - AXI SRAM works with MDMA
__attribute__((section(".axisram")))
uint16_t framebuffer[128 * 160];
```

---

## Linker Script Configuration

To use different memory regions, configure your linker script:

```ld
MEMORY
{
    FLASH    (rx)  : ORIGIN = 0x08000000, LENGTH = 2M
    DTCM     (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    AXISRAM  (rwx) : ORIGIN = 0x24000000, LENGTH = 512K
    SRAM1    (rwx) : ORIGIN = 0x30000000, LENGTH = 128K
    SRAM2    (rwx) : ORIGIN = 0x30020000, LENGTH = 128K
    SRAM3    (rwx) : ORIGIN = 0x30040000, LENGTH = 32K
    SRAM4    (rwx) : ORIGIN = 0x38000000, LENGTH = 64K
}

SECTIONS
{
    /* Stack and heap in DTCM (fastest) */
    .stack (NOLOAD) : { . = . + 0x4000; } > DTCM

    /* DMA buffers in SRAM1 */
    .sram1 (NOLOAD) : {
        *(.sram1)
    } > SRAM1

    /* Large buffers in AXI SRAM */
    .axisram (NOLOAD) : {
        *(.axisram)
    } > AXISRAM
}
```

---

## Practical Memory Layout for PocketGateEngine

```c
// Stack & critical variables → DTCM (128KB, fastest, no DMA)
// Default .bss and .data

// Framebuffers → AXI SRAM (512KB, MDMA accessible)
__attribute__((section(".axisram")))
uint16_t framebuffer_a[160 * 128];  // 40KB

__attribute__((section(".axisram")))
uint16_t framebuffer_b[160 * 128];  // 40KB

// SPI DMA buffers → SRAM1 (DMA1/DMA2 accessible)
__attribute__((section(".sram1")))
uint8_t spi_tx_buffer[4096];

__attribute__((section(".sram1")))
uint8_t spi_rx_buffer[4096];

// Audio buffer → SRAM2 (DMA accessible, separate from SPI)
__attribute__((section(".sram2")))
uint16_t audio_buffer[2048];
```

---

## Raspberry Pi Pico (RP2040) Comparison

Much simpler - single SRAM region:

```
┌─────────────────────────────────────────┐
│            RP2040 Memory                │
├─────────────────────────────────────────┤
│  0x00000000  Flash (external, 2-16MB)   │
│  0x20000000  SRAM (264KB total)         │
│              ├── Bank 0: 64KB           │
│              ├── Bank 1: 64KB           │
│              ├── Bank 2: 64KB           │
│              ├── Bank 3: 64KB           │
│              └── Bank 4: 4KB (striped)  │
└─────────────────────────────────────────┘

All SRAM accessible by CPU and DMA - no domain restrictions!
```

---

## Quick Reference

| Question | Answer |
|----------|--------|
| What is SRAM? | Static RAM - fast, no refresh, used in MCUs |
| What is DRAM? | Dynamic RAM - needs refresh, used in PCs |
| Why MCUs use SRAM? | On-chip, fast, deterministic, no refresh |
| Why PCs use DRAM? | Cheap, high density, can have GBs |
| Where to put framebuffer? | AXI SRAM or SRAM1/2 (not DTCM) |
| Where to put DMA buffers? | SRAM1/2/3 for DMA1/DMA2 |
| Where to put stack? | DTCM (fastest) |

---

## Common Mistakes

### 1. Assuming All RAM is Equal

```c
// WRONG - may not work with DMA
uint8_t dma_buffer[1024];  // Linker puts in DTCM

// CORRECT - explicitly placed
__attribute__((section(".sram1")))
uint8_t dma_buffer[1024];
```

### 2. Ignoring Memory Boundaries

```c
// WRONG - 300KB buffer won't fit in any single region except AXI
uint8_t huge_buffer[300 * 1024];  // Linker error or corruption

// CORRECT - use AXI SRAM (512KB)
__attribute__((section(".axisram")))
uint8_t huge_buffer[300 * 1024];
```

### 3. Forgetting Cache Coherency (Cortex-M7)

```c
// DMA writes to buffer, CPU reads stale cached data
__attribute__((section(".sram1")))
uint8_t rx_buffer[1024];

// After DMA completes:
SCB_InvalidateDCache_by_Addr(rx_buffer, sizeof(rx_buffer));
// Now CPU sees fresh data
```

---

## See Also

- `Docs/hardware/dma.md` - DMA and memory transfers
- `Docs/hardware/qspi.md` - External flash storage
