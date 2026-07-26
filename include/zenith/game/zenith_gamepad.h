#ifndef ZENITH_GAMEPAD_H
#define ZENITH_GAMEPAD_H

#include <cmath>
#include <algorithm>

namespace zenith {

enum class GamepadButton {
    A,
    B,
    X,
    Y,
    LeftBumper,
    RightBumper,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    Start,
    Select
};

class Gamepad {
private:
    float m_axisLeftX = 0.0f;
    float m_axisLeftY = 0.0f;
    float m_axisRightX = 0.0f;
    float m_axisRightY = 0.0f;
    float m_triggerLeft = 0.0f;
    float m_triggerRight = 0.0f;
    bool m_connected = false;

public:
    Gamepad() = default;

    void updateAxis(float leftX, float leftY, float rightX, float rightY) {
        m_axisLeftX = (fabsf(leftX) > 0.15f) ? leftX : 0.0f;  // Deadzone filter
        m_axisLeftY = (fabsf(leftY) > 0.15f) ? leftY : 0.0f;
        m_axisRightX = (fabsf(rightX) > 0.15f) ? rightX : 0.0f;
        m_axisRightY = (fabsf(rightY) > 0.15f) ? rightY : 0.0f;
        m_connected = true;
    }

    float getAxisLeftX() const { return m_axisLeftX; }
    float getAxisLeftY() const { return m_axisLeftY; }
    float getAxisRightX() const { return m_axisRightX; }
    float getAxisRightY() const { return m_axisRightY; }
    bool isConnected() const { return m_connected; }
};

} // namespace zenith

#endif // ZENITH_GAMEPAD_H
