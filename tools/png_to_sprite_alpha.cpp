  // tools/png_to_sprite_alpha.cpp - RGB565 + Alpha8 format
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
  #include <fstream>
  #include <iostream>
  #include <cstdint>
  #include <cstring>

  // RGB888 to RGB565 conversion
  uint16_t rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
      return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
  }

  // Swap endianness for binary file (little-endian)
  uint16_t to_little_endian(uint16_t value) {
      return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
  }

  bool convert_png_to_sprite(const char* input_png, const char* output_sprite) {
      int width, height, channels;

      // Load PNG with RGBA
      unsigned char* img = stbi_load(input_png, &width, &height, &channels, 4);
      if (!img) {
          std::cerr << "Error: Failed to load " << input_png << std::endl;
          return false;
      }

      std::cout << "Converting (RGB565+Alpha8): " << input_png << std::endl;
      std::cout << "  Dimensions: " << width << "x" << height << std::endl;
      std::cout << "  Pixels: " << (width * height) << std::endl;
      std::cout << "  Size: " << (4 + width * height * 3) << " bytes (3 bytes/pixel)" << std::endl;

      // Open output file (binary mode)
      std::ofstream out(output_sprite, std::ios::binary);
      if (!out) {
          std::cerr << "Error: Cannot create " << output_sprite << std::endl;
          stbi_image_free(img);
          return false;
      }

      // Write header (width, height as uint16_t little-endian)
      uint16_t w = static_cast<uint16_t>(width);
      uint16_t h = static_cast<uint16_t>(height);
      out.write(reinterpret_cast<const char*>(&w), sizeof(uint16_t));
      out.write(reinterpret_cast<const char*>(&h), sizeof(uint16_t));

      // Convert and write pixel data (RGB565 + Alpha8 = 3 bytes per pixel)
      int transparent_count = 0;
      for (int y = 0; y < height; y++) {
          for (int x = 0; x < width; x++) {
              int index = (y * width + x) * 4;  // RGBA = 4 bytes/pixel
              uint8_t r = img[index + 0];
              uint8_t g = img[index + 1];
              uint8_t b = img[index + 2];
              uint8_t a = img[index + 3];

              uint16_t rgb565 = rgb_to_rgb565(r, g, b);

              if (a < 10) {
                  transparent_count++;
              }

              // Write RGB565 (2 bytes) + alpha (1 byte) = 3 bytes total
              out.write(reinterpret_cast<const char*>(&rgb565), sizeof(uint16_t));
              out.write(reinterpret_cast<const char*>(&a), sizeof(uint8_t));
          }
      }

      out.close();
      stbi_image_free(img);

      std::cout << "  Transparent pixels: " << transparent_count << std::endl;
      std::cout << "Success: Generated " << output_sprite << std::endl;
      return true;
  }

  int main(int argc, char** argv) {
      if (argc < 2) {
          std::cout << "PNG to Binary Sprite Converter (Alpha Blending) for TriggEngine\n";
          std::cout << "Usage:\n";
          std::cout << "  " << argv[0] << " <input.png> [output.sprite]\n";
          std::cout << "\nOutput format:\n";
          std::cout << "  - Width: uint16_t (2 bytes)\n";
          std::cout << "  - Height: uint16_t (2 bytes)\n";
          std::cout << "  - Pixels: RGB565 (2 bytes) + Alpha8 (1 byte) = 3 bytes per pixel\n";
          std::cout << "  - Supports full alpha blending (0-255)\n";
          return 1;
      }

      std::string input = argv[1];
      std::string output;

      if (argc >= 3) {
          output = argv[2];
      } else {
          // Auto-generate output filename
          size_t dot = input.find_last_of('.');
          if (dot != std::string::npos) {
              output = input.substr(0, dot) + ".sprite";
          } else {
              output = input + ".sprite";
          }
      }

      if (!convert_png_to_sprite(input.c_str(), output.c_str())) {
          return 1;
      }

      return 0;
  }
