#ifndef ZENITH_DESTRUCTIBLE_TERRAIN_H
#define ZENITH_DESTRUCTIBLE_TERRAIN_H

#include <vector>
#include <cmath>

namespace zenith {

class DestructibleTerrain2D {
private:
    int m_width = 0;
    int m_height = 0;
    std::vector<uint8_t> m_grid;

public:
    DestructibleTerrain2D(int width, int height, bool fill = true)
        : m_width(width), m_height(height), m_grid(width * height, fill ? 1 : 0) {}

    void carveCircle(int centerX, int centerY, int radius) {
        int rSq = radius * radius;
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx * dx + dy * dy <= rSq) {
                    int px = centerX + dx;
                    int py = centerY + dy;
                    if (px >= 0 && px < m_width && py >= 0 && py < m_height) {
                        m_grid[py * m_width + px] = 0;
                    }
                }
            }
        }
    }

    bool isSolid(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return false;
        return m_grid[y * m_width + x] != 0;
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
};

} // namespace zenith

#endif // ZENITH_DESTRUCTIBLE_TERRAIN_H
