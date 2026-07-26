#ifndef ZENITH_LASER_SCAN2D_H
#define ZENITH_LASER_SCAN2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class LaserScan2D {
public:
    static void applyLaserScan(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float scanSpeed = 2.0f, uint32_t beamColor = 0xFF00FF00, int beamThickness = 4)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        int currentY = static_cast<int>(std::fmod(time * scanSpeed * 100.0f, static_cast<float>(height)));

        for (int dy = -beamThickness; dy <= beamThickness; ++dy) {
            int y = currentY + dy;
            if (y >= 0 && y < height) {
                float intensity = 1.0f - std::abs(dy) / static_cast<float>(beamThickness + 1);

                for (int x = 0; x < width; ++x) {
                    int idx = y * width + x;
                    uint32_t orig = buffer[idx];

                    uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) + ((beamColor >> 16) & 0xFF) * intensity);
                    uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  + ((beamColor >> 8) & 0xFF)  * intensity);
                    uint32_t b = static_cast<uint32_t>((orig & 0xFF)         + (beamColor & 0xFF)         * intensity);

                    r = std::min(r, 255u);
                    g = std::min(g, 255u);
                    b = std::min(b, 255u);

                    buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LASER_SCAN2D_H
