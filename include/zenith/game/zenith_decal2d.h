#ifndef ZENITH_DECAL2D_H
#define ZENITH_DECAL2D_H

#include "zenith_window.h"
#include <vector>

namespace zenith {

struct Decal2D {
    float x = 0.0f;
    float y = 0.0f;
    float width = 16.0f;
    float height = 16.0f;
    float rotation = 0.0f;
    float opacity = 1.0f;
    Color color = Color::White();
    float lifetime = 30.0f; // Seconds before fading
};

class DecalManager2D {
private:
    std::vector<Decal2D> m_decals;
    std::size_t m_maxDecals = 200;

public:
    DecalManager2D(std::size_t maxDecals = 200) : m_maxDecals(maxDecals) {}

    void addDecal(float x, float y, float width, float height, Color color = Color::White(), float lifetime = 30.0f) {
        if (m_decals.size() >= m_maxDecals) {
            m_decals.erase(m_decals.begin()); // Reuse oldest decal slot
        }
        m_decals.push_back({x, y, width, height, 0.0f, 1.0f, color, lifetime});
    }

    void update(float dt) {
        for (auto it = m_decals.begin(); it != m_decals.end();) {
            it->lifetime -= dt;
            if (it->lifetime <= 0.0f) {
                it = m_decals.erase(it);
            } else {
                if (it->lifetime < 2.0f) {
                    it->opacity = it->lifetime / 2.0f; // Fade out in last 2 seconds
                }
                ++it;
            }
        }
    }

    void clear() { m_decals.clear(); }
    std::size_t getDecalCount() const { return m_decals.size(); }
    const Decal2D& getDecal(std::size_t index) const { return m_decals[index]; }
};

} // namespace zenith

#endif // ZENITH_DECAL2D_H
