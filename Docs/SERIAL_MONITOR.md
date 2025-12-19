# Serial Monitor - macOS with screen

## Overview
Monitor Raspberry Pi Pico's `printf()` output over USB serial using the built-in `screen` command.

## Prerequisites
- USB serial must be enabled in CMakeLists.txt:
  ```cmake
  pico_enable_stdio_usb(${PROJECT_NAME} 1)
  pico_enable_stdio_uart(${PROJECT_NAME} 0)
  ```
- Your code must call `stdio_init_all()` before any `printf()`

## Find Serial Port

List available USB serial devices:
```bash
ls /dev/tty.usbmodem*
```

Example output:
```
/dev/tty.usbmodem14201
```

## Connect with screen

**Method 1: Using wildcard (simplest)**
```bash
screen /dev/tty.usbmodem* 115200
```

**Method 2: Using exact port**
```bash
screen /dev/tty.usbmodem14201 115200
```

Parameters:
- `/dev/tty.usbmodem*` - Serial port path
- `115200` - Baud rate (default for Pico)

## Exit screen

### Method 1: Unplug Pico (Easiest & Most Reliable)
Simply unplug the USB cable - screen will exit automatically

### Method 2: Keyboard Shortcut (may not always work)
1. Press `Ctrl+A` (release)
2. Press `K` (lowercase k)
3. Press `Y` to confirm

**Note:** If this doesn't work, use Method 1 (unplug)

## Find and Kill Background Screen Sessions

If you closed the terminal window or screen is stuck, it may still be running in the background and locking the serial port.

### List all running screen sessions:
```bash
screen -ls
```

Example output:
```
There is a screen on:
    12345.ttys001.hostname    (Detached)
1 Socket in /var/folders/...
```

### Kill specific session by ID:
```bash
screen -X -S 12345 quit
```
Replace `12345` with the actual session ID from `screen -ls`

### Kill all screen sessions at once:
```bash
killall screen
```

### Verify they're gone:
```bash
screen -ls
```

Should show: `No Sockets found in /var/folders/...`

### Before reconnecting, always check:
```bash
screen -ls              # Check for existing sessions
killall screen          # Kill them if any exist
screen /dev/tty.usbmodem* 115200  # Now connect
```

## Important Workflow

**To see startup messages, you have two options:**

### Option A: Quick Connect (Race Against Boot)
1. **Plug in Pico**
2. **Immediately run screen** (within ~1 second):
   ```bash
   screen /dev/tty.usbmodem* 115200
   ```
3. If fast enough, you'll catch the boot messages

**Problem:** Hard to time correctly, often miss the first `printf()`

### Option B: Add Startup Delay (Recommended)
1. **Add longer delay in your code** (gives you time to connect):
   ```cpp
   int main(){
       stdio_init_all();
       sleep_ms(3000);  // 3 second delay
       printf("TriggEngine v0.1\n");
       // rest of code...
   }
   ```
2. **Rebuild and flash** your program
3. **Plug in Pico**
4. **Run screen within 3 seconds**:
   ```bash
   screen /dev/tty.usbmodem* 115200
   ```
5. You'll see the startup message

### Option C: Continuous Output (Best for Testing)
Add ongoing `printf()` so you can connect anytime:
```cpp
while (true) {
    printf("Status update [%d]\n", counter++);
    sleep_ms(1000);
}
```
Connect whenever convenient - you'll see output immediately.

**Note:** Replugging USB disconnects screen (must restart screen command)

## Normal Behavior

When screen starts, you'll see:
- **Blank terminal** - This is normal! Waiting for output
- Any `printf()` from Pico will appear here
- No prompt or indication until Pico sends data

## Troubleshooting

**"No device found"**
- Check Pico is plugged in: `ls /dev/tty.usbmodem*`
- Try unplugging and replugging
- Check USB cable (data cable, not power-only)

**No output appears**
- Verify `stdio_init_all()` is called in your code
- Add small delay after init: `sleep_ms(1000);`
- Check CMakeLists.txt has `pico_enable_stdio_usb` enabled
- Rebuild and reflash the program

**Screen exits immediately**
- Pico disconnected (unplugged or crashed)
- Port was already in use by another program

**Can't exit screen**
- Press `Ctrl+A` then `K` then `Y`
- If stuck, close terminal window or press `Ctrl+A` then `\` then `Y`

## Tips

**Add continuous output for testing:**
```cpp
int counter = 0;
while (true) {
    printf("Running... [%d]\n", counter++);
    sleep_ms(1000);
}
```

This way you can verify the connection works anytime without timing the connection perfectly.

**Multiple monitoring sessions:**
- Only one `screen` session can connect to a port at a time
- Close existing session before starting a new one
