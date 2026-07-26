#ifndef ZENITH_RIPPLE2D_H
#define ZENITH_RIPPLE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RippleSystem2D {
public:
    static void applySineRipple(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float amplitude = 4.0f, float frequency = 0.05f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || amplitude <= 0.0f) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            int offsetX = static_cast<int>(std::sin(y * frequency + time) * amplitude);
            for (int x = 0; x < width; ++x) {
                int srcX = std::clamp(x + offsetX, 0, width - 1);
                buffer[y * width + x] = original[y * width + srcX];
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RIPPLE2D_H
