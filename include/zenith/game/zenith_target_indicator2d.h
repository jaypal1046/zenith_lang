#ifndef ZENITH_TARGET_INDICATOR2D_H
#define ZENITH_TARGET_INDICATOR2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class TargetIndicator2D {
public:
    static void drawAOECircle(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX, int posY, float radius,
        uint32_t color = 0x8000E5FF, uint32_t borderCol = 0xFF00E5FF)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0.0f) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        int r = static_cast<int>(radius);
        float rSq = radius * radius;

        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                float distSq = dx * dx + dy * dy;
                if (distSq <= rSq) {
                    if (distSq >= (radius - 2.0f) * (radius - 2.0f)) {
                        drawPixel(posX + dx, posY + dy, borderCol);
                    } else {
                        drawPixel(posX + dx, posY + dy, color);
                    }
                }
            }
        }
    }

    static void drawConeArc(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX, int posY, float range, float centerAngle, float halfFov,
        uint32_t color = 0x80FF4500)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || range <= 0.0f) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        int r = static_cast<int>(range);
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (dx * dx + dy * dy <= range * range) {
                    float angle = std::atan2(static_cast<float>(dy), static_cast<float>(dx));
                    float diff = std::abs(angle - centerAngle);
                    if (diff > 3.14159265f) diff = 6.2831853f - diff;

                    if (diff <= halfFov) {
                        drawPixel(posX + dx, posY + dy, color);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_TARGET_INDICATOR2D_H
