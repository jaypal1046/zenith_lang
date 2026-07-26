#ifndef ZENITH_CHROMATIC_VIGNETTE2D_H
#define ZENITH_CHROMATIC_VIGNETTE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ChromaticVignetteFilter2D {
public:
    static void applyChromaticVignette(
        std::vector<uint32_t>& buffer, int width, int height,
        float fringeStrength = 5.0f, float vignettePower = 1.5f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;
        float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;
                float distNorm = std::sqrt(dx * dx + dy * dy) / maxDist;

                float fringe = distNorm * fringeStrength;
                int rX = std::clamp(static_cast<int>(x + fringe), 0, width - 1);
                int bX = std::clamp(static_cast<int>(x - fringe), 0, width - 1);

                uint32_t pR = original[y * width + rX];
                uint32_t pG = original[y * width + x];
                uint32_t pB = original[y * width + bX];

                uint32_t r = (pR >> 16) & 0xFF;
                uint32_t g = (pG >> 8) & 0xFF;
                uint32_t b = pB & 0xFF;

                float vignette = 1.0f - std::pow(distNorm, vignettePower);
                vignette = std::clamp(vignette, 0.0f, 1.0f);

                r = static_cast<uint32_t>(r * vignette);
                g = static_cast<uint32_t>(g * vignette);
                b = static_cast<uint32_t>(b * vignette);

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CHROMATIC_VIGNETTE2D_H
