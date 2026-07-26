#ifndef ZENITH_CAUSTICS2D_H
#define ZENITH_CAUSTICS2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class CausticsFilter2D {
public:
    static void applyCaustics(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float scale = 0.03f, float intensity = 0.35f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float fx = x * scale;
                float fy = y * scale;

                float c1 = std::sin(fx + time) + std::cos(fy + time * 1.3f);
                float c2 = std::sin(fx * 1.4f - time * 0.8f) + std::cos(fy * 1.2f + time);
                float caustics = std::pow(std::abs(c1 + c2) * 0.5f, 2.5f) * intensity;

                int idx = y * width + x;
                uint32_t orig = buffer[idx];

                uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) + 200.0f * caustics * 0.7f);
                uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  + 220.0f * caustics * 0.9f);
                uint32_t b = static_cast<uint32_t>((orig & 0xFF)         + 255.0f * caustics);

                r = std::min(r, 255u);
                g = std::min(g, 255u);
                b = std::min(b, 255u);

                buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CAUSTICS2D_H
