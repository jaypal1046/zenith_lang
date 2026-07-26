#ifndef ZENITH_BLUR2D_H
#define ZENITH_BLUR2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class BlurPass2D {
public:
    static void applyBoxBlur(std::vector<uint32_t>& buffer, int width, int height, int radius = 2) {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0) return;

        std::vector<uint32_t> temp = buffer;

        // Horizontal Pass
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint32_t rSum = 0, gSum = 0, bSum = 0, count = 0;
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = std::clamp(x + dx, 0, width - 1);
                    uint32_t pixel = buffer[y * width + nx];
                    rSum += (pixel >> 16) & 0xFF;
                    gSum += (pixel >> 8) & 0xFF;
                    bSum += pixel & 0xFF;
                    count++;
                }
                uint32_t r = rSum / count;
                uint32_t g = gSum / count;
                uint32_t b = bSum / count;
                temp[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
        buffer = temp;
    }
};

} // namespace zenith

#endif // ZENITH_BLUR2D_H
