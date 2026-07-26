#ifndef ZENITH_SDF2D_H
#define ZENITH_SDF2D_H

#include <cmath>
#include <algorithm>

namespace zenith {

class SDF2D {
public:
    static float circleSDF(float px, float py, float cx, float cy, float radius) {
        float dx = px - cx;
        float dy = py - cy;
        return std::sqrt(dx * dx + dy * dy) - radius;
    }

    static float boxSDF(float px, float py, float bx, float by, float width, float height) {
        float dx = std::abs(px - bx) - width * 0.5f;
        float dy = std::abs(py - by) - height * 0.5f;
        float outside = std::sqrt(std::max(dx, 0.0f) * std::max(dx, 0.0f) + std::max(dy, 0.0f) * std::max(dy, 0.0f));
        float inside = std::min(std::max(dx, dy), 0.0f);
        return outside + inside;
    }
};

} // namespace zenith

#endif // ZENITH_SDF2D_H
