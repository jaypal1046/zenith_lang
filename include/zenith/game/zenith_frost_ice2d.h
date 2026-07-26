#ifndef ZENITH_FROST_ICE2D_H
#define ZENITH_FROST_ICE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class FrostIce2D {
public:
    static void applyFrostIce(
        std::vector<uint32_t>& buffer, int width, int height,
        float freezeAmount = 0.5f, uint32_t iceColor = 0xFFD0F0FF) // 0xFF + Ice Light Blue
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        freezeAmount = std::clamp(freezeAmount, 0.0f, 1.0f);
        if (freezeAmount <= 0.0f) return;

        float maxDist = std::sqrt((width * 0.5f) * (width * 0.5f) + (height * 0.5f) * (height * 0.5f));
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;

                float dx = x - centerX;
                float dy = y - centerY;
                float distNorm = std::sqrt(dx * dx + dy * dy) / maxDist;

                // Spiky crystalline noise
                float angle = std::atan2(dy, dx);
                float crystalNoise = std::abs(std::sin(angle * 12.0f) * std::cos(angle * 7.0f)) * 0.25f;

                float freezeThreshold = 1.0f - freezeAmount + crystalNoise;

                if (distNorm > freezeThreshold) {
                    float factor = std::min(1.0f, (distNorm - freezeThreshold) * 3.0f);
                    uint32_t orig = buffer[idx];

                    uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) * (1.0f - factor) + ((iceColor >> 16) & 0xFF) * factor);
                    uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  * (1.0f - factor) + ((iceColor >> 8) & 0xFF)  * factor);
                    uint32_t b = static_cast<uint32_t>((orig & 0xFF)         * (1.0f - factor) + (iceColor & 0xFF)         * factor);

                    buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_FROST_ICE2D_H
