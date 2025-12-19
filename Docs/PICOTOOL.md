# Picotool Reference

**Picotool** is the official Raspberry Pi tool for interacting with RP2040/RP2350 devices and binaries.

## Installation

```bash
brew install picotool
```

**Verify installation:**
```bash
picotool version
```

---

## Core Commands

### info - Device/Binary Information

Display information about connected Pico or a binary file.

```bash
picotool info                    # Basic info about connected device
picotool info -a                 # All info (detailed)
picotool info myprogram.uf2      # Info about a binary file
```

**Output includes:**
- Program name
- Binary size and memory usage
- Features used (USB, SPI, etc.)
- Build date and SDK version
- Pin assignments

**Use cases:**
- Check what's currently flashed on the Pico
- Verify build information
- Debug memory usage issues

---

### load - Flash Programs

Upload a program to the Pico.

```bash
picotool load myprogram.uf2              # Flash to device in BOOTSEL mode
picotool load -x myprogram.uf2           # Flash and execute immediately
picotool load -x -f myprogram.uf2        # Force flash (auto-reboot if running)
picotool load -t elf myprogram.elf       # Flash ELF file directly
```

**Flags:**
- `-x` - Execute after loading
- `-f` - Force operation (reboot device if needed)
- `-t <type>` - Specify file type (elf, bin, uf2)

**Common workflow for TriggEngine:**
```bash
cd build
make
picotool load -x -f TriggEngine.uf2
```

---

### save - Backup Flash Memory

Save the contents of Pico's flash memory to a file.

```bash
picotool save -a backup.uf2                      # Save entire flash
picotool save -r 0x10000000 0x10010000 prog.bin  # Save specific range
picotool save program.uf2                        # Save program only
```

**Flags:**
- `-a` - Save all flash memory
- `-r <start> <end>` - Save specific address range

**Use cases:**
- Backup working firmware before experimenting
- Extract program from a Pico
- Debug flash contents

**Example: Backup before testing:**
```bash
# Save current working version
picotool save -a working_backup.uf2

# Flash experimental code
picotool load -x experimental.uf2

# Restore if it breaks
picotool load -x working_backup.uf2
```

---

### verify - Check Flash Integrity

Verify that flash contents match a file.

```bash
picotool verify myprogram.uf2    # Check if flash matches file
```

**Use cases:**
- Confirm successful flash
- Debug flashing issues
- Verify no corruption

---

### reboot - Control Device State

Reboot the Pico in various modes.

```bash
picotool reboot                  # Normal reboot (run program)
picotool reboot -u               # Reboot to BOOTSEL mode
picotool reboot -a 0x10000000    # Jump to specific address
picotool reboot -f               # Force reboot (if device running)
```

**Flags:**
- `-u` - Reboot to USB BOOTSEL mode
- `-a <address>` - Boot to specific address
- `-f` - Force operation

**Remote flashing workflow:**
```bash
# Reboot to BOOTSEL from running program
picotool reboot -u -f

# Flash new firmware
picotool load -x new_version.uf2
```

---

### uf2 convert - Binary Conversion

Convert between binary formats.

```bash
picotool uf2 convert input.elf output.uf2        # ELF → UF2
picotool uf2 convert input.bin output.uf2        # BIN → UF2
picotool uf2 convert input.uf2 output.bin        # UF2 → BIN
```

**Supported formats:**
- ELF (Executable and Linkable Format)
- BIN (Raw binary)
- UF2 (USB Flashing Format)

**Use cases:**
- Convert build output to flashable format
- Extract binary from UF2 for analysis
- Prepare files for bootloaders

---

### version - Display Picotool Version

```bash
picotool version
```

Shows picotool version and SDK compatibility.

---

## Advanced Commands

### otp - One-Time Programmable Memory

**WARNING: OTP writes are PERMANENT and IRREVERSIBLE!**

```bash
picotool otp list                # Show available OTP registers
picotool otp get <reg>           # Read OTP register value
picotool otp set <reg> <value>   # Write OTP register (PERMANENT!)
picotool otp dump                # Dump all OTP contents
picotool otp dump -o otp.bin     # Dump to binary file
```

**Use cases:**
- Store device-specific configuration
- Set boot options
- Program security keys
- Lock device settings

**IMPORTANT:** Only use OTP commands if you fully understand the consequences. Mistakes cannot be undone.

---

### encrypt - Binary Encryption (RP2350 only)

