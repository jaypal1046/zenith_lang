#ifndef ZENITH_PINCH_BULGE2D_H
#define ZENITH_PINCH_BULGE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class PinchBulgeFilter2D {
public:
    static void applyPinchBulge(
        std::vector<uint32_t>& buffer, int width, int height,
        float radius = 150.0f, float factor = 0.5f) // factor > 0 = Bulge, factor < 0 = Pinch
    {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0.0f) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < radius) {
                    float normDist = dist / radius;
                    float scale = 1.0f + factor * (1.0f - normDist * normDist);

                    int srcX = std::clamp(static_cast<int>(centerX + dx / scale), 0, width - 1);
                    int srcY = std::clamp(static_cast<int>(centerY + dy / scale), 0, height - 1);

                    buffer[y * width + x] = original[srcY * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_PINCH_BULGE2D_H
