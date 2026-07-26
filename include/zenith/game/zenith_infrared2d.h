#ifndef ZENITH_INFRARED2D_H
#define ZENITH_INFRARED2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class InfraredFilter2D {
public:
    static void applyInfrared(std::vector<uint32_t>& buffer, int width, int height) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                uint32_t p = buffer[idx];

                float r = (p >> 16) & 0xFF;
                float g = (p >> 8) & 0xFF;
                float b = p & 0xFF;
                float luma = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;

                // Infrared thermal color ramp: Black -> Dark Blue -> Purple -> Red -> Yellow -> White
                uint32_t outR = 0, outG = 0, outB = 0;

                if (luma < 0.2f) {
                    outB = static_cast<uint32_t>(luma * 5.0f * 180.0f);
                } else if (luma < 0.4f) {
                    outR = static_cast<uint32_t>((luma - 0.2f) * 5.0f * 150.0f);
                    outB = static_cast<uint32_t>(180.0f - (luma - 0.2f) * 5.0f * 80.0f);
                } else if (luma < 0.7f) {
                    outR = static_cast<uint32_t>(150.0f + (luma - 0.4f) * 3.33f * 105.0f);
                    outG = static_cast<uint32_t>((luma - 0.4f) * 3.33f * 180.0f);
                } else {
                    outR = 255;
                    outG = static_cast<uint32_t>(180.0f + (luma - 0.7f) * 3.33f * 75.0f);
                    outB = static_cast<uint32_t>((luma - 0.7f) * 3.33f * 255.0f);
                }

                outR = std::min(outR, 255u);
                outG = std::min(outG, 255u);
                outB = std::min(outB, 255u);

                buffer[idx] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_INFRARED2D_H
