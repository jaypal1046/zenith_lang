#ifndef ZENITH_FOG_OF_WAR_H
#define ZENITH_FOG_OF_WAR_H

#include <vector>
#include <cmath>
#include <algorithm>

namespace zenith {

enum class FogTileState {
    Unexplored = 0,
    Explored = 1,
    Visible = 2
};

class FogOfWar2D {
private:
    int m_cols = 0;
    int m_rows = 0;
    std::vector<uint8_t> m_grid;

public:
    FogOfWar2D(int cols, int rows)
        : m_cols(cols), m_rows(rows), m_grid(cols * rows, static_cast<uint8_t>(FogTileState::Unexplored)) {}

    void updateBeginFrame() {
        // Demote Visible to Explored for new frame vision updates
        for (auto& tile : m_grid) {
            if (tile == static_cast<uint8_t>(FogTileState::Visible)) {
                tile = static_cast<uint8_t>(FogTileState::Explored);
            }
        }
    }

    void reveal(int centerCol, int centerRow, int radiusTiles) {
        int minCol = std::max(0, centerCol - radiusTiles);
        int maxCol = std::min(m_cols - 1, centerCol + radiusTiles);
        int minRow = std::max(0, centerRow - radiusTiles);
        int maxRow = std::min(m_rows - 1, centerRow + radiusTiles);

        float rSq = static_cast<float>(radiusTiles * radiusTiles);

        for (int r = minRow; r <= maxRow; ++r) {
            for (int c = minCol; c <= maxCol; ++c) {
                int dc = c - centerCol;
                int dr = r - centerRow;
                if (static_cast<float>(dc * dc + dr * dr) <= rSq) {
                    m_grid[r * m_cols + c] = static_cast<uint8_t>(FogTileState::Visible);
                }
            }
        }
    }

    FogTileState getTileState(int col, int row) const {
        if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return FogTileState::Unexplored;
        return static_cast<FogTileState>(m_grid[row * m_cols + col]);
    }

    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }
};

} // namespace zenith

#endif // ZENITH_FOG_OF_WAR_H
