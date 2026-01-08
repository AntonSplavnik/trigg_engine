// PNG to Sprite Converter with Verbose Output
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

uint16_t rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

bool convert_png_to_sprite(const char* input_png, const char* output_sprite, bool verbose) {
    int width, height, channels;

    // Load PNG with RGBA (force 4 channels)
    unsigned char* img = stbi_load(input_png, &width, &height, &channels, 4);
    if (!img) {
        std::cerr << "Error: Failed to load " << input_png << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "=== PNG to Sprite Converter ===\n";
    std::cout << "Input: " << input_png << "\n";
    std::cout << "  Original channels: " << channels << " (";
    switch(channels) {
        case 1: std::cout << "Grayscale"; break;
        case 2: std::cout << "Grayscale + Alpha"; break;
        case 3: std::cout << "RGB"; break;
        case 4: std::cout << "RGBA"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << ")\n";
    std::cout << "  Loaded as: 4 channels (RGBA forced)\n";
    std::cout << "  Dimensions: " << width << "x" << height << "\n";
    std::cout << "  Total pixels: " << (width * height) << "\n";

    if (channels < 4) {
        std::cout << "\n⚠️  WARNING: Original PNG has no alpha channel!\n";
        std::cout << "  Transparency detection may not work as expected.\n";
        std::cout << "  stb_image will fill alpha=255 (opaque) for missing alpha.\n\n";
    }

    // Open output file
    std::ofstream out(output_sprite, std::ios::binary);
    if (!out) {
        std::cerr << "Error: Cannot create " << output_sprite << std::endl;
        stbi_image_free(img);
        return false;
    }

    // Write header
    uint16_t w = static_cast<uint16_t>(width);
    uint16_t h = static_cast<uint16_t>(height);
    out.write(reinterpret_cast<const char*>(&w), sizeof(uint16_t));
    out.write(reinterpret_cast<const char*>(&h), sizeof(uint16_t));

    // Show first few pixels if verbose
    if (verbose) {
        std::cout << "\n=== First 10 Pixels Analysis ===\n";
        for (int i = 0; i < 10 && i < width * height; i++) {
            int index = i * 4;
            uint8_t r = img[index + 0];
            uint8_t g = img[index + 1];
            uint8_t b = img[index + 2];
            uint8_t a = img[index + 3];

            int x = i % width;
            int y = i / width;

            std::cout << "Pixel [" << x << "," << y << "]: ";
            std::cout << "RGBA(" << (int)r << "," << (int)g << "," << (int)b << "," << (int)a << ") -> ";

            if (a < 10) {
                std::cout << "0xF81F (TRANSPARENT)\n";
            } else {
                uint16_t rgb565 = rgb_to_rgb565(r, g, b);
                std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0')
                          << rgb565 << std::dec << " (OPAQUE)\n";
            }
        }
        std::cout << "\n";
    }

    // Convert all pixels
    int transparent_count = 0;
    int opaque_count = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            uint8_t r = img[index + 0];
            uint8_t g = img[index + 1];
            uint8_t b = img[index + 2];
            uint8_t a = img[index + 3];

            uint16_t rgb565;
            if (a < 10) {
                rgb565 = 0xF81F;  // Magenta transparency
                transparent_count++;
            } else {
                rgb565 = rgb_to_rgb565(r, g, b);
                opaque_count++;
            }

            out.write(reinterpret_cast<const char*>(&rgb565), sizeof(uint16_t));
        }
    }

    out.close();
    stbi_image_free(img);

    std::cout << "=== Conversion Summary ===\n";
    std::cout << "Transparent pixels: " << transparent_count
              << " (" << (transparent_count * 100 / (width * height)) << "%)\n";
    std::cout << "Opaque pixels: " << opaque_count
              << " (" << (opaque_count * 100 / (width * height)) << "%)\n";
    std::cout << "Output file: " << output_sprite << "\n";
    std::cout << "File size: " << (4 + width * height * 2) << " bytes\n";

    if (transparent_count == 0 && channels == 4) {
        std::cout << "\n⚠️  No transparent pixels found, but PNG has alpha channel.\n";
        std::cout << "  Check if your alpha values are all >= 128.\n";
    }

    std::cout << "\n✓ Conversion complete!\n";
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "PNG to Binary Sprite Converter (Verbose)\n";
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " <input.png> [output.sprite] [-v]\n";
        std::cout << "\nOptions:\n";
        std::cout << "  -v    Verbose mode (show first 10 pixels)\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output;
    bool verbose = false;

    // Check for verbose flag
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v") {
            verbose = true;
        }
    }

    // Determine output filename
    if (argc >= 3 && std::string(argv[2]) != "-v") {
        output = argv[2];
    } else {
        size_t dot = input.find_last_of('.');
        if (dot != std::string::npos) {
            output = input.substr(0, dot) + ".sprite";
        } else {
            output = input + ".sprite";
        }
    }

    if (!convert_png_to_sprite(input.c_str(), output.c_str(), verbose)) {
        return 1;
    }

    return 0;
}
