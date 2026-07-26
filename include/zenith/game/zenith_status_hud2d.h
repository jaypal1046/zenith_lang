#ifndef ZENITH_STATUS_HUD2D_H
#define ZENITH_STATUS_HUD2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct StatusEffect {
    std::string name;
    uint32_t iconColor;
    float duration;
    float maxDuration;
};

class StatusHUD2D {
private:
    std::vector<StatusEffect> m_effects;

public:
    void addEffect(const std::string& name, uint32_t iconColor, float duration) {
        m_effects.push_back({ name, iconColor, duration, duration });
    }

    void update(float dt) {
        for (auto it = m_effects.begin(); it != m_effects.end(); ) {
            it->duration -= dt;
            if (it->duration <= 0.0f) {
                it = m_effects.erase(it);
            } else {
                ++it;
            }
        }
    }

    void drawStatusIcons(
        std::vector<uint32_t>& buffer, int width, int height,
        int startX = 20, int startY = 80, int iconSize = 24, int iconSpacing = 8) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_effects.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_effects.size(); ++i) {
            int iX = startX + static_cast<int>(i) * (iconSize + iconSpacing);

            // Draw Icon Fill
            for (int y = startY; y < startY + iconSize; ++y) {
                for (int x = iX; x < iX + iconSize; ++x) {
                    drawPixel(x, y, m_effects[i].iconColor);
                }
            }

            // Draw Duration Sweep Overlay (Top down swipe)
            float pct = std::clamp(m_effects[i].duration / m_effects[i].maxDuration, 0.0f, 1.0f);
            int sweepH = static_cast<int>(iconSize * (1.0f - pct));

            for (int y = startY; y < startY + sweepH; ++y) {
                for (int x = iX; x < iX + iconSize; ++x) {
                    drawPixel(x, y, 0x80000000); // Dark translucent mask for expired time
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_STATUS_HUD2D_H
