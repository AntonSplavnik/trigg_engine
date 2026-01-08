#!/usr/bin/env python3
"""
Converts binary sprite files to C++ header files with embedded data.
Usage: python sprite_to_cpp.py input.sprite output.h width height
  python3 tools/sprite_to_cpp.py assets/skeleton.sprite assets/skeleton.h 59 43 skeleton
"""

import sys
import struct

def convert_sprite_to_cpp(input_file, output_file, width, height, sprite_name):
    """Convert binary sprite file to C++ header with const array."""

    # Read binary sprite data
    with open(input_file, 'rb') as f:
        data = f.read()
        data = data[4:]

    # Calculate expected size
    expected_size = width * height * 2  # 2 bytes per pixel (RGB565)
    actual_size = len(data)

    if actual_size != expected_size:
        print(f"Warning: Expected {expected_size} bytes, got {actual_size} bytes")

    # Convert to uint16_t values
    num_pixels = actual_size // 2
    pixels = struct.unpack(f'>{num_pixels}H', data[:num_pixels * 2])

    # Generate C++ header file
    with open(output_file, 'w') as f:
        f.write(f"// Auto-generated from {input_file}\n")
        f.write(f"// Sprite dimensions: {width}x{height}\n\n")
        f.write(f"#ifndef SPRITE_{sprite_name.upper()}_H\n")
        f.write(f"#define SPRITE_{sprite_name.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n\n")

        # Write sprite dimensions as constants
        f.write(f"const uint16_t {sprite_name}_width = {width};\n")
        f.write(f"const uint16_t {sprite_name}_height = {height};\n\n")

        # Write pixel data array
        f.write(f"const uint16_t {sprite_name}_data[{num_pixels}] = {{\n")

        # Write 8 values per line for readability
        for i in range(0, len(pixels), 8):
            line_pixels = pixels[i:i+8]
            hex_values = ', '.join(f'0x{p:04X}' for p in line_pixels)
            f.write(f"    {hex_values},\n")

        f.write("};\n\n")
        f.write(f"#endif // SPRITE_{sprite_name.upper()}_H\n")

    print(f"✓ Converted {input_file} → {output_file}")
    print(f"  Dimensions: {width}x{height}")
    print(f"  Pixels: {num_pixels}")
    print(f"  Size: {actual_size} bytes")

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: python sprite_to_cpp.py <input.sprite> <output.h> <width> <height> <sprite_name>")
        print("Example: python sprite_to_cpp.py skeleton.sprite skeleton.h 59 43 skeleton")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    width = int(sys.argv[3])
    height = int(sys.argv[4])
    sprite_name = sys.argv[5]

    convert_sprite_to_cpp(input_file, output_file, width, height, sprite_name)
