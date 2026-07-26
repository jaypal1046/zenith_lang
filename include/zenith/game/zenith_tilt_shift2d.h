#ifndef ZENITH_TILT_SHIFT2D_H
#define ZENITH_TILT_SHIFT2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "zenith_blur2d.h"

namespace zenith {

class TiltShiftFilter2D {
public:
    static void applyTiltShift(
        std::vector<uint32_t>& buffer, int width, int height,
        float focusCenterY = 0.5f, float focusBandHeight = 0.2f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> blurred = buffer;
        BlurPass2D::applyBoxBlur(blurred, width, height, 3);

        float centerPixelY = focusCenterY * height;
        float bandHalf = (focusBandHeight * height) * 0.5f;

        for (int y = 0; y < height; ++y) {
            float distFromBand = std::abs(y - centerPixelY) - bandHalf;
            if (distFromBand > 0.0f) {
                float blurFactor = std::clamp(distFromBand / (height * 0.3f), 0.0f, 1.0f);

                for (int x = 0; x < width; ++x) {
                    int idx = y * width + x;
                    uint32_t orig = buffer[idx];
                    uint32_t blur = blurred[idx];

                    uint32_t r = static_cast<uint32_t>(((orig >> 16) & 0xFF) * (1.0f - blurFactor) + ((blur >> 16) & 0xFF) * blurFactor);
                    uint32_t g = static_cast<uint32_t>(((orig >> 8) & 0xFF)  * (1.0f - blurFactor) + ((blur >> 8) & 0xFF)  * blurFactor);
                    uint32_t b = static_cast<uint32_t>((orig & 0xFF)         * (1.0f - blurFactor) + (blur & 0xFF)         * blurFactor);

                    buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_TILT_SHIFT2D_H
