#ifndef ZENITH_TACTICAL_GRID2D_H
#define ZENITH_TACTICAL_GRID2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

enum class CoverType {
    None,
    Half, // Shield half icon
    Full  // Shield full icon
};

class TacticalGrid2D {
private:
    int m_gridWidth = 10;
    int m_gridHeight = 10;
    int m_tileSize = 32;
    std::vector<CoverType> m_coverMap;

public:
    TacticalGrid2D(int gW = 10, int gH = 10, int tileSize = 32)
        : m_gridWidth(gW), m_gridHeight(gH), m_tileSize(tileSize),
          m_coverMap(gW * gH, CoverType::None) {}

    void setCover(int tileX, int tileY, CoverType cover) {
        if (tileX >= 0 && tileX < m_gridWidth && tileY >= 0 && tileY < m_gridHeight) {
            m_coverMap[tileY * m_gridWidth + tileX] = cover;
        }
    }

    void drawTacticalGrid(
        std::vector<uint32_t>& buffer, int width, int height,
        int unitTileX, int unitTileY, int moveRange = 3,
        int cameraOffsetX = 0, int cameraOffsetY = 0) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Highlight valid Manhattan distance movement tiles in blue
        for (int y = 0; y < m_gridHeight; ++y) {
            for (int x = 0; x < m_gridWidth; ++x) {
                int dist = std::abs(x - unitTileX) + std::abs(y - unitTileY);
                if (dist > 0 && dist <= moveRange) {
                    int startX = x * m_tileSize - cameraOffsetX;
                    int startY = y * m_tileSize - cameraOffsetY;

                    for (int dy = 0; dy < m_tileSize; ++dy) {
                        for (int dx = 0; dx < m_tileSize; ++dx) {
                            if (dx == 0 || dx == m_tileSize - 1 || dy == 0 || dy == m_tileSize - 1) {
                                drawPixel(startX + dx, startY + dy, 0xFF1E90FF); // Blue border
                            } else {
                                drawPixel(startX + dx, startY + dy, 0x301E90FF); // Translucent blue fill
                            }
                        }
                    }
                }

                // Draw cover icons
                CoverType cover = m_coverMap[y * m_gridWidth + x];
                if (cover != CoverType::None) {
                    int cX = x * m_tileSize + m_tileSize / 2 - cameraOffsetX;
                    int cY = y * m_tileSize + m_tileSize / 2 - cameraOffsetY;
                    uint32_t covCol = (cover == CoverType::Full) ? 0xFFFFD700 : 0xFFC0C0C0;

                    for (int dy = -4; dy <= 4; ++dy) {
                        for (int dx = -4; dx <= 4; ++dx) {
                            drawPixel(cX + dx, cY + dy, covCol);
                        }
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_TACTICAL_GRID2D_H
