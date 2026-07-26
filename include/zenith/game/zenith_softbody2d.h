#ifndef ZENITH_SOFTBODY2D_H
#define ZENITH_SOFTBODY2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct SoftNode2D {
    float x = 0.0f;
    float y = 0.0f;
    float prevX = 0.0f;
    float prevY = 0.0f;
};

struct SoftSpring2D {
    std::size_t nodeA = 0;
    std::size_t nodeB = 0;
    float restLength = 1.0f;
    float stiffness = 0.5f;
};

class SoftBody2D {
private:
    std::vector<SoftNode2D> m_nodes;
    std::vector<SoftSpring2D> m_springs;

public:
    SoftBody2D() = default;

    std::size_t addNode(float x, float y) {
        m_nodes.push_back({x, y, x, y});
        return m_nodes.size() - 1;
    }

    void addSpring(std::size_t a, std::size_t b, float stiffness = 0.5f) {
        if (a < m_nodes.size() && b < m_nodes.size()) {
            float dx = m_nodes[b].x - m_nodes[a].x;
            float dy = m_nodes[b].y - m_nodes[a].y;
            float len = std::sqrt(dx * dx + dy * dy);
            m_springs.push_back({a, b, len, stiffness});
        }
    }

    void update(float dt, float gravity = 980.0f) {
        // Verlet integration
        for (auto& node : m_nodes) {
            float vx = node.x - node.prevX;
            float vy = node.y - node.prevY;
            node.prevX = node.x;
            node.prevY = node.y;
            node.x += vx;
            node.y += vy + gravity * dt * dt;
        }

        // Satisfy spring constraints
        for (const auto& spring : m_springs) {
            auto& nA = m_nodes[spring.nodeA];
            auto& nB = m_nodes[spring.nodeB];
            float dx = nB.x - nA.x;
            float dy = nB.y - nA.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 0.0001f) {
                float diff = (dist - spring.restLength) / dist;
                float offX = dx * 0.5f * diff * spring.stiffness;
                float offY = dy * 0.5f * diff * spring.stiffness;
                nA.x += offX;
                nA.y += offY;
                nB.x -= offX;
                nB.y -= offY;
            }
        }
    }

    std::size_t getNodeCount() const { return m_nodes.size(); }
    const SoftNode2D& getNode(std::size_t index) const { return m_nodes[index]; }
};

} // namespace zenith

#endif // ZENITH_SOFTBODY2D_H
