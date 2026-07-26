#ifndef ZENITH_KALEIDOSCOPE2D_H
#define ZENITH_KALEIDOSCOPE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class KaleidoscopeFilter2D {
public:
    static void applyKaleidoscope(std::vector<uint32_t>& buffer, int width, int height, int segments = 6) {
        if (buffer.empty() || width <= 0 || height <= 0 || segments < 2) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;
        float segmentAngle = 6.28318530718f / static_cast<float>(segments);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;

                float radius = std::sqrt(dx * dx + dy * dy);
                float angle = std::atan2(dy, dx);

                if (angle < 0.0f) angle += 6.28318530718f;

                float sector = std::fmod(angle, segmentAngle);
                if (static_cast<int>(angle / segmentAngle) % 2 == 1) {
                    sector = segmentAngle - sector; // Mirror alternate sectors
                }

                int srcX = std::clamp(static_cast<int>(centerX + radius * std::cos(sector)), 0, width - 1);
                int srcY = std::clamp(static_cast<int>(centerY + radius * std::sin(sector)), 0, height - 1);

                buffer[y * width + x] = original[srcY * width + srcX];
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_KALEIDOSCOPE2D_H
