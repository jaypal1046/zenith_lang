#ifndef ZENITH_QTE2D_H
#define ZENITH_QTE2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

enum class QTEType {
    ButtonMash,   // Press button rapidly to fill meter
    TimingRing    // Press button when shrinking ring hits target circle
};

class QTE2D {
private:
    QTEType m_type = QTEType::ButtonMash;
    std::string m_buttonPrompt = "E";
    float m_progress = 0.0f; // [0.0, 1.0] for mash / ring radius for timing
    float m_timeRemaining = 3.0f;
    float m_maxTime = 3.0f;
    bool m_isActive = false;
    bool m_isSuccess = false;

public:
    void triggerQTE(QTEType type, const std::string& buttonPrompt, float duration = 3.0f) {
        m_type = type;
        m_buttonPrompt = buttonPrompt;
        m_progress = (type == QTEType::ButtonMash) ? 0.0f : 1.0f; // Ring starts big
        m_timeRemaining = duration;
        m_maxTime = duration;
        m_isActive = true;
        m_isSuccess = false;
    }

    void update(float dt) {
        if (!m_isActive) return;

        m_timeRemaining -= dt;
        if (m_type == QTEType::TimingRing) {
            m_progress = std::max(0.0f, m_progress - (dt / m_maxTime));
        }

        if (m_timeRemaining <= 0.0f) {
            m_isActive = false;
            m_isSuccess = false; // Timed out failure
        }
    }

    bool pressButton() {
        if (!m_isActive) return false;

        if (m_type == QTEType::ButtonMash) {
            m_progress += 0.15f;
            if (m_progress >= 1.0f) {
                m_progress = 1.0f;
                m_isActive = false;
                m_isSuccess = true;
                return true;
            }
        } else if (m_type == QTEType::TimingRing) {
            // Target hit range is [0.15, 0.35]
            if (m_progress >= 0.15f && m_progress <= 0.35f) {
                m_isActive = false;
                m_isSuccess = true;
                return true;
            } else {
                m_isActive = false;
                m_isSuccess = false;
                return false;
            }
        }
        return false;
    }

    bool isActive() const { return m_isActive; }
    bool isSuccess() const { return m_isSuccess; }

    void drawQTE(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = -1, int posY = -1) const
    {
        if (!m_isActive || buffer.empty() || width <= 0 || height <= 0) return;

        int cX = (posX < 0) ? width / 2 : posX;
        int cY = (posY < 0) ? height / 2 : posY;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        if (m_type == QTEType::ButtonMash) {
            // Render central prompt square
            for (int y = cY - 15; y <= cY + 15; ++y) {
                for (int x = cX - 15; x <= cX + 15; ++x) {
                    drawPixel(x, y, 0xFF4169E1);
                }
            }

            // Render progress bar underneath
            int fillW = static_cast<int>(50.0f * m_progress);
            for (int y = cY + 20; y <= cY + 26; ++y) {
                for (int x = cX - 25; x < cX - 25 + fillW; ++x) {
                    drawPixel(x, y, 0xFF00FF00);
                }
            }
        } else if (m_type == QTEType::TimingRing) {
            // Target circle radius 20
            float targetR = 20.0f;
            for (int dy = -20; dy <= 20; ++dy) {
                for (int dx = -20; dx <= 20; ++dx) {
                    if (dx * dx + dy * dy <= 400 && dx * dx + dy * dy >= 324) {
                        drawPixel(cX + dx, cY + dy, 0xFF00FF00); // Green target ring
                    }
                }
            }

            // Shrinking ring radius
            float shrinkR = targetR + 30.0f * m_progress;
            int r = static_cast<int>(shrinkR);
            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    float distSq = dx * dx + dy * dy;
                    if (distSq <= shrinkR * shrinkR && distSq >= (shrinkR - 2.0f) * (shrinkR - 2.0f)) {
                        drawPixel(cX + dx, cY + dy, 0xFFFFD700); // Gold shrinking ring
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_QTE2D_H
