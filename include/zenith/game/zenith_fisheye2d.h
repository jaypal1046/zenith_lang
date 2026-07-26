#ifndef ZENITH_FISHEYE2D_H
#define ZENITH_FISHEYE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class FisheyeFilter2D {
public:
    static void applyFisheye(std::vector<uint32_t>& buffer, int width, int height, float power = 1.2f) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;
        float maxRadius = std::sqrt(centerX * centerX + centerY * centerY);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float normX = (x - centerX) / maxRadius;
                float normY = (y - centerY) / maxRadius;
                float r = std::sqrt(normX * normX + normY * normY);

                if (r > 0.001f) {
                    float theta = std::atan2(normY, normX);
                    float newR = std::pow(r, power);
                    int srcX = std::clamp(static_cast<int>(centerX + std::cos(theta) * newR * maxRadius), 0, width - 1);
                    int srcY = std::clamp(static_cast<int>(centerY + std::sin(theta) * newR * maxRadius), 0, height - 1);

                    buffer[y * width + x] = original[srcY * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_FISHEYE2D_H
