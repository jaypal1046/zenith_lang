#ifndef ZENITH_BUILDING_PLACEMENT2D_H
#define ZENITH_BUILDING_PLACEMENT2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

class BuildingPlacement2D {
private:
    std::string m_buildingName;
    int m_gridW = 2; // In tiles
    int m_gridH = 2; // In tiles
    int m_tileSize = 32; // Pixels per tile
    int m_cursorTileX = 0;
    int m_cursorTileY = 0;
    bool m_isValidLocation = true;
    bool m_isPlacing = false;

public:
    void setBuilding(const std::string& name, int gridW, int gridH, int tileSize = 32) {
        m_buildingName = name;
        m_gridW = gridW;
        m_gridH = gridH;
        m_tileSize = tileSize;
        m_isPlacing = true;
    }

    void updateCursor(float worldX, float worldY, bool isValid = true) {
        m_cursorTileX = static_cast<int>(worldX) / m_tileSize;
        m_cursorTileY = static_cast<int>(worldY) / m_tileSize;
        m_isValidLocation = isValid;
    }

    void cancelPlacement() { m_isPlacing = false; }
    bool isPlacing() const { return m_isPlacing; }
    bool isValidLocation() const { return m_isValidLocation; }
    int getTileX() const { return m_cursorTileX; }
    int getTileY() const { return m_cursorTileY; }

    void drawPreview(
        std::vector<uint32_t>& buffer, int width, int height,
        int cameraOffsetX = 0, int cameraOffsetY = 0) const
    {
        if (!m_isPlacing || buffer.empty() || width <= 0 || height <= 0) return;

        uint32_t fillCol = m_isValidLocation ? 0x4000FF00 : 0x40FF0000;   // Green translucent / Red translucent
        uint32_t borderCol = m_isValidLocation ? 0xFF00FF00 : 0xFFFF0000; // Green border / Red border

        int startX = m_cursorTileX * m_tileSize - cameraOffsetX;
        int startY = m_cursorTileY * m_tileSize - cameraOffsetY;
        int endX = startX + m_gridW * m_tileSize;
        int endY = startY + m_gridH * m_tileSize;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                if (x == startX || x == endX - 1 || y == startY || y == endY - 1) {
                    drawPixel(x, y, borderCol);
                } else {
                    drawPixel(x, y, fillCol);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_BUILDING_PLACEMENT2D_H
