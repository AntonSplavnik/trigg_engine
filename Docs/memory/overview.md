# Memory & Storage Overview

Quick comparison of the three storage types available on the Raspberry Pi Pico.

---

## Quick Comparison Table

| Feature                          | Flash (2MB)      | RAM (264KB)      | SD Card (GB)        |
|----------------------------------|------------------|------------------|---------------------|
| **Keeps data when powered off?** | ✅ Yes           | ❌ No            | ✅ Yes              |
| **Read speed**                   | 1-3 µs/byte      | <1 ns/byte       | 50-100 µs/byte      |
| **Write speed**                  | Very slow        | Instant          | 100-200 µs/byte     |
| **Seek/access time**             | Instant (direct) | Instant          | 10-50ms             |
| **Access method**                | Direct (XIP)     | Direct           | SPI + FatFS         |
| **Write cycles**                 | ~100,000         | Unlimited        | ~10,000-100,000     |
| **User-modifiable?**             | ❌ No (reflash)  | ✅ Yes (runtime) | ✅ Yes (swap card)  |
| **Best for**                     | Program + assets | Active game data | Music, user content |

---

## Speed Comparison (Reading 1KB Sprite)

- **RAM:** ~1 microsecond (fastest)
- **Flash:** ~3 microseconds (3× slower than RAM)
- **SD Card:** ~150 microseconds (50× slower than Flash, 150× slower than RAM)

**Conclusion:** Flash is fast enough for 60 FPS game rendering. SD card is NOT.

---

## Storage Locations

```
┌─────────────────────────────────┐
│       FLASH (2MB)               │  External chip
│  - Your compiled program        │  Non-volatile
│  - const sprite/tile arrays     │  Fast reads
│  - Level data                   │  Slow writes
└─────────────────────────────────┘
           ↓ CPU reads directly (XIP)

┌─────────────────────────────────┐
│       RAM (264KB)               │  Inside RP2040
│  - Framebuffer (40KB)           │  Volatile
│  - Active game state            │  Instant read/write
│  - Variables, stack, heap       │  Limited size
└─────────────────────────────────┘
           ↓ CPU reads/writes directly

┌─────────────────────────────────┐
│       SD CARD (GB)              │  Removable card
│  - Music files (3-5MB each)     │  Non-volatile
│  - User-generated levels        │  Slow access
│  - Optional content             │  Massive storage
└─────────────────────────────────┘
           ↓ Access via SPI + FatFS
```

---

## When to Use Each

### Flash Memory
✅ Sprites and tiles (accessed every frame)
✅ Level data (100 levels = 1.1MB fits easily)
✅ Engine code
✅ Save games (using hardware_flash library)

**See:** [Flash Memory Details](flash.md)

---

### RAM
✅ Framebuffer (40KB, mandatory)
✅ Active game variables (player position, score)
✅ Current level tilemap (if modifying)
✅ Temporary buffers

**See:** [RAM Details](ram.md)

---

### SD Card (Optional)
✅ Background music (3-5MB per track)
✅ User-generated content
✅ Development workflow (no recompilation)
❌ NOT for sprites/tiles (too slow!)

**See:** [SD Card Details](sd_card.md)

---

## Recommended Budget

### Phase 1-3: Flash Only (No SD Card)
```
Flash (2MB):
├─ Engine code:       100KB
├─ Sprites/tiles:     70KB
├─ 100 levels:        1.1MB
└─ Available:         ~700KB

RAM (264KB):
├─ Framebuffer:       40KB
├─ Active data:       15KB
├─ Stack/heap:        20KB
└─ Available:         ~189KB
```

### Phase 4+: Add SD Card for Audio
```
Flash (2MB):
├─ Engine + assets    (same as above)

SD Card (GB):
├─ Music:             5 tracks × 4MB = 20MB
├─ User levels:       Variable
└─ Backups:           Optional
```

---

## Learn More

- **[Flash Memory (2MB)](flash.md)** - Detailed guide to Flash storage
- **[RAM (264KB)](ram.md)** - RAM usage and optimization
- **[SD Card Storage](sd_card.md)** - When and how to use SD cards
- **[Level Loading Strategies](level_loading.md)** - BSS vs Heap allocation for loading game levels from SD
