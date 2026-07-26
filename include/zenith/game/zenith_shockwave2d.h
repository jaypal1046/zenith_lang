#ifndef ZENITH_SHOCKWAVE2D_H
#define ZENITH_SHOCKWAVE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class Shockwave2D {
private:
    float m_centerX = 0.0f;
    float m_centerY = 0.0f;
    float m_radius = 0.0f;
    float m_maxRadius = 300.0f;
    float m_waveWidth = 30.0f;
    float m_amplitude = 25.0f;
    bool m_active = false;

public:
    void trigger(float x, float y, float maxRadius = 300.0f, float amplitude = 25.0f) {
        m_centerX = x;
        m_centerY = y;
        m_radius = 0.0f;
        m_maxRadius = maxRadius;
        m_amplitude = amplitude;
        m_active = true;
    }

    void update(float dt, float expansionSpeed = 500.0f) {
        if (!m_active) return;
        m_radius += expansionSpeed * dt;
        if (m_radius >= m_maxRadius) {
            m_active = false;
        }
    }

    bool isActive() const { return m_active; }

    void applyShockwave(std::vector<uint32_t>& buffer, int width, int height) const {
        if (!m_active || buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - m_centerX;
                float dy = y - m_centerY;
                float dist = std::sqrt(dx * dx + dy * dy);

                float diff = dist - m_radius;
                if (std::abs(diff) < m_waveWidth && dist > 1.0f) {
                    float factor = std::sin(diff / m_waveWidth * 3.14159265f);
                    float offset = factor * m_amplitude * (1.0f - m_radius / m_maxRadius);

                    float normX = dx / dist;
                    float normY = dy / dist;

                    int srcX = std::clamp(static_cast<int>(x + normX * offset), 0, width - 1);
                    int srcY = std::clamp(static_cast<int>(y + normY * offset), 0, height - 1);

                    buffer[y * width + x] = original[srcY * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SHOCKWAVE2D_H
