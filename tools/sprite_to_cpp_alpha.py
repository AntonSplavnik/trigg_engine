#!/usr/bin/env python3
"""
Converts binary sprite files (RGB565+Alpha8) to C++ header files with embedded data.
Usage: python sprite_to_cpp_alpha.py input.sprite output.h width height sprite_name
  python3 tools/sprite_to_cpp_alpha.py assets/skeleton.sprite assets/skeleton.h 59 43 skeleton
"""

import sys
import struct

def convert_sprite_to_cpp(input_file, output_file, width, height, sprite_name):
    """Convert binary sprite file (RGB565+Alpha8) to C++ header with struct array."""

    # Read binary sprite data
    with open(input_file, 'rb') as f:
        data = f.read()
        data = data[4:]  # Skip 4-byte header (width, height)

    # Calculate expected size (3 bytes per pixel: 2 RGB565 + 1 Alpha8)
    expected_size = width * height * 3
    actual_size = len(data)

    if actual_size != expected_size:
        print(f"Warning: Expected {expected_size} bytes, got {actual_size} bytes")

    # Parse pixel data (each pixel = RGB565 uint16_t + Alpha uint8_t)
    num_pixels = width * height
    pixels = []
    for i in range(num_pixels):
        offset = i * 3
        # Read RGB565 (2 bytes, little-endian) and alpha (1 byte)
        rgb565 = struct.unpack('<H', data[offset:offset+2])[0]
        alpha = data[offset+2]
        pixels.append((rgb565, alpha))

    # Generate C++ header file
    with open(output_file, 'w') as f:
        f.write(f"// Auto-generated from {input_file}\n")
        f.write(f"// Sprite dimensions: {width}x{height}\n")
        f.write(f"// Format: RGB565 + Alpha8 (3 bytes per pixel)\n\n")
        f.write(f"#ifndef SPRITE_{sprite_name.upper()}_H\n")
        f.write(f"#define SPRITE_{sprite_name.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n")
        f.write(f'#include "framebuffer.h"  // For SpritePixel definition\n\n')

        # Write sprite dimensions as constants
        f.write(f"const uint16_t {sprite_name}_width = {width};\n")
        f.write(f"const uint16_t {sprite_name}_height = {height};\n\n")

        # Write pixel data array
        f.write(f"const SpritePixel {sprite_name}_data[{num_pixels}] = {{\n")

        # Write 4 pixels per line for readability
        for i in range(0, len(pixels), 4):
            line_pixels = pixels[i:i+4]
            hex_values = ', '.join(f'{{0x{p[0]:04X}, {p[1]:3d}}}' for p in line_pixels)
            f.write(f"    {hex_values},\n")

        f.write("};\n\n")
        f.write(f"#endif // SPRITE_{sprite_name.upper()}_H\n")

    print(f"✓ Converted {input_file} → {output_file}")
    print(f"  Dimensions: {width}x{height}")
    print(f"  Pixels: {num_pixels}")
    print(f"  Size: {actual_size} bytes (3 bytes/pixel: RGB565+Alpha8)")

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: python sprite_to_cpp_alpha.py <input.sprite> <output.h> <width> <height> <sprite_name>")
        print("Example: python sprite_to_cpp_alpha.py skeleton.sprite skeleton_alpha.h 59 43 skeleton")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    width = int(sys.argv[3])
    height = int(sys.argv[4])
    sprite_name = sys.argv[5]

    convert_sprite_to_cpp(input_file, output_file, width, height, sprite_name)
