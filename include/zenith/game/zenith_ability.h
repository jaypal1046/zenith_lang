#ifndef ZENITH_ABILITY_H
#define ZENITH_ABILITY_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace zenith {

struct GameplayEffect {
    std::string id;
    float duration = 5.0f; // Total duration in seconds (-1 = infinite)
    float elapsedTime = 0.0f;
    float tickInterval = 1.0f;
    float timeSinceLastTick = 0.0f;
    int stackCount = 1;
    bool isExpired = false;

    std::function<void()> onApply;
    std::function<void()> onTick;
    std::function<void()> onRemove;
};

class AbilityContainer {
private:
    std::vector<GameplayEffect> m_activeEffects;

public:
    AbilityContainer() = default;

    void applyEffect(const GameplayEffect& effect) {
        auto it = std::find_if(m_activeEffects.begin(), m_activeEffects.end(), [&](const GameplayEffect& e) {
            return e.id == effect.id;
        });

        if (it != m_activeEffects.end()) {
            it->elapsedTime = 0.0f; // Reset duration on re-apply
            it->stackCount++;
        } else {
            m_activeEffects.push_back(effect);
            if (effect.onApply) effect.onApply();
        }
    }

    void update(float dt) {
        for (auto& effect : m_activeEffects) {
            if (effect.isExpired) continue;

            effect.elapsedTime += dt;
            effect.timeSinceLastTick += dt;

            if (effect.onTick && effect.timeSinceLastTick >= effect.tickInterval) {
                effect.timeSinceLastTick = 0.0f;
                effect.onTick();
            }

            if (effect.duration > 0.0f && effect.elapsedTime >= effect.duration) {
                effect.isExpired = true;
                if (effect.onRemove) effect.onRemove();
            }
        }

        m_activeEffects.erase(
            std::remove_if(m_activeEffects.begin(), m_activeEffects.end(), [](const GameplayEffect& e) {
                return e.isExpired;
            }),
            m_activeEffects.end()
        );
    }

    bool hasEffect(const std::string& id) const {
        return std::any_of(m_activeEffects.begin(), m_activeEffects.end(), [&](const GameplayEffect& e) {
            return e.id == id && !e.isExpired;
        });
    }

    std::size_t getEffectCount() const { return m_activeEffects.size(); }
};

} // namespace zenith

#endif // ZENITH_ABILITY_H
