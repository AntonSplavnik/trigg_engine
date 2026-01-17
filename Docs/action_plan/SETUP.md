# 1. Write code (on Mac)
vim main.cpp

# 2. Build with SDK (on Mac)
mkdir build && cd build
cmake ..        # SDK's CMake finds toolchain
make            # Compiles to ARM, creates .uf2

# 3a. Flash manually (no picotool needed)
# Hold BOOTSEL, plug Pico, drag .uf2 to RPI-RP2 drive

# 3b. Flash with picotool (faster)
picotool load myprogram.uf2 -f
```

### What Happens Behind the Scenes

**When you run `cmake ..`**:
```
Your CMakeLists.txt 
    ↓
PICO_SDK_PATH points to SDK
    ↓
SDK provides: pico_stdlib, hardware_spi, etc.
    ↓
ARM cross-compiler (arm-none-eabi-gcc) builds for RP2040
    ↓
Creates: myprogram.elf → myprogram.uf2
