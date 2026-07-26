#ifndef ZENITH_RGB_WAVE2D_H
#define ZENITH_RGB_WAVE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RGBWaveFilter2D {
public:
    static void applyRGBWave(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float frequency = 5.0f, float amplitude = 8.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            float wave = std::sin((y * 0.05f) + (time * frequency)) * amplitude;
            int shiftR = static_cast<int>(wave);
            int shiftB = -shiftR;

            for (int x = 0; x < width; ++x) {
                int srcRx = std::clamp(x + shiftR, 0, width - 1);
                int srcBx = std::clamp(x + shiftB, 0, width - 1);

                uint32_t pR = original[y * width + srcRx];
                uint32_t pG = original[y * width + x];
                uint32_t pB = original[y * width + srcBx];

                uint32_t r = (pR >> 16) & 0xFF;
                uint32_t g = (pG >> 8) & 0xFF;
                uint32_t b = pB & 0xFF;

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RGB_WAVE2D_H
