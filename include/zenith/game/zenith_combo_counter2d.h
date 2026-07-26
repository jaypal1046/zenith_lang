#ifndef ZENITH_COMBO_COUNTER2D_H
#define ZENITH_COMBO_COUNTER2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ComboCounter2D {
private:
    int m_comboCount = 0;
    float m_comboTimer = 0.0f;
    float m_maxComboTimer = 2.5f;
    float m_scalePulse = 1.0f;

public:
    void addHit(int count = 1) {
        m_comboCount += count;
        m_comboTimer = m_maxComboTimer;
        m_scalePulse = 1.4f; // Trigger bounce pop
    }

    void reset() {
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_scalePulse = 1.0f;
    }

    void update(float dt) {
        if (m_comboCount > 0) {
            m_comboTimer -= dt;
            if (m_comboTimer <= 0.0f) {
                reset();
            }
        }
        m_scalePulse = std::max(1.0f, m_scalePulse - dt * 2.0f);
    }

    int getComboCount() const { return m_comboCount; }
    float getComboPercent() const { return std::clamp(m_comboTimer / m_maxComboTimer, 0.0f, 1.0f); }

    void drawCombo(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 50, int posY = 150, uint32_t color = 0xFFFFD700) // Gold color
    {
        if (m_comboCount <= 1 || buffer.empty() || width <= 0 || height <= 0) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render combo numbers & multiplier bar representation
        int digits = m_comboCount;
        int numLength = 0;
        int temp = digits;
        while (temp > 0) { numLength++; temp /= 10; }

        int size = static_cast<int>(12.0f * m_scalePulse);

        // Draw block digits for combo count
        for (int i = 0; i < numLength; ++i) {
            int dX = posX + i * (size + 4);
            for (int dy = 0; dy < size * 2; ++dy) {
                for (int dx = 0; dx < size; ++dx) {
                    drawPixel(dX + dx, posY + dy, color);
                }
            }
        }

        // Draw combo timer bar below
        int barW = static_cast<int>(numLength * (size + 4) * getComboPercent());
        for (int dy = 0; dy < 3; ++dy) {
            for (int dx = 0; dx < barW; ++dx) {
                drawPixel(posX + dx, posY + size * 2 + 4 + dy, 0xFF00E5FF);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_COMBO_COUNTER2D_H
