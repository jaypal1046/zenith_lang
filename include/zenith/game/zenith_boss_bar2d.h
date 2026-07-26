#ifndef ZENITH_BOSS_BAR2D_H
#define ZENITH_BOSS_BAR2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class BossBar2D {
public:
    static void drawBossBar(
        std::vector<uint32_t>& buffer, int width, int height,
        float currentHealth, float maxHealth,
        float currentShield = 0.0f, float maxShield = 0.0f,
        int barY = 30, int barWidth = 400, int barHeight = 16,
        uint32_t hpColor = 0xFFFF0000, uint32_t shieldColor = 0xFF00E5FF)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || maxHealth <= 0.0f) return;

        int startX = (width - barWidth) / 2;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        float hpPercent = std::clamp(currentHealth / maxHealth, 0.0f, 1.0f);
        int hpPixels = static_cast<int>(barWidth * hpPercent);

        float shieldPercent = maxShield > 0.0f ? std::clamp(currentShield / maxShield, 0.0f, 1.0f) : 0.0f;
        int shieldPixels = static_cast<int>(barWidth * shieldPercent);

        // Draw Outer Frame
        for (int y = barY - 2; y <= barY + barHeight + 2; ++y) {
            for (int x = startX - 2; x <= startX + barWidth + 2; ++x) {
                if (x < startX || x > startX + barWidth || y < barY || y > barY + barHeight) {
                    drawPixel(x, y, 0xFFE0E0E0); // Metallic gold border
                } else {
                    drawPixel(x, y, 0xFF1A1A1A); // Dark background
                }
            }
        }

        // Draw HP Fill
        for (int y = barY; y < barY + barHeight; ++y) {
            for (int x = startX; x < startX + hpPixels; ++x) {
                drawPixel(x, y, hpColor);
            }
        }

        // Draw Shield Fill Overlay (Top 4px of bar)
        if (shieldPixels > 0) {
            for (int y = barY; y < barY + 4; ++y) {
                for (int x = startX; x < startX + shieldPixels; ++x) {
                    drawPixel(x, y, shieldColor);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_BOSS_BAR2D_H
