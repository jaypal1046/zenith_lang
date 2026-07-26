#ifndef ZENITH_HEAT_HAZE2D_H
#define ZENITH_HEAT_HAZE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class HeatHazeFilter2D {
public:
    static void applyHeatHaze(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float hazeSpeed = 4.0f, float hazeIntensity = 6.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            float shiftX = std::sin((y * 0.1f) + (time * hazeSpeed)) * hazeIntensity;
            float shiftY = std::cos((y * 0.08f) + (time * hazeSpeed * 0.7f)) * (hazeIntensity * 0.5f);

            for (int x = 0; x < width; ++x) {
                int srcX = std::clamp(static_cast<int>(x + shiftX), 0, width - 1);
                int srcY = std::clamp(static_cast<int>(y + shiftY), 0, height - 1);

                buffer[y * width + x] = original[srcY * width + srcX];
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_HEAT_HAZE2D_H
