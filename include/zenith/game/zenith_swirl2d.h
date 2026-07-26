#ifndef ZENITH_SWIRL2D_H
#define ZENITH_SWIRL2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class SwirlFilter2D {
public:
    static void applySwirl(
        std::vector<uint32_t>& buffer, int width, int height,
        float radius = 150.0f, float swirlAngle = 3.14159265f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || radius <= 0.0f) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < radius) {
                    float factor = (radius - dist) / radius;
                    float theta = factor * factor * swirlAngle;

                    float sinT = std::sin(theta);
                    float cosT = std::cos(theta);

                    int srcX = std::clamp(static_cast<int>(centerX + dx * cosT - dy * sinT), 0, width - 1);
                    int srcY = std::clamp(static_cast<int>(centerY + dx * sinT + dy * cosT), 0, height - 1);

                    buffer[y * width + x] = original[srcY * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SWIRL2D_H
