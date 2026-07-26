#ifndef ZENITH_SCORE_NOTIFIER2D_H
#define ZENITH_SCORE_NOTIFIER2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct ScorePopup {
    std::string text;
    float x, y;
    float lifetime;
    float maxLifetime;
    uint32_t color;
};

class ScoreNotifier2D {
private:
    std::vector<ScorePopup> m_popups;

public:
    void addScore(const std::string& text, float x, float y, uint32_t color = 0xFFFFD700, float lifetime = 1.0f) {
        m_popups.push_back({ text, x, y, lifetime, lifetime, color });
    }

    void update(float dt, float floatSpeed = 40.0f) {
        for (auto it = m_popups.begin(); it != m_popups.end(); ) {
            it->lifetime -= dt;
            it->y -= floatSpeed * dt; // Float upwards
            if (it->lifetime <= 0.0f) {
                it = m_popups.erase(it);
            } else {
                ++it;
            }
        }
    }

    void drawScores(std::vector<uint32_t>& buffer, int width, int height) const {
        if (buffer.empty() || width <= 0 || height <= 0 || m_popups.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (const auto& pop : m_popups) {
            float alphaFactor = std::clamp(pop.lifetime / pop.maxLifetime, 0.0f, 1.0f);
            int pX = static_cast<int>(pop.x);
            int pY = static_cast<int>(pop.y);

            uint32_t r = static_cast<uint32_t>(((pop.color >> 16) & 0xFF) * alphaFactor);
            uint32_t g = static_cast<uint32_t>(((pop.color >> 8) & 0xFF) * alphaFactor);
            uint32_t b = static_cast<uint32_t>((pop.color & 0xFF) * alphaFactor);
            uint32_t col = (0xFF << 24) | (r << 16) | (g << 8) | b;

            int textLen = static_cast<int>(pop.text.length() * 8);
            for (int dy = 0; dy < 10; ++dy) {
                for (int dx = 0; dx < textLen; ++dx) {
                    drawPixel(pX + dx, pY + dy, col);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SCORE_NOTIFIER2D_H
