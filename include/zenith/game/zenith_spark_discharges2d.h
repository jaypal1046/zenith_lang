#ifndef ZENITH_SPARK_DISCHARGES2D_H
#define ZENITH_SPARK_DISCHARGES2D_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct SparkArc {
    float x1, y1;
    float x2, y2;
    float lifetime;
    float maxLifetime;
    uint32_t color;
};

class SparkDischarges2D {
private:
    std::vector<SparkArc> m_arcs;

public:
    void triggerSparks(float startX, float startY, int arcCount = 6, uint32_t color = 0xFF00FFFF) {
        for (int i = 0; i < arcCount; ++i) {
            float angle = (std::rand() % 360) * 0.0174533f;
            float length = 20.0f + (std::rand() % 40);
            SparkArc arc;
            arc.x1 = startX;
            arc.y1 = startY;
            arc.x2 = startX + std::cos(angle) * length;
            arc.y2 = startY + std::sin(angle) * length;
            arc.lifetime = 0.15f;
            arc.maxLifetime = 0.15f;
            arc.color = color;
            m_arcs.push_back(arc);
        }
    }

    void update(float dt) {
        for (auto& arc : m_arcs) {
            arc.lifetime -= dt;
        }
        m_arcs.erase(
            std::remove_if(m_arcs.begin(), m_arcs.end(), [](const SparkArc& a) { return a.lifetime <= 0.0f; }),
            m_arcs.end()
        );
    }

    const std::vector<SparkArc>& getArcs() const { return m_arcs; }
};

} // namespace zenith

#endif // ZENITH_SPARK_DISCHARGES2D_H
