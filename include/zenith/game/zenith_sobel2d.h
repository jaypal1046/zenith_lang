#ifndef ZENITH_SOBEL2D_H
#define ZENITH_SOBEL2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class SobelFilter2D {
public:
    static void applySobel(std::vector<uint32_t>& buffer, int width, int height) {
        if (buffer.empty() || width < 3 || height < 3) return;

        std::vector<uint32_t> original = buffer;

        auto getGray = [&](int x, int y) -> float {
            uint32_t p = original[y * width + x];
            float r = ((p >> 16) & 0xFF) / 255.0f;
            float g = ((p >> 8) & 0xFF) / 255.0f;
            float b = (p & 0xFF) / 255.0f;
            return 0.2126f * r + 0.7152f * g + 0.0722f * b;
        };

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                float gx = -1 * getGray(x - 1, y - 1) + 1 * getGray(x + 1, y - 1)
                           -2 * getGray(x - 1, y)     + 2 * getGray(x + 1, y)
                           -1 * getGray(x - 1, y + 1) + 1 * getGray(x + 1, y + 1);

                float gy = -1 * getGray(x - 1, y - 1) - 2 * getGray(x, y - 1) - 1 * getGray(x + 1, y - 1)
                           +1 * getGray(x - 1, y + 1) + 2 * getGray(x, y + 1) + 1 * getGray(x + 1, y + 1);

                float mag = std::clamp(std::sqrt(gx * gx + gy * gy), 0.0f, 1.0f);
                uint32_t val = static_cast<uint32_t>(mag * 255.0f);

                buffer[y * width + x] = (0xFF << 24) | (val << 16) | (val << 8) | val;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SOBEL2D_H
