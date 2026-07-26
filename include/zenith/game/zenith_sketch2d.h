#ifndef ZENITH_SKETCH2D_H
#define ZENITH_SKETCH2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class SketchFilter2D {
public:
    static void applySketch(std::vector<uint32_t>& buffer, int width, int height, float threshold = 0.5f) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                uint32_t p = buffer[idx];

                float r = (p >> 16) & 0xFF;
                float g = (p >> 8) & 0xFF;
                float b = p & 0xFF;
                float luma = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;

                uint32_t strokeColor = 0xFFFFFFFF; // White paper background

                if (luma < threshold) {
                    // Hatching lines pattern
                    if ((x + y) % 4 == 0 || (x - y) % 4 == 0) {
                        strokeColor = 0xFF202020; // Pencil graphite line
                    }
                }

                if (luma < threshold * 0.5f) {
                    // Dense cross-hatching
                    if ((x * 2 + y) % 3 == 0) {
                        strokeColor = 0xFF101010;
                    }
                }

                buffer[idx] = strokeColor;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SKETCH2D_H
