#ifndef ZENITH_PARTICLES_H
#define ZENITH_PARTICLES_H

#include "zenith_window.h"
#include <vector>
#include <cstdlib>
#include <cmath>

namespace zenith {

struct Particle2D {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float size = 4.0f;
    float life = 1.0f;      // Remaining life in seconds
    float maxLife = 1.0f;   // Total lifetime
    Color color = Color::Yellow();
    bool active = false;
};

class ParticleEmitter2D {
private:
    std::vector<Particle2D> m_particles;
    std::size_t m_maxParticles = 500;

    float randomFloat(float minVal, float maxVal) {
        float scale = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + scale * (maxVal - minVal);
    }

public:
    explicit ParticleEmitter2D(std::size_t maxParticles = 500) : m_maxParticles(maxParticles) {
        m_particles.resize(m_maxParticles);
    }

    void burst(float x, float y, int count = 20, const Color& color = Color::Yellow(), float minSpeed = 50.0f, float maxSpeed = 150.0f) {
        int spawned = 0;
        for (auto& p : m_particles) {
            if (!p.active) {
                p.x = x;
                p.y = y;
                float angle = randomFloat(0.0f, 6.2831853f);
                float speed = randomFloat(minSpeed, maxSpeed);
                p.vx = cosf(angle) * speed;
                p.vy = sinf(angle) * speed;
                p.size = randomFloat(3.0f, 8.0f);
                p.maxLife = randomFloat(0.3f, 1.2f);
                p.life = p.maxLife;
                p.color = color;
                p.active = true;

                spawned++;
                if (spawned >= count) break;
            }
        }
    }

    void update(float dt, float gravityY = 100.0f) {
        for (auto& p : m_particles) {
            if (p.active) {
                p.life -= dt;
                if (p.life <= 0.0f) {
                    p.active = false;
                    continue;
                }
                p.vy += gravityY * dt;
                p.x += p.vx * dt;
                p.y += p.vy * dt;
            }
        }
    }

    void render() {
        for (const auto& p : m_particles) {
            if (p.active) {
                float alpha = p.life / p.maxLife;
                Color renderColor = p.color;
                renderColor.a *= alpha;

                Renderer2D::drawCircle(p.x, p.y, p.size * alpha, renderColor);
            }
        }
    }

    std::size_t getActiveCount() const {
        std::size_t count = 0;
        for (const auto& p : m_particles) {
            if (p.active) count++;
        }
        return count;
    }
};

} // namespace zenith

#endif // ZENITH_PARTICLES_H
