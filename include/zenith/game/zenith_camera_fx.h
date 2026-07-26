#ifndef ZENITH_CAMERA_FX_H
#define ZENITH_CAMERA_FX_H

#include "zenith_window.h"
#include <cmath>
#include <cstdlib>

namespace zenith {

class CameraShake2D {
private:
    float m_intensity = 0.0f;
    float m_duration = 0.0f;
    float m_elapsed = 0.0f;
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;

    float randomRange(float minVal, float maxVal) {
        float scale = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + scale * (maxVal - minVal);
    }

public:
    CameraShake2D() = default;

    void trigger(float intensity = 10.0f, float duration = 0.3f) {
        m_intensity = intensity;
        m_duration = (duration > 0.0f) ? duration : 0.01f;
        m_elapsed = 0.0f;
    }

    void update(float dt, Camera2D& camera) {
        if (m_elapsed < m_duration) {
            m_elapsed += dt;
            float damping = 1.0f - (m_elapsed / m_duration);
            if (damping < 0.0f) damping = 0.0f;

            m_offsetX = randomRange(-m_intensity, m_intensity) * damping;
            m_offsetY = randomRange(-m_intensity, m_intensity) * damping;

            camera.positionX += m_offsetX;
            camera.positionY += m_offsetY;
        } else {
            m_offsetX = 0.0f;
            m_offsetY = 0.0f;
        }
    }

    bool isShaking() const { return m_elapsed < m_duration; }
    float getOffsetX() const { return m_offsetX; }
    float getOffsetY() const { return m_offsetY; }
};

} // namespace zenith

#endif // ZENITH_CAMERA_FX_H
