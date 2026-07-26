#ifndef ZENITH_HEAT_VISION2D_H
#define ZENITH_HEAT_VISION2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class HeatVisionFilter2D {
public:
    static void applyHeatVision(std::vector<uint32_t>& buffer, int width, int height) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t pixel = buffer[i];
            uint32_t r = (pixel >> 16) & 0xFF;
            uint32_t g = (pixel >> 8) & 0xFF;
            uint32_t b = pixel & 0xFF;

            float intensity = (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;

            uint32_t outR = 0, outG = 0, outB = 0;

            if (intensity < 0.25f) {
                // Dark Blue to Blue
                outB = static_cast<uint32_t>(intensity * 4.0f * 255.0f);
            } else if (intensity < 0.5f) {
                // Blue to Cyan/Green
                outG = static_cast<uint32_t>((intensity - 0.25f) * 4.0f * 255.0f);
                outB = static_cast<uint32_t>((0.5f - intensity) * 4.0f * 255.0f);
            } else if (intensity < 0.75f) {
                // Green to Yellow/Red
                outR = static_cast<uint32_t>((intensity - 0.5f) * 4.0f * 255.0f);
                outG = static_cast<uint32_t>(255.0f);
            } else {
                // Yellow to Bright White
                outR = static_cast<uint32_t>(255.0f);
                outG = static_cast<uint32_t>(255.0f);
                outB = static_cast<uint32_t>((intensity - 0.75f) * 4.0f * 255.0f);
            }

            buffer[i] = (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
        }
    }
};

} // namespace zenith

#endif // ZENITH_HEAT_VISION2D_H
