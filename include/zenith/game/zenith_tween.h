#ifndef ZENITH_TWEEN_H
#define ZENITH_TWEEN_H

#include <cmath>
#include <functional>

namespace zenith {

enum class EaseType {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseOutBounce,
    EaseOutElastic
};

class Easing {
public:
    static float evaluate(float t, EaseType type = EaseType::Linear) {
        if (t <= 0.0f) return 0.0f;
        if (t >= 1.0f) return 1.0f;

        switch (type) {
            case EaseType::Linear:
                return t;
            case EaseType::EaseInQuad:
                return t * t;
            case EaseType::EaseOutQuad:
                return t * (2.0f - t);
            case EaseType::EaseInOutQuad:
                return (t < 0.5f) ? (2.0f * t * t) : (-1.0f + (4.0f - 2.0f * t) * t);
            case EaseType::EaseOutBounce: {
                const float n1 = 7.5625f;
                const float d1 = 2.75f;
                if (t < 1.0f / d1) {
                    return n1 * t * t;
                } else if (t < 2.0f / d1) {
                    t -= 1.5f / d1;
                    return n1 * t * t + 0.75f;
                } else if (t < 2.5f / d1) {
                    t -= 2.25f / d1;
                    return n1 * t * t + 0.9375f;
                } else {
                    t -= 2.625f / d1;
                    return n1 * t * t + 0.984375f;
                }
            }
            case EaseType::EaseOutElastic: {
                const float c4 = (2.0f * 3.14159265f) / 3.0f;
                return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
            }
            default:
                return t;
        }
    }
};

class TweenFloat {
private:
    float m_start = 0.0f;
    float m_target = 0.0f;
    float m_duration = 1.0f;
    float m_elapsed = 0.0f;
    EaseType m_ease = EaseType::Linear;
    bool m_active = false;
    std::function<void()> m_onComplete = nullptr;

public:
    TweenFloat() = default;

    void start(float startVal, float targetVal, float duration, EaseType ease = EaseType::Linear, std::function<void()> onComplete = nullptr) {
        m_start = startVal;
        m_target = targetVal;
        m_duration = (duration > 0.0001f) ? duration : 0.0001f;
        m_elapsed = 0.0f;
        m_ease = ease;
        m_active = true;
        m_onComplete = onComplete;
    }

    void update(float dt) {
        if (!m_active) return;

        m_elapsed += dt;
        if (m_elapsed >= m_duration) {
            m_elapsed = m_duration;
            m_active = false;
            if (m_onComplete) m_onComplete();
        }
    }

    float getValue() const {
        float progress = m_elapsed / m_duration;
        float easedProgress = Easing::evaluate(progress, m_ease);
        return m_start + (m_target - m_start) * easedProgress;
    }

    bool isActive() const { return m_active; }
};

} // namespace zenith

#endif // ZENITH_TWEEN_H
