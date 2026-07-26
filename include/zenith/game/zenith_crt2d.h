#ifndef ZENITH_CRT2D_H
#define ZENITH_CRT2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class CRTFilter2D {
public:
    static void applyScanlines(std::vector<uint32_t>& buffer, int width, int height, int lineSpacing = 2, float intensity = 0.25f) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        float factor = 1.0f - intensity;

        for (int y = 0; y < height; ++y) {
            if (y % lineSpacing == 0) {
                for (int x = 0; x < width; ++x) {
                    uint32_t pixel = buffer[y * width + x];
                    uint32_t r = static_cast<uint32_t>(((pixel >> 16) & 0xFF) * factor);
                    uint32_t g = static_cast<uint32_t>(((pixel >> 8) & 0xFF) * factor);
                    uint32_t b = static_cast<uint32_t>((pixel & 0xFF) * factor);
                    buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CRT2D_H
