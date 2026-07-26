#ifndef ZENITH_FOCUS_VIGNETTE2D_H
#define ZENITH_FOCUS_VIGNETTE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class FocusVignette2D {
public:
    static void applyFocusVignette(
        std::vector<uint32_t>& buffer, int width, int height,
        float focusX = 400.0f, float focusY = 300.0f,
        float innerRadius = 80.0f, float outerRadius = 250.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || outerRadius <= innerRadius) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - focusX;
                float dy = y - focusY;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > innerRadius) {
                    float factor = std::min(1.0f, (dist - innerRadius) / (outerRadius - innerRadius));
                    int idx = y * width + x;
                    uint32_t orig = buffer[idx];

                    float dim = 1.0f - factor * 0.75f;

                    uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) * dim);
                    uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  * dim);
                    uint32_t b = static_cast<uint32_t>((orig & 0xFF)         * dim);

                    buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_FOCUS_VIGNETTE2D_H
