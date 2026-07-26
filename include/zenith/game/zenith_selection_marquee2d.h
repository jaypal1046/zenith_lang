#ifndef ZENITH_SELECTION_MARQUEE2D_H
#define ZENITH_SELECTION_MARQUEE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct MarqueeRect {
    float startX, startY;
    float currentX, currentY;
    bool isDragging;
};

class SelectionMarquee2D {
private:
    MarqueeRect m_rect = { 0, 0, 0, 0, false };

public:
    void startDrag(float x, float y) {
        m_rect.startX = x;
        m_rect.startY = y;
        m_rect.currentX = x;
        m_rect.currentY = y;
        m_rect.isDragging = true;
    }

    void updateDrag(float x, float y) {
        if (m_rect.isDragging) {
            m_rect.currentX = x;
            m_rect.currentY = y;
        }
    }

    void endDrag() {
        m_rect.isDragging = false;
    }

    bool isDragging() const { return m_rect.isDragging; }

    bool containsPoint(float x, float y) const {
        float minX = std::min(m_rect.startX, m_rect.currentX);
        float maxX = std::max(m_rect.startX, m_rect.currentX);
        float minY = std::min(m_rect.startY, m_rect.currentY);
        float maxY = std::max(m_rect.startY, m_rect.currentY);
        return (x >= minX && x <= maxX && y >= minY && y <= maxY);
    }

    void drawMarquee(
        std::vector<uint32_t>& buffer, int width, int height,
        uint32_t borderCol = 0xFF00FF00, uint32_t fillCol = 0x3000FF00) const
    {
        if (!m_rect.isDragging || buffer.empty() || width <= 0 || height <= 0) return;

        int minX = std::clamp(static_cast<int>(std::min(m_rect.startX, m_rect.currentX)), 0, width - 1);
        int maxX = std::clamp(static_cast<int>(std::max(m_rect.startX, m_rect.currentX)), 0, width - 1);
        int minY = std::clamp(static_cast<int>(std::min(m_rect.startY, m_rect.currentY)), 0, height - 1);
        int maxY = std::clamp(static_cast<int>(std::max(m_rect.startY, m_rect.currentY)), 0, height - 1);

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (x == minX || x == maxX || y == minY || y == maxY) {
                    drawPixel(x, y, borderCol);
                } else {
                    drawPixel(x, y, fillCol);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SELECTION_MARQUEE2D_H
