#ifndef ZENITH_DEV_OVERLAY_H
#define ZENITH_DEV_OVERLAY_H

#include "zenith_window.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace zenith {

class DebugOverlay {
private:
    float m_fps = 0.0f;
    float m_frameTimeMs = 0.0f;
    std::size_t m_entityCount = 0;
    std::size_t m_drawCalls = 0;
    bool m_visible = true;

public:
    DebugOverlay() = default;

    void toggleVisibility() { m_visible = !m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void updateMetrics(float dt, std::size_t entityCount = 0, std::size_t drawCalls = 0) {
        if (dt > 0.00001f) {
            m_fps = 1.0f / dt;
            m_frameTimeMs = dt * 1000.0f;
        }
        m_entityCount = entityCount;
        m_drawCalls = drawCalls;
    }

    void render(int windowWidth, int windowHeight) {
        if (!m_visible) return;

        // Render debug panel background card in top-left corner
        Renderer2D::drawRect(10.0f, 10.0f, 220.0f, 110.0f, Color{0.0f, 0.0f, 0.0f, 0.75f});
        Renderer2D::drawRectOutline(10.0f, 10.0f, 220.0f, 110.0f, Color::Cyan(), 1.5f);

        // Visual indicator bar for FPS health
        Color barColor = (m_fps >= 55.0f) ? Color::Green() : (m_fps >= 30.0f ? Color::Yellow() : Color::Red());
        float barWidth = (m_fps / 60.0f) * 200.0f;
        if (barWidth > 200.0f) barWidth = 200.0f;
        Renderer2D::drawRect(20.0f, 100.0f, barWidth, 10.0f, barColor);
    }
};

} // namespace zenith

#endif // ZENITH_DEV_OVERLAY_H
