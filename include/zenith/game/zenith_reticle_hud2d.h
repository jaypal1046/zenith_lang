#ifndef ZENITH_RETICLE_HUD2D_H
#define ZENITH_RETICLE_HUD2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ReticleHUD2D {
public:
    static void drawReticle(
        std::vector<uint32_t>& buffer, int width, int height,
        float targetX, float targetY, float radius = 30.0f,
        bool isLocked = false, uint32_t color = 0xFF00FF00)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0.0f) return;

        uint32_t reticleColor = isLocked ? 0xFFFF0000 : color;

        // Outer corner bracket markers
        int tX = static_cast<int>(targetX);
        int tY = static_cast<int>(targetY);
        int r = static_cast<int>(radius);

        auto drawPixel = [&](int px, int py) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = reticleColor;
            }
        };

        // Crosshair ticks
        for (int i = 5; i <= 15; ++i) {
            drawPixel(tX - r - i, tY);
            drawPixel(tX + r + i, tY);
            drawPixel(tX, tY - r - i);
            drawPixel(tX, tY + r + i);
        }

        // Center dot
        drawPixel(tX, tY);
        drawPixel(tX + 1, tY);
        drawPixel(tX, tY + 1);
        drawPixel(tX + 1, tY + 1);

        // Circular ring ticks
        for (int angle = 0; angle < 360; angle += 10) {
            float rad = angle * 0.0174533f;
            int px = static_cast<int>(tX + std::cos(rad) * radius);
            int py = static_cast<int>(tY + std::sin(rad) * radius);
            drawPixel(px, py);
        }
    }
};

} // namespace zenith

#endif // ZENITH_RETICLE_HUD2D_H
