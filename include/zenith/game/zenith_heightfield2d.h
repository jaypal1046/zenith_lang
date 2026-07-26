#ifndef ZENITH_HEIGHTFIELD2D_H
#define ZENITH_HEIGHTFIELD2D_H

#include <vector>
#include <cmath>
#include <algorithm>

namespace zenith {

class Heightfield2D {
private:
    int m_cols = 0;
    int m_rows = 0;
    std::vector<float> m_heights;

public:
    Heightfield2D(int cols, int rows)
        : m_cols(cols), m_rows(rows), m_heights(cols * rows, 0.0f) {}

    void setHeight(int col, int row, float height) {
        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            m_heights[row * m_cols + col] = height;
        }
    }

    float getHeight(int col, int row) const {
        if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return 0.0f;
        return m_heights[row * m_cols + col];
    }

    float calculateSlope(int col, int row) const {
        float hCenter = getHeight(col, row);
        float hRight = getHeight(col + 1, row);
        float hDown = getHeight(col, row + 1);
        float dx = hRight - hCenter;
        float dy = hDown - hCenter;
        return std::sqrt(dx * dx + dy * dy);
    }

    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }
};

} // namespace zenith

#endif // ZENITH_HEIGHTFIELD2D_H
