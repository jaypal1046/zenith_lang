#ifndef ZENITH_STEALTH_CONE2D_H
#define ZENITH_STEALTH_CONE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class StealthCone2D {
public:
    static void drawGuardVisionCone(
        std::vector<uint32_t>& buffer, int width, int height,
        float guardX, float guardY, float facingAngle, float fovAngle, float range,
        float alertPercent = 0.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || range <= 0.0f) return;

        alertPercent = std::clamp(alertPercent, 0.0f, 1.0f);

        // Color transition: Green (0x5000FF00) -> Yellow (0x50FFFF00) -> Red (0x60FF0000)
        uint32_t coneColor;
        if (alertPercent < 0.5f) {
            float t = alertPercent * 2.0f;
            uint32_t r = static_cast<uint32_t>(255 * t);
            coneColor = (0x50 << 24) | (r << 16) | (0xFF << 8) | 0x00;
        } else {
            float t = (alertPercent - 0.5f) * 2.0f;
            uint32_t g = static_cast<uint32_t>(255 * (1.0f - t));
            coneColor = (0x60 << 24) | (0xFF << 16) | (g << 8) | 0x00;
        }

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        int r = static_cast<int>(range);
        int gX = static_cast<int>(guardX);
        int gY = static_cast<int>(guardY);
        float halfFov = fovAngle * 0.5f;

        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (dx * dx + dy * dy <= range * range) {
                    float angle = std::atan2(static_cast<float>(dy), static_cast<float>(dx));
                    float diff = std::abs(angle - facingAngle);
                    if (diff > 3.14159265f) diff = 6.2831853f - diff;

                    if (diff <= halfFov) {
                        drawPixel(gX + dx, gY + dy, coneColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_STEALTH_CONE2D_H
