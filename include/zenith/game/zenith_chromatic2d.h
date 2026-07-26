#ifndef ZENITH_CHROMATIC2D_H
#define ZENITH_CHROMATIC2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ChromaticAberration2D {
public:
    static void applyRGBShift(
        std::vector<uint32_t>& buffer, int width, int height,
        int rShiftX, int rShiftY,
        int bShiftX, int bShiftY)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int rx = std::clamp(x + rShiftX, 0, width - 1);
                int ry = std::clamp(y + rShiftY, 0, height - 1);
                int bx = std::clamp(x + bShiftX, 0, width - 1);
                int by = std::clamp(y + bShiftY, 0, height - 1);

                uint32_t rPixel = original[ry * width + rx];
                uint32_t gPixel = original[y * width + x];
                uint32_t bPixel = original[by * width + bx];

                uint32_t r = (rPixel >> 16) & 0xFF;
                uint32_t g = (gPixel >> 8) & 0xFF;
                uint32_t b = bPixel & 0xFF;

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CHROMATIC2D_H
