#ifndef ZENITH_HEX_SHIELD2D_H
#define ZENITH_HEX_SHIELD2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class HexShield2D {
public:
    static void applyHexShield(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float hexSize = 20.0f, uint32_t gridColor = 0xFF00E5FF) // 0xFF + Cyan Shield
    {
        if (buffer.empty() || width <= 0 || height <= 0 || hexSize <= 0.0f) return;

        float r = hexSize;
        float h = r * 0.8660254f; // sqrt(3)/2

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float fx = x / r;
                float fy = y / h;

                // Hex grid edge calculation
                float dx = std::abs(std::fmod(fx, 3.0f) - 1.5f);
                float dy = std::abs(std::fmod(fy, 2.0f) - 1.0f);

                float pulse = (std::sin((x * 0.02f) + (y * 0.02f) + (time * 3.0f)) + 1.0f) * 0.5f;

                if (dx > 1.2f || dy > 0.85f) {
                    int idx = y * width + x;
                    uint32_t orig = buffer[idx];

                    float intensity = 0.3f + pulse * 0.4f;

                    uint32_t red = static_cast<uint32_t>(((orig >> 16) & 0xFF) + ((gridColor >> 16) & 0xFF) * intensity);
                    uint32_t green = static_cast<uint32_t>(((orig >> 8) & 0xFF)  + ((gridColor >> 8) & 0xFF)  * intensity);
                    uint32_t blue = static_cast<uint32_t>((orig & 0xFF)         + (gridColor & 0xFF)         * intensity);

                    red = std::min(red, 255u);
                    green = std::min(green, 255u);
                    blue = std::min(blue, 255u);

                    buffer[idx] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_HEX_SHIELD2D_H
