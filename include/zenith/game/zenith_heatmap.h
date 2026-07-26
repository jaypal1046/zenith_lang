#ifndef ZENITH_HEATMAP_H
#define ZENITH_HEATMAP_H

#include <vector>
#include <algorithm>

namespace zenith {

class Heatmap2D {
private:
    int m_cols = 0;
    int m_rows = 0;
    std::vector<float> m_grid;

public:
    Heatmap2D(int cols, int rows)
        : m_cols(cols), m_rows(rows), m_grid(cols * rows, 0.0f) {}

    void addHeat(int col, int row, float amount) {
        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            m_grid[row * m_cols + col] += amount;
        }
    }

    void decay(float factor = 0.95f) {
        for (auto& val : m_grid) {
            val *= factor;
            if (val < 0.001f) val = 0.0f;
        }
    }

    float getHeat(int col, int row) const {
        if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return 0.0f;
        return m_grid[row * m_cols + col];
    }

    void clear() {
        std::fill(m_grid.begin(), m_grid.end(), 0.0f);
    }

    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }
};

} // namespace zenith

#endif // ZENITH_HEATMAP_H
