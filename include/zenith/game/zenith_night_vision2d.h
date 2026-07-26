#ifndef ZENITH_NIGHT_VISION2D_H
#define ZENITH_NIGHT_VISION2D_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

class NightVisionFilter2D {
public:
    static void applyNightVision(
        std::vector<uint32_t>& buffer, int width, int height,
        float noiseAmount = 0.1f, float vignettePower = 2.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        float centerX = width * 0.5f;
        float centerY = height * 0.5f;
        float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                uint32_t pixel = buffer[idx];

                float r = (pixel >> 16) & 0xFF;
                float g = (pixel >> 8) & 0xFF;
                float b = pixel & 0xFF;

                float luminance = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;

                // Add green phosphor amplification
                float greenLuma = std::clamp(luminance * 1.5f, 0.0f, 1.0f);

                // Add noise grain
                float noise = ((rand() % 100) / 100.0f - 0.5f) * noiseAmount;
                greenLuma = std::clamp(greenLuma + noise, 0.0f, 1.0f);

                // Add lens vignette
                float dx = x - centerX;
                float dy = y - centerY;
                float distNorm = std::sqrt(dx * dx + dy * dy) / maxDist;
                float vignette = 1.0f - std::pow(distNorm, vignettePower);
                vignette = std::clamp(vignette, 0.0f, 1.0f);

                greenLuma *= vignette;

                uint32_t outR = static_cast<uint32_t>(greenLuma * 30.0f);  // Subtle red base
                uint32_t outG = static_cast<uint32_t>(greenLuma * 255.0f); // Bright green phosphor
                uint32_t outB = static_cast<uint32_t>(greenLuma * 40.0f);  // Subtle blue base

                buffer[idx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_NIGHT_VISION2D_H
