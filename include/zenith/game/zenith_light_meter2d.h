#ifndef ZENITH_LIGHT_METER2D_H
#define ZENITH_LIGHT_METER2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class LightMeter2D {
private:
    float m_lightLevel = 0.0f; // [0.0 = Hidden in darkness, 1.0 = Fully illuminated]

public:
    void updateLightLevel(float lightLevel) {
        m_lightLevel = std::clamp(lightLevel, 0.0f, 1.0f);
    }

    float getLightLevel() const { return m_lightLevel; }

    void drawLightMeter(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 140, int barWidth = 80, int barHeight = 12) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Dark container background with white border
        for (int y = posY; y < posY + barHeight; ++y) {
            for (int x = posX; x < posX + barWidth; ++x) {
                if (x == posX || x == posX + barWidth - 1 || y == posY || y == posY + barHeight - 1) {
                    drawPixel(x, y, 0xFF808080);
                } else {
                    drawPixel(x, y, 0xFF101015);
                }
            }
        }

        // Fill visibility meter: Dark Blue (Stealth) -> Bright Yellow (Exposed)
        int fillW = static_cast<int>((barWidth - 4) * m_lightLevel);
        uint32_t r = static_cast<uint32_t>(255 * m_lightLevel);
        uint32_t g = static_cast<uint32_t>(215 * m_lightLevel);
        uint32_t b = static_cast<uint32_t>(255 * (1.0f - m_lightLevel));
        uint32_t fillCol = (0xFF << 24) | (r << 16) | (g << 8) | b;

        for (int y = posY + 2; y < posY + barHeight - 2; ++y) {
            for (int x = posX + 2; x < posX + 2 + fillW; ++x) {
                drawPixel(x, y, fillCol);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LIGHT_METER2D_H
