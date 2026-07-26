#ifndef ZENITH_FLOATING_TEXT_H
#define ZENITH_FLOATING_TEXT_H

#include "zenith_window.h"
#include <vector>
#include <string>

namespace zenith {

struct FloatingText2D {
    std::string text;
    float x = 0.0f;
    float y = 0.0f;
    float vy = -50.0f; // Float upwards
    Color color = Color::Yellow();
    float scale = 1.0f;
    float lifetime = 1.0f;
    float maxLifetime = 1.0f;
};

class FloatingTextManager2D {
private:
    std::vector<FloatingText2D> m_items;

public:
    FloatingTextManager2D() = default;

    void addText(const std::string& text, float x, float y, Color color = Color::Yellow(), float scale = 1.0f, float lifetime = 1.0f) {
        m_items.push_back({text, x, y, -60.0f, color, scale, lifetime, lifetime});
    }

    void spawnDamageNumber(int damage, float x, float y, bool isCritical = false) {
        std::string str = std::to_string(damage);
        if (isCritical) {
            str += "!";
            addText(str, x, y, Color::Red(), 1.5f, 1.2f);
        } else {
            addText(str, x, y, Color::Yellow(), 1.0f, 0.8f);
        }
    }

    void update(float dt) {
        for (auto it = m_items.begin(); it != m_items.end();) {
            it->y += it->vy * dt;
            it->lifetime -= dt;
            if (it->lifetime <= 0.0f) {
                it = m_items.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::size_t getCount() const { return m_items.size(); }
    const FloatingText2D& getItem(std::size_t index) const { return m_items[index]; }
};

} // namespace zenith

#endif // ZENITH_FLOATING_TEXT_H
