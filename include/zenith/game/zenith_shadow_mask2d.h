#ifndef ZENITH_SHADOW_MASK2D_H
#define ZENITH_SHADOW_MASK2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ShadowMaskFilter2D {
public:
    enum class Pattern {
        ApertureGrille, // Trinitron RGB vertical stripes
        DotMatrix       // Retro CRT dot triad matrix
    };

    static void applyShadowMask(
        std::vector<uint32_t>& buffer, int width, int height,
        Pattern pattern = Pattern::ApertureGrille, float intensity = 0.25f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || intensity <= 0.0f) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                uint32_t pixel = buffer[idx];

                float factorR = 1.0f, factorG = 1.0f, factorB = 1.0f;

                if (pattern == Pattern::ApertureGrille) {
                    int subPixel = x % 3;
                    if (subPixel == 0)      { factorG -= intensity; factorB -= intensity; }
                    else if (subPixel == 1) { factorR -= intensity; factorB -= intensity; }
                    else                    { factorR -= intensity; factorG -= intensity; }
                } else if (pattern == Pattern::DotMatrix) {
                    if ((x + y) % 2 == 0) {
                        factorR -= intensity * 0.5f;
                        factorG -= intensity * 0.5f;
                        factorB -= intensity * 0.5f;
                    }
                }

                uint32_t r = static_cast<uint32_t>(std::clamp(((pixel >> 16) & 0xFF) * factorR, 0.0f, 255.0f));
                uint32_t g = static_cast<uint32_t>(std::clamp(((pixel >> 8) & 0xFF)  * factorG, 0.0f, 255.0f));
                uint32_t b = static_cast<uint32_t>(std::clamp((pixel & 0xFF)         * factorB, 0.0f, 255.0f));

                buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SHADOW_MASK2D_H
