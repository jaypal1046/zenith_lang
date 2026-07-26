#ifndef ZENITH_VIGNETTE2D_H
#define ZENITH_VIGNETTE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class VignetteFilter2D {
public:
    static void applyVignette(
        std::vector<uint32_t>& buffer, int width, int height,
        float radius = 0.75f, float intensity = 0.6f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        float centerX = width * 0.5f;
        float centerY = height * 0.5f;
        float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;
                float dist = std::sqrt(dx * dx + dy * dy) / maxDist;

                if (dist > radius) {
                    float factor = 1.0f - std::clamp((dist - radius) / (1.0f - radius) * intensity, 0.0f, 1.0f);

                    uint32_t pixel = buffer[y * width + x];
                    uint32_t r = static_cast<uint32_t>(((pixel >> 16) & 0xFF) * factor);
                    uint32_t g = static_cast<uint32_t>(((pixel >> 8) & 0xFF) * factor);
                    uint32_t b = static_cast<uint32_t>((pixel & 0xFF) * factor);

                    buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_VIGNETTE2D_H