Create encrypted binaries for secure boot.

```bash
picotool encrypt input.elf -o encrypted.uf2 --key aes_key.bin --iv salt.bin
```

**Requirements:**
- RP2350 device (not RP2040)
- AES encryption key file
- IV salt file
- Properly configured OTP security settings

**Use cases:**
- Protect intellectual property
- Prevent firmware extraction
- Secure boot implementation

---

## Common Workflows

### Fast Development Cycle (No Unplugging)

```bash
# One-command build and flash
cd build && make && picotool load -x -f TriggEngine.uf2

# Or create an alias in ~/.zshrc:
alias flash='cd build && make && picotool load -x -f TriggEngine.uf2 && cd ..'
```

### Debug Flash Issues

```bash
# Check what's on the device
picotool info -a

# Verify flash succeeded
picotool verify TriggEngine.uf2

# If verification fails, try reflashing
picotool load -x -f TriggEngine.uf2
```

### Safe Experimentation

```bash
# 1. Backup current firmware
picotool save -a backup.uf2

# 2. Try experimental code
picotool load -x experimental.uf2

# 3. If it breaks, restore backup
picotool load -x backup.uf2
```

### Remote Firmware Update

```bash
# Device is running a program with USB stdio enabled
picotool reboot -u -f           # Force reboot to BOOTSEL
picotool load -x firmware.uf2   # Flash new version
```

---

## Device Modes

### BOOTSEL Mode

**How to enter manually:**
1. Unplug Pico
2. Hold BOOTSEL button
3. Plug in Pico
4. Release button
5. Pico appears as USB drive

**How to enter with picotool:**
```bash
picotool reboot -u -f
```

**In BOOTSEL mode you can:**
- Load programs
- Save flash contents
- Get device info
- Verify flash
- Access OTP

### Running Mode (with USB stdio)

**Requirements:**
- Program compiled with USB stdio enabled
- `pico_enable_stdio_usb(program 1)` in CMakeLists.txt

**Available commands with `-f` flag:**
- `load -f` - Flash without manual BOOTSEL
- `reboot -f` - Remote reboot
- `info -f` - Get info from running device

---

## Flags Reference

### Common Flags

| Flag | Description | Used With |
|------|-------------|-----------|
| `-f` | Force operation (auto-reboot) | load, reboot, info |
| `-x` | Execute after loading | load |
| `-a` | All / Full operation | info, save |
| `-u` | USB BOOTSEL mode | reboot |
| `-t <type>` | Specify file type | load |
| `-r <start> <end>` | Address range | save |
| `-o <file>` | Output file | encrypt, otp dump |

---

## Troubleshooting

### "No accessible RP2040 devices in BOOTSEL mode were found"

**Solutions:**
1. Check USB cable (must support data, not just power)
2. Enter BOOTSEL mode manually (hold button while plugging in)
3. Try different USB port
4. Check USB permissions (macOS usually doesn't need sudo)

### "Device is busy" or "Unable to access device"

**Solutions:**
1. Use `-f` flag to force operation
2. Close serial monitor (minicom, screen, etc.)
3. Disconnect other programs using the device
4. Try unplugging and manually entering BOOTSEL

### Flash verification fails

**Solutions:**
1. Reflash the binary
2. Try slower flash: disconnect and use BOOTSEL mode
3. Check binary file integrity
4. Try different USB cable/port

---

## TriggEngine Integration

### CMakeLists.txt Configuration

Ensure USB stdio is enabled for `-f` flag support:

```cmake
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
```

### Build Script Example

Create `flash.sh` in project root:

```bash
#!/bin/bash
cd build
cmake ..
make -j4
if [ $? -eq 0 ]; then
    echo "Build successful, flashing..."
    picotool load -x -f TriggEngine.uf2
else
    echo "Build failed!"
    exit 1
fi
```

Make executable:
```bash
chmod +x flash.sh
```

Usage:
```bash
./flash.sh
```

---

## Related Tools

- **minicom** - Serial monitor for USB stdio output
- **OpenOCD** - Debugger (requires debug probe)
- **arm-none-eabi-gdb** - GDB debugger for ARM
- **cmake** - Build system

---

## Resources

- [Official GitHub Repository](https://github.com/raspberrypi/picotool)
- [Raspberry Pi Pico Documentation](https://www.raspberrypi.com/documentation/microcontrollers/)
- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)

---

**Last Updated:** 2025-12-19
