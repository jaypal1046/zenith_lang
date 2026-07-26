#ifndef ZENITH_STATIC_NOISE2D_H
#define ZENITH_STATIC_NOISE2D_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

class StaticNoise2D {
public:
    static void applyStaticNoise(
        std::vector<uint32_t>& buffer, int width, int height,
        float intensity = 0.25f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        intensity = std::clamp(intensity, 0.0f, 1.0f);
        if (intensity <= 0.0f) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if ((std::rand() % 100) < static_cast<int>(intensity * 100.0f)) {
                    uint32_t noiseVal = std::rand() % 255;
                    buffer[y * width + x] = (0xFF << 24) | (noiseVal << 16) | (noiseVal << 8) | noiseVal;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_STATIC_NOISE2D_H
