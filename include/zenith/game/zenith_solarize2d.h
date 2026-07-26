#ifndef ZENITH_SOLARIZE2D_H
#define ZENITH_SOLARIZE2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class SolarizeFilter2D {
public:
    static void applySolarize(std::vector<uint32_t>& buffer, int width, int height, float threshold = 0.5f) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        uint32_t threshVal = static_cast<uint32_t>(threshold * 255.0f);

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t pixel = buffer[i];
            uint32_t r = (pixel >> 16) & 0xFF;
            uint32_t g = (pixel >> 8) & 0xFF;
            uint32_t b = pixel & 0xFF;

            if (r > threshVal) r = 255 - r;
            if (g > threshVal) g = 255 - g;
            if (b > threshVal) b = 255 - b;

            buffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
};

} // namespace zenith

#endif // ZENITH_SOLARIZE2D_H
