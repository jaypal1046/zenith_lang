#ifndef ZENITH_LIGHT_RAYS2D_H
#define ZENITH_LIGHT_RAYS2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class LightRays2D {
public:
    static void applyGodRays(
        std::vector<uint32_t>& buffer, int width, int height,
        float sunNormX = 0.5f, float sunNormY = 0.1f,
        int samples = 8, float density = 0.8f, float decay = 0.9f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || samples <= 1) return;

        std::vector<uint32_t> original = buffer;
        float sunX = sunNormX * width;
        float sunY = sunNormY * height;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = (x - sunX) * density / static_cast<float>(samples);
                float dy = (y - sunY) * density / static_cast<float>(samples);

                float curX = static_cast<float>(x);
                float curY = static_cast<float>(y);
                float illuminationDecay = 1.0f;

                float accumR = 0.0f, accumG = 0.0f, accumB = 0.0f;

                for (int i = 0; i < samples; ++i) {
                    curX -= dx;
                    curY -= dy;

                    int sX = std::clamp(static_cast<int>(curX), 0, width - 1);
                    int sY = std::clamp(static_cast<int>(curY), 0, height - 1);

                    uint32_t samplePixel = original[sY * width + sX];
                    accumR += ((samplePixel >> 16) & 0xFF) * illuminationDecay;
                    accumG += ((samplePixel >> 8) & 0xFF)  * illuminationDecay;
                    accumB += (samplePixel & 0xFF)         * illuminationDecay;

                    illuminationDecay *= decay;
                }

                uint32_t orig = original[y * width + x];
                uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) + accumR / samples);
                uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  + accumG / samples);
                uint32_t b = static_cast<uint32_t>((orig & 0xFF)         + accumB / samples);

                r = std::min(r, 255u);
                g = std::min(g, 255u);
                b = std::min(b, 255u);

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LIGHT_RAYS2D_H
