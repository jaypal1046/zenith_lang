#ifndef ZENITH_RECOIL_CROSSHAIR2D_H
#define ZENITH_RECOIL_CROSSHAIR2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RecoilCrosshair2D {
private:
    float m_spread = 8.0f;
    float m_minSpread = 8.0f;
    float m_maxSpread = 32.0f;
    float m_recoverySpeed = 30.0f;

public:
    void fireShot(float recoilKick = 6.0f) {
        m_spread = std::min(m_maxSpread, m_spread + recoilKick);
    }

    void update(float dt) {
        m_spread = std::max(m_minSpread, m_spread - m_recoverySpeed * dt);
    }

    float getSpread() const { return m_spread; }

    void drawCrosshair(
        std::vector<uint32_t>& buffer, int width, int height,
        int centerX = -1, int centerY = -1, int reticleLength = 8,
        uint32_t color = 0xFF00FF00) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        int cX = (centerX < 0) ? width / 2 : centerX;
        int cY = (centerY < 0) ? height / 2 : centerY;
        int gap = static_cast<int>(m_spread);

        auto drawPixel = [&](int px, int py) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = color;
            }
        };

        // Top line
        for (int i = 0; i < reticleLength; ++i) drawPixel(cX, cY - gap - i);
        // Bottom line
        for (int i = 0; i < reticleLength; ++i) drawPixel(cX, cY + gap + i);
        // Left line
        for (int i = 0; i < reticleLength; ++i) drawPixel(cX - gap - i, cY);
        // Right line
        for (int i = 0; i < reticleLength; ++i) drawPixel(cX + gap + i, cY);
    }
};

} // namespace zenith

#endif // ZENITH_RECOIL_CROSSHAIR2D_H
