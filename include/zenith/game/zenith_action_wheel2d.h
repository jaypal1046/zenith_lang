#ifndef ZENITH_ACTION_WHEEL2D_H
#define ZENITH_ACTION_WHEEL2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct WheelSector {
    std::string name;
    uint32_t iconColor;
};

class ActionWheel2D {
private:
    std::vector<WheelSector> m_sectors;

public:
    void addSector(const std::string& name, uint32_t iconColor) {
        m_sectors.push_back({ name, iconColor });
    }

    int getSelectedSector(float mouseX, float mouseY, int centerX, int centerY) const {
        if (m_sectors.empty()) return -1;
        float dx = mouseX - centerX;
        float dy = mouseY - centerY;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 15.0f) return -1; // Deadzone center

        float angle = std::atan2(dy, dx); // [-PI, PI]
        if (angle < 0) angle += 6.2831853f;

        float sectorAngle = 6.2831853f / m_sectors.size();
        int idx = static_cast<int>(angle / sectorAngle);
        return std::clamp(idx, 0, static_cast<int>(m_sectors.size()) - 1);
    }

    void drawWheel(
        std::vector<uint32_t>& buffer, int width, int height,
        float mouseX, float mouseY, int centerX = -1, int centerY = -1, float radius = 70.0f) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_sectors.empty()) return;

        int cX = (centerX < 0) ? width / 2 : centerX;
        int cY = (centerY < 0) ? height / 2 : centerY;

        int selectedIdx = getSelectedSector(mouseX, mouseY, cX, cY);

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render circular outer wheel base
        int r = static_cast<int>(radius);
        float sectorAngle = 6.2831853f / m_sectors.size();

        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                float distSq = dx * dx + dy * dy;
                if (distSq <= radius * radius && distSq >= 15.0f * 15.0f) {
                    float angle = std::atan2(static_cast<float>(dy), static_cast<float>(dx));
                    if (angle < 0) angle += 6.2831853f;

                    int sIdx = std::clamp(static_cast<int>(angle / sectorAngle), 0, static_cast<int>(m_sectors.size()) - 1);
                    uint32_t col = (sIdx == selectedIdx) ? 0xFFFFD700 : m_sectors[sIdx].iconColor;

                    drawPixel(cX + dx, cY + dy, col);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_ACTION_WHEEL2D_H
