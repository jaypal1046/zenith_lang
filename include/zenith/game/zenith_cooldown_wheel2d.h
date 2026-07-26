#ifndef ZENITH_COOLDOWN_WHEEL2D_H
#define ZENITH_COOLDOWN_WHEEL2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class CooldownWheel2D {
public:
    static void drawCooldownWheel(
        std::vector<uint32_t>& buffer, int width, int height,
        float cooldownPercent, int posX = 100, int posY = 100, float radius = 24.0f,
        uint32_t iconColor = 0xFF4169E1, uint32_t maskColor = 0xAA000000)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0.0f) return;

        cooldownPercent = std::clamp(cooldownPercent, 0.0f, 1.0f);

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render circular icon base
        int r = static_cast<int>(radius);
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                float distSq = dx * dx + dy * dy;
                if (distSq <= radius * radius) {
                    drawPixel(posX + dx, posY + dy, iconColor);
                }
            }
        }

        // Draw radial clock sweep mask for active cooldown
        if (cooldownPercent > 0.0f) {
            float maxAngle = cooldownPercent * 6.2831853f; // 2 * PI

            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    if (dx * dx + dy * dy <= radius * radius) {
                        float angle = std::atan2(-dx, dy); // Top clockwise angle
                        if (angle < 0) angle += 6.2831853f;

                        if (angle <= maxAngle) {
                            int px = posX + dx;
                            int py = posY + dy;
                            if (px >= 0 && px < width && py >= 0 && py < height) {
                                uint32_t orig = buffer[py * width + px];
                                uint32_t cr = ((orig >> 16) & 0xFF) / 2;
                                uint32_t cg = ((orig >> 8) & 0xFF) / 2;
                                uint32_t cb = (orig & 0xFF) / 2;
                                buffer[py * width + px] = (0xFF << 24) | (cr << 16) | (cg << 8) | cb;
                            }
                        }
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_COOLDOWN_WHEEL2D_H
