// Sprite Inspector - Shows what's actually in a .sprite file
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

struct RGB {
    uint8_t r, g, b;
};

RGB rgb565_to_rgb888(uint16_t rgb565) {
    RGB color;
    color.r = ((rgb565 >> 11) & 0x1F) << 3;  // 5-bit -> 8-bit
    color.g = ((rgb565 >> 5) & 0x3F) << 2;   // 6-bit -> 8-bit
    color.b = (rgb565 & 0x1F) << 3;          // 5-bit -> 8-bit
    return color;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <sprite_file.sprite>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open " << argv[1] << std::endl;
        return 1;
    }

    // Read header
    uint16_t width, height;
    file.read(reinterpret_cast<char*>(&width), 2);
    file.read(reinterpret_cast<char*>(&height), 2);

    std::cout << "=== Sprite File Inspector ===\n";
    std::cout << "File: " << argv[1] << "\n";
    std::cout << "Dimensions: " << width << "x" << height << "\n";
    std::cout << "Total pixels: " << (width * height) << "\n\n";

    // Count colors
    int transparent_count = 0;
    int opaque_count = 0;
    int total_pixels = width * height;

    // Sample first 10 transparent and 10 opaque pixels
    std::cout << "=== First 10 Transparent Pixels (0xF81F) ===\n";
    int trans_shown = 0;

    std::cout << "=== First 10 Opaque Pixels ===\n";
    int opaque_shown = 0;

    // Reset file position
    file.seekg(4, std::ios::beg);

    for (int i = 0; i < total_pixels; i++) {
        uint16_t pixel;
        file.read(reinterpret_cast<char*>(&pixel), 2);

        if (pixel == 0xF81F) {
            transparent_count++;
            if (trans_shown < 10) {
                int x = i % width;
                int y = i / width;
                std::cout << "  Pixel [" << x << "," << y << "]: 0x"
                          << std::hex << std::setw(4) << std::setfill('0')
                          << pixel << " (TRANSPARENT)\n" << std::dec;
                trans_shown++;
            }
        } else {
            opaque_count++;
            if (opaque_shown < 10) {
                RGB color = rgb565_to_rgb888(pixel);
                int x = i % width;
                int y = i / width;
                std::cout << "  Pixel [" << x << "," << y << "]: 0x"
                          << std::hex << std::setw(4) << std::setfill('0')
                          << pixel << std::dec
                          << " -> RGB(" << (int)color.r << ","
                          << (int)color.g << "," << (int)color.b << ")\n";
                opaque_shown++;
            }
        }
    }

    file.close();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Transparent pixels (0xF81F): " << transparent_count
              << " (" << (transparent_count * 100 / total_pixels) << "%)\n";
    std::cout << "Opaque pixels: " << opaque_count
              << " (" << (opaque_count * 100 / total_pixels) << "%)\n";

    if (transparent_count == 0) {
        std::cout << "\n⚠️  WARNING: No transparent pixels found!\n";
        std::cout << "Your PNG might not have alpha channel data.\n";
    } else if (transparent_count == total_pixels) {
        std::cout << "\n⚠️  WARNING: All pixels are transparent!\n";
        std::cout << "Your PNG might be completely transparent or empty.\n";
    } else {
        std::cout << "\n✓ Sprite looks valid!\n";
    }

    return 0;
}
