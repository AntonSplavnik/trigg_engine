#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <map>

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    
    int w, h, ch;
    unsigned char* img = stbi_load(argv[1], &w, &h, &ch, 4);
    if (!img) return 1;
    
    std::map<int, int> alpha_histogram;
    
    for (int i = 0; i < w * h; i++) {
        uint8_t a = img[i * 4 + 3];
        alpha_histogram[a]++;
    }
    
    std::cout << "Alpha channel histogram for " << argv[1] << ":\n";
    for (auto& p : alpha_histogram) {
        std::cout << "  Alpha " << p.first << ": " << p.second << " pixels";
        if (p.first < 128) std::cout << " (TRANSPARENT)";
        else std::cout << " (OPAQUE)";
        std::cout << "\n";
    }
    
    stbi_image_free(img);
    return 0;
}
