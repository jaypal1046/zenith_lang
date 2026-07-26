#ifndef ZENITH_PIXELATE2D_H
#define ZENITH_PIXELATE2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class PixelateFilter2D {
public:
    static void applyPixelate(std::vector<uint32_t>& buffer, int width, int height, int pixelSize = 8) {
        if (buffer.empty() || width <= 0 || height <= 0 || pixelSize <= 1) return;

        for (int y = 0; y < height; y += pixelSize) {
            for (int x = 0; x < width; x += pixelSize) {
                uint32_t blockColor = buffer[y * width + x];

                for (int py = y; py < std::min(y + pixelSize, height); ++py) {
                    for (int px = x; px < std::min(x + pixelSize, width); ++px) {
                        buffer[py * width + px] = blockColor;
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_PIXELATE2D_H
