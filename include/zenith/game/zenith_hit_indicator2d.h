#ifndef ZENITH_HIT_INDICATOR2D_H
#define ZENITH_HIT_INDICATOR2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct HitArc {
    float angle; // Angle in radians relative to player facing
    float lifetime;
    float maxLifetime;
    uint32_t color;
};

class HitIndicator2D {
private:
    std::vector<HitArc> m_arcs;

public:
    void addHit(float attackerAngle, float lifetime = 1.0f, uint32_t color = 0xFFFF0000) {
        m_arcs.push_back({ attackerAngle, lifetime, lifetime, color });
    }

    void update(float dt) {
        for (auto it = m_arcs.begin(); it != m_arcs.end(); ) {
            it->lifetime -= dt;
            if (it->lifetime <= 0.0f) {
                it = m_arcs.erase(it);
            } else {
                ++it;
            }
        }
    }

    void drawIndicators(
        std::vector<uint32_t>& buffer, int width, int height,
        float playerFacingAngle = 0.0f, float indicatorRadius = 120.0f) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_arcs.empty()) return;

        int centerX = width / 2;
        int centerY = height / 2;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (const auto& arc : m_arcs) {
            float alphaFactor = std::clamp(arc.lifetime / arc.maxLifetime, 0.0f, 1.0f);
            float relAngle = arc.angle - playerFacingAngle;

            // Draw curved damage arc
            for (float dA = -0.2f; dA <= 0.2f; dA += 0.02f) {
                float a = relAngle + dA;
                int px = static_cast<int>(centerX + std::cos(a) * indicatorRadius);
                int py = static_cast<int>(centerY + std::sin(a) * indicatorRadius);

                uint32_t r = static_cast<uint32_t>(((arc.color >> 16) & 0xFF) * alphaFactor);
                uint32_t g = static_cast<uint32_t>(((arc.color >> 8) & 0xFF) * alphaFactor);
                uint32_t b = static_cast<uint32_t>((arc.color & 0xFF) * alphaFactor);

                drawPixel(px, py, (0xFF << 24) | (r << 16) | (g << 8) | b);
                drawPixel(px + 1, py, (0xFF << 24) | (r << 16) | (g << 8) | b);
                drawPixel(px, py + 1, (0xFF << 24) | (r << 16) | (g << 8) | b);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_HIT_INDICATOR2D_H
