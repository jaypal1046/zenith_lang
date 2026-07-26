#ifndef ZENITH_RACING_GAUGE2D_H
#define ZENITH_RACING_GAUGE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RacingGauge2D {
public:
    static void drawGauge(
        std::vector<uint32_t>& buffer, int width, int height,
        float currentValue, float maxValue,
        int centerX = 100, int centerY = 100, float radius = 50.0f,
        uint32_t needleColor = 0xFFFF0000, uint32_t dialColor = 0xFF00FF00)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || maxValue <= 0.0f || radius <= 0.0f) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Draw dial arc (-135 degrees to +135 degrees)
        float pct = std::clamp(currentValue / maxValue, 0.0f, 1.0f);
        float startAngle = -2.35619f; // -135 deg in rad
        float endAngle = 2.35619f;    // +135 deg in rad

        for (float a = startAngle; a <= endAngle; a += 0.05f) {
            int px = static_cast<int>(centerX + std::cos(a) * radius);
            int py = static_cast<int>(centerY + std::sin(a) * radius);
            drawPixel(px, py, dialColor);
            drawPixel(px + 1, py, dialColor);
        }

        // Draw active needle
        float needleAngle = startAngle + (endAngle - startAngle) * pct;
        for (float r = 0.0f; r <= radius - 4.0f; r += 1.0f) {
            int nX = static_cast<int>(centerX + std::cos(needleAngle) * r);
            int nY = static_cast<int>(centerY + std::sin(needleAngle) * r);
            drawPixel(nX, nY, needleColor);
            drawPixel(nX + 1, nY, needleColor);
        }

        // Center pivot pin
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                drawPixel(centerX + dx, centerY + dy, 0xFFFFFFFF);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RACING_GAUGE2D_H
