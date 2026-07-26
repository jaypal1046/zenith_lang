#ifndef ZENITH_RAIN_DROPS2D_H
#define ZENITH_RAIN_DROPS2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RainDrops2D {
public:
    static void applyRainDrops(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, int dropCount = 15, float trickleSpeed = 80.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || dropCount <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int i = 0; i < dropCount; ++i) {
            float seedX = (i * 137.5f);
            float dropX = static_cast<float>(static_cast<int>(seedX) % width);
            float dropY = static_cast<float>(static_cast<int>(time * trickleSpeed + i * 89.3f) % height);
            float radius = 4.0f + (i % 3) * 2.0f;

            int minX = std::max(0, static_cast<int>(dropX - radius));
            int maxX = std::min(width - 1, static_cast<int>(dropX + radius));
            int minY = std::max(0, static_cast<int>(dropY - radius));
            int maxY = std::min(height - 1, static_cast<int>(dropY + radius));

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    float dx = x - dropX;
                    float dy = y - dropY;
                    float distSq = dx * dx + dy * dy;

                    if (distSq <= radius * radius) {
                        float normDist = std::sqrt(distSq) / radius;
                        float refraction = (1.0f - normDist) * 6.0f;

                        int srcX = std::clamp(static_cast<int>(x + dx * refraction * 0.2f), 0, width - 1);
                        int srcY = std::clamp(static_cast<int>(y + dy * refraction * 0.2f), 0, height - 1);

                        uint32_t refr = original[srcY * width + srcX];
                        // Highlight rim
                        if (normDist > 0.7f) {
                            uint32_t r = std::min(255u, ((refr >> 16) & 0xFF) + 40u);
                            uint32_t g = std::min(255u, ((refr >> 8) & 0xFF)  + 40u);
                            uint32_t b = std::min(255u, (refr & 0xFF)         + 60u);
                            refr = (0xFF << 24) | (r << 16) | (g << 8) | b;
                        }

                        buffer[y * width + x] = refr;
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RAIN_DROPS2D_H
