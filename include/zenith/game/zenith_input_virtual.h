#ifndef ZENITH_INPUT_VIRTUAL_H
#define ZENITH_INPUT_VIRTUAL_H

#include "zenith_window.h"
#include <cmath>

namespace zenith {

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

class VirtualJoystick {
private:
    float m_centerX = 0.0f;
    float m_centerY = 0.0f;
    float m_radius = 60.0f;
    float m_knobRadius = 25.0f;
    Vector2 m_axis{0.0f, 0.0f};
    bool m_active = false;

public:
    VirtualJoystick() = default;
    VirtualJoystick(float x, float y, float radius = 60.0f)
        : m_centerX(x), m_centerY(y), m_radius(radius) {}

    void update() {
        float touchX = Input::getMouseX();
        float touchY = Input::getMouseY();
        bool isDown = Input::isMouseButtonDown(MouseButton::Left);

        if (isDown) {
            float dx = touchX - m_centerX;
            float dy = touchY - m_centerY;
            float dist = sqrtf(dx * dx + dy * dy);

            if (!m_active && dist <= m_radius * 1.5f) {
                m_active = true;
            }

            if (m_active) {
                if (dist > 0.001f) {
                    float clampDist = std::min(dist, m_radius);
                    m_axis.x = (dx / dist) * (clampDist / m_radius);
                    m_axis.y = (dy / dist) * (clampDist / m_radius);
                } else {
                    m_axis = {0.0f, 0.0f};
                }
            }
        } else {
            m_active = false;
            m_axis = {0.0f, 0.0f};
        }
    }

    void render() {
        // Base outer ring
        Renderer2D::drawCircle(m_centerX, m_centerY, m_radius, Color{0.2f, 0.2f, 0.2f, 0.5f});
        Renderer2D::drawCircle(m_centerX, m_centerY, m_radius, Color{1.0f, 1.0f, 1.0f, 0.3f});

        // Inner knob
        float knobX = m_centerX + m_axis.x * m_radius;
        float knobY = m_centerY + m_axis.y * m_radius;
        Color knobColor = m_active ? Color::Cyan() : Color::White();
        Renderer2D::drawCircle(knobX, knobY, m_knobRadius, knobColor);
    }

    Vector2 getAxis() const { return m_axis; }
    bool isActive() const { return m_active; }
};

} // namespace zenith

#endif // ZENITH_INPUT_VIRTUAL_H
