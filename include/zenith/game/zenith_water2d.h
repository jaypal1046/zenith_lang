#ifndef ZENITH_WATER2D_H
#define ZENITH_WATER2D_H

#include "zenith_window.h"
#include <vector>
#include <algorithm>

namespace zenith {

struct WaterSpring {
    float height = 0.0f;
    float targetHeight = 0.0f;
    float velocity = 0.0f;
};

class WaterBody2D {
private:
    std::vector<WaterSpring> m_springs;
    float m_tension = 0.025f;
    float m_damping = 0.025f;
    float m_spread = 0.25f;

public:
    WaterBody2D(std::size_t numSprings = 30, float baseHeight = 100.0f) {
        m_springs.resize(numSprings);
        for (auto& spring : m_springs) {
            spring.height = baseHeight;
            spring.targetHeight = baseHeight;
            spring.velocity = 0.0f;
        }
    }

    void splash(std::size_t index, float speed) {
        if (index < m_springs.size()) {
            m_springs[index].velocity = speed;
        }
    }

    void update(float dt) {
        (void)dt;
        for (auto& spring : m_springs) {
            float x = spring.height - spring.targetHeight;
            float accel = -m_tension * x - m_damping * spring.velocity;
            spring.height += spring.velocity;
            spring.velocity += accel;
        }

        std::vector<float> leftDeltas(m_springs.size(), 0.0f);
        std::vector<float> rightDeltas(m_springs.size(), 0.0f);

        for (std::size_t i = 0; i < m_springs.size(); ++i) {
            if (i > 0) {
                leftDeltas[i] = m_spread * (m_springs[i].height - m_springs[i - 1].height);
                m_springs[i - 1].velocity += leftDeltas[i];
            }
            if (i < m_springs.size() - 1) {
                rightDeltas[i] = m_spread * (m_springs[i].height - m_springs[i + 1].height);
                m_springs[i + 1].velocity += rightDeltas[i];
            }
        }
    }

    std::size_t getSpringCount() const { return m_springs.size(); }
    float getSpringHeight(std::size_t index) const {
        return (index < m_springs.size()) ? m_springs[index].height : 0.0f;
    }
};

} // namespace zenith

#endif // ZENITH_WATER2D_H
