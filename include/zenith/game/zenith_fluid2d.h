#ifndef ZENITH_FLUID2D_H
#define ZENITH_FLUID2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct FluidParticle2D {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float density = 0.0f;
    float pressure = 0.0f;
};

class FluidSimulation2D {
private:
    std::vector<FluidParticle2D> m_particles;
    float m_smoothingRadius = 20.0f;
    float m_restDensity = 1.0f;
    float m_stiffness = 200.0f;

public:
    FluidSimulation2D() = default;

    void addParticle(float x, float y, float vx = 0.0f, float vy = 0.0f) {
        m_particles.push_back({x, y, vx, vy, 0.0f, 0.0f});
    }

    void update(float dt, float gravity = 980.0f) {
        float hSq = m_smoothingRadius * m_smoothingRadius;

        // Calculate densities & pressures
        for (auto& pi : m_particles) {
            pi.density = 0.0f;
            for (const auto& pj : m_particles) {
                float dx = pj.x - pi.x;
                float dy = pj.y - pi.y;
                float rSq = dx * dx + dy * dy;
                if (rSq < hSq) {
                    pi.density += (hSq - rSq) * (hSq - rSq);
                }
            }
            if (pi.density < 0.0001f) pi.density = 0.0001f;
            pi.pressure = m_stiffness * (pi.density - m_restDensity);
        }

        // Integrate positions
        for (auto& p : m_particles) {
            p.vy += gravity * dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
        }
    }

    std::size_t getParticleCount() const { return m_particles.size(); }
    const FluidParticle2D& getParticle(std::size_t index) const { return m_particles[index]; }
};

} // namespace zenith

#endif // ZENITH_FLUID2D_H
