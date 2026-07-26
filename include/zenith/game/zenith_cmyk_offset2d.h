#ifndef ZENITH_CMYK_OFFSET2D_H
#define ZENITH_CMYK_OFFSET2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class CMYKOffset2D {
public:
    static void applyCMYKOffset(
        std::vector<uint32_t>& buffer, int width, int height,
        int offsetX = 3, int offsetY = 2)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Shift Cyan/Red and Yellow/Blue plates separately to mimic misaligned printing press
                int srcCx = std::clamp(x - offsetX, 0, width - 1);
                int srcCy = std::clamp(y - offsetY, 0, height - 1);

                int srcMx = std::clamp(x + offsetX, 0, width - 1);
                int srcMy = std::clamp(y + offsetY, 0, height - 1);

                uint32_t pC = original[srcCy * width + srcCx];
                uint32_t pM = original[srcMy * width + srcMx];
                uint32_t pY = original[y * width + x];

                uint32_t r = (pC >> 16) & 0xFF;
                uint32_t g = (pM >> 8) & 0xFF;
                uint32_t b = pY & 0xFF;

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CMYK_OFFSET2D_H
