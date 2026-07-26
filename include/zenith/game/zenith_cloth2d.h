#ifndef ZENITH_CLOTH2D_H
#define ZENITH_CLOTH2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct ClothParticle2D {
    float x = 0.0f;
    float y = 0.0f;
    float prevX = 0.0f;
    float prevY = 0.0f;
    bool pinned = false;
};

struct ClothConstraint2D {
    std::size_t p1 = 0;
    std::size_t p2 = 0;
    float restDistance = 1.0f;
};

class Cloth2D {
private:
    std::vector<ClothParticle2D> m_particles;
    std::vector<ClothConstraint2D> m_constraints;

public:
    Cloth2D() = default;

    void createRope(float startX, float startY, float endX, float endY, std::size_t segments) {
        m_particles.clear();
        m_constraints.clear();

        for (std::size_t i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / segments;
            float px = startX + (endX - startX) * t;
            float py = startY + (endY - startY) * t;
            bool isPinned = (i == 0); // Pin top end of rope
            m_particles.push_back({px, py, px, py, isPinned});
        }

        for (std::size_t i = 0; i < segments; ++i) {
            float dx = m_particles[i + 1].x - m_particles[i].x;
            float dy = m_particles[i + 1].y - m_particles[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);
            m_constraints.push_back({i, i + 1, dist});
        }
    }

    void update(float dt, float gravity = 980.0f) {
        // Verlet integration
        for (auto& p : m_particles) {
            if (p.pinned) continue;
            float vx = p.x - p.prevX;
            float vy = p.y - p.prevY;
            p.prevX = p.x;
            p.prevY = p.y;
            p.x += vx;
            p.y += vy + gravity * dt * dt;
        }

        // Satisfy distance constraints
        for (const auto& c : m_constraints) {
            auto& p1 = m_particles[c.p1];
            auto& p2 = m_particles[c.p2];
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 0.0001f) {
                float diff = (dist - c.restDistance) / dist;
                float offX = dx * 0.5f * diff;
                float offY = dy * 0.5f * diff;
                if (!p1.pinned) { p1.x += offX; p1.y += offY; }
                if (!p2.pinned) { p2.x -= offX; p2.y -= offY; }
            }
        }
    }

    std::size_t getParticleCount() const { return m_particles.size(); }
    const ClothParticle2D& getParticle(std::size_t index) const { return m_particles[index]; }
};

} // namespace zenith

#endif // ZENITH_CLOTH2D_H
