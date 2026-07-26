#ifndef ZENITH_TRAIL2D_H
#define ZENITH_TRAIL2D_H

#include <vector>

namespace zenith {

struct GhostSprite2D {
    float x = 0.0f;
    float y = 0.0f;
    float width = 32.0f;
    float height = 32.0f;
    uint32_t color = 0x88FFFFFF;
    float life = 0.5f;      // Remaining lifetime in seconds
    float maxLife = 0.5f;   // Total lifetime
};

class TrailManager2D {
private:
    std::vector<GhostSprite2D> m_ghosts;

public:
    void spawnGhost(float x, float y, float width, float height, uint32_t color, float lifetime = 0.5f) {
        m_ghosts.push_back({x, y, width, height, color, lifetime, lifetime});
    }

    void update(float dt) {
        for (auto it = m_ghosts.begin(); it != m_ghosts.end();) {
            it->life -= dt;
            if (it->life <= 0.0f) {
                it = m_ghosts.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::size_t getGhostCount() const { return m_ghosts.size(); }
    const std::vector<GhostSprite2D>& getGhosts() const { return m_ghosts; }
    void clear() { m_ghosts.clear(); }
};

} // namespace zenith

#endif // ZENITH_TRAIL2D_H
