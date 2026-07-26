#ifndef ZENITH_MARCHING_CUBES2D_H
#define ZENITH_MARCHING_CUBES2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct LineSegment2D {
    float x1, y1;
    float x2, y2;
};

class MarchingCubes2D {
public:
    // Marching squares 2D contour generator for smooth cave maps
    static std::vector<LineSegment2D> generateContours(const std::vector<float>& grid, int cols, int rows, float isoValue = 0.5f) {
        std::vector<LineSegment2D> segments;

        auto sample = [&](int c, int r) -> float {
            if (c < 0 || c >= cols || r < 0 || r >= rows) return 0.0f;
            return grid[r * cols + c];
        };

        for (int r = 0; r < rows - 1; ++r) {
            for (int c = 0; c < cols - 1; ++c) {
                float v0 = sample(c, r);         // Top-left
                float v1 = sample(c + 1, r);     // Top-right
                float v2 = sample(c + 1, r + 1); // Bottom-right
                float v3 = sample(c, r + 1);     // Bottom-left

                int mask = 0;
                if (v0 >= isoValue) mask |= 1;
                if (v1 >= isoValue) mask |= 2;
                if (v2 >= isoValue) mask |= 4;
                if (v3 >= isoValue) mask |= 8;

                float fc = static_cast<float>(c);
                float fr = static_cast<float>(r);

                // Simple mid-edge segment generation for active cell mask
                if (mask > 0 && mask < 15) {
                    segments.push_back({fc + 0.5f, fr, fc + 0.5f, fr + 1.0f});
                }
            }
        }
        return segments;
    }
};

} // namespace zenith

#endif // ZENITH_MARCHING_CUBES2D_H
