#ifndef ZENITH_BLOOM2D_H
#define ZENITH_BLOOM2D_H

#include <vector>
#include <algorithm>
#include <cstdint>
#include "zenith_blur2d.h"

namespace zenith {

class BloomPass2D {
public:
    static void applyBloom(
        std::vector<uint32_t>& buffer, int width, int height,
        float threshold = 0.7f, float bloomIntensity = 0.5f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> glowBuffer = buffer;

        // 1. Extract bright pixels
        for (std::size_t i = 0; i < glowBuffer.size(); ++i) {
            uint32_t pixel = glowBuffer[i];
            float r = ((pixel >> 16) & 0xFF) / 255.0f;
            float g = ((pixel >> 8) & 0xFF) / 255.0f;
            float b = (pixel & 0xFF) / 255.0f;

            float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            if (luminance < threshold) {
                glowBuffer[i] = 0xFF000000;
            }
        }

        // 2. Blur bright highlights
        BlurPass2D::applyBoxBlur(glowBuffer, width, height, 2);

        // 3. Additively blend glow buffer back to original
        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t orig = buffer[i];
            uint32_t glow = glowBuffer[i];

            uint32_t r = std::min(255u, static_cast<uint32_t>(((orig >> 16) & 0xFF) + ((glow >> 16) & 0xFF) * bloomIntensity));
            uint32_t g = std::min(255u, static_cast<uint32_t>(((orig >> 8) & 0xFF) + ((glow >> 8) & 0xFF) * bloomIntensity));
            uint32_t b = std::min(255u, static_cast<uint32_t>((orig & 0xFF) + (glow & 0xFF) * bloomIntensity));

            buffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
};

} // namespace zenith

#endif // ZENITH_BLOOM2D_H
