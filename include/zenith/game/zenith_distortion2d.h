#ifndef ZENITH_DISTORTION2D_H
#define ZENITH_DISTORTION2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct DistortionWave2D {
    float x = 0.0f;
    float y = 0.0f;
    float radius = 0.0f;
    float maxRadius = 200.0f;
    float speed = 300.0f;
    float strength = 15.0f;
    bool active = true;
};

class DistortionSystem2D {
private:
    std::vector<DistortionWave2D> m_waves;

public:
    DistortionSystem2D() = default;

    void addShockwave(float x, float y, float maxRadius = 200.0f, float speed = 300.0f, float strength = 15.0f) {
        m_waves.push_back({x, y, 0.0f, maxRadius, speed, strength, true});
    }

    void update(float dt) {
        for (auto it = m_waves.begin(); it != m_waves.end();) {
            it->radius += it->speed * dt;
            if (it->radius >= it->maxRadius) {
                it = m_waves.erase(it);
            } else {
                ++it;
            }
        }
    }

    void getDisplacement(float px, float py, float& outOffsetX, float& outOffsetY) const {
        outOffsetX = 0.0f;
        outOffsetY = 0.0f;

        for (const auto& wave : m_waves) {
            float dx = px - wave.x;
            float dy = py - wave.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            float ringWidth = 30.0f;
            float diff = std::abs(dist - wave.radius);
            if (diff < ringWidth && dist > 0.0001f) {
                float factor = (1.0f - diff / ringWidth) * (1.0f - wave.radius / wave.maxRadius);
                outOffsetX += (dx / dist) * wave.strength * factor;
                outOffsetY += (dy / dist) * wave.strength * factor;
            }
        }
    }

    std::size_t getWaveCount() const { return m_waves.size(); }
};

} // namespace zenith

#endif // ZENITH_DISTORTION2D_H
