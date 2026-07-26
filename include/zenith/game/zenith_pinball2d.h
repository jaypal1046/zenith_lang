#ifndef ZENITH_PINBALL2D_H
#define ZENITH_PINBALL2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class PinballBumper2D {
private:
    float m_posX;
    float m_posY;
    float m_radius;
    float m_reboundForce = 500.0f;
    float m_flashTimer = 0.0f;

public:
    PinballBumper2D(float x = 100.0f, float y = 100.0f, float r = 15.0f, float force = 500.0f)
        : m_posX(x), m_posY(y), m_radius(r), m_reboundForce(force) {}

    bool checkAndResolveCollision(float ballX, float ballY, float ballR, float& outVelX, float& outVelY) {
        float dx = ballX - m_posX;
        float dy = ballY - m_posY;
        float distSq = dx * dx + dy * dy;
        float minDist = m_radius + ballR;

        if (distSq <= minDist * minDist && distSq > 0.0001f) {
            float dist = std::sqrt(distSq);
            float nx = dx / dist;
            float ny = dy / dist;

            // Reflect & boost velocity outward
            outVelX = nx * m_reboundForce;
            outVelY = ny * m_reboundForce;
            m_flashTimer = 0.2f; // Trigger 200ms flash animation
            return true;
        }
        return false;
    }

    void update(float deltaTime) {
        if (m_flashTimer > 0.0f) {
            m_flashTimer -= deltaTime;
        }
    }

    void drawBumper(
        std::vector<uint32_t>& buffer, int width, int height,
        int cameraOffsetX = 0, int cameraOffsetY = 0) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        int cX = static_cast<int>(m_posX) - cameraOffsetX;
        int cY = static_cast<int>(m_posY) - cameraOffsetY;
        int r = static_cast<int>(m_radius);

        uint32_t fillCol = (m_flashTimer > 0.0f) ? 0xFFFFFF00 : 0xFFFF4500; // Bright Yellow flash / Orange Red idle

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (dx * dx + dy * dy <= r * r) {
                    if (dx * dx + dy * dy >= (r - 2) * (r - 2)) {
                        drawPixel(cX + dx, cY + dy, 0xFFFFFFFF); // White outer rim
                    } else {
                        drawPixel(cX + dx, cY + dy, fillCol);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_PINBALL2D_H
