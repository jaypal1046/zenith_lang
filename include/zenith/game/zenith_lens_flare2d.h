#ifndef ZENITH_LENS_FLARE2D_H
#define ZENITH_LENS_FLARE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct FlareElement {
    float offsetRatio; // Position along screen center axis (-1.0 to 1.0)
    float radius;
    uint32_t color;
};

class LensFlare2D {
public:
    static std::vector<FlareElement> generateFlareChain(uint32_t lightColor = 0xFFFFFF00) {
        return {
            {-0.8f, 15.0f, lightColor & 0x66FFFFFF},
            {-0.4f, 25.0f, lightColor & 0x88FFFFFF},
            {-0.1f, 10.0f, lightColor & 0x44FFFFFF},
            { 0.2f, 35.0f, lightColor & 0xAAFFFFFF},
            { 0.5f, 18.0f, lightColor & 0x66FFFFFF},
            { 0.9f, 45.0f, lightColor & 0xDDFFFFFF}
        };
    }

    static void applyAnamorphicStreak(
        std::vector<uint32_t>& buffer, int width, int height,
        int lightX, int lightY, uint32_t streakColor = 0xFF00FFFF, int streakLength = 60)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || lightY < 0 || lightY >= height) return;

        for (int dx = -streakLength; dx <= streakLength; ++dx) {
            int px = lightX + dx;
            if (px >= 0 && px < width) {
                int idx = lightY * width + px;
                float falloff = 1.0f - std::abs(dx) / static_cast<float>(streakLength);

                uint32_t orig = buffer[idx];
                uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) + ((streakColor >> 16) & 0xFF) * falloff);
                uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  + ((streakColor >> 8) & 0xFF)  * falloff);
                uint32_t b = static_cast<uint32_t>((orig & 0xFF)         + (streakColor & 0xFF)         * falloff);

                r = std::min(r, 255u);
                g = std::min(g, 255u);
                b = std::min(b, 255u);

                buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LENS_FLARE2D_H
