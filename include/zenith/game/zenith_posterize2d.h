#ifndef ZENITH_POSTERIZE2D_H
#define ZENITH_POSTERIZE2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class PosterizeFilter2D {
public:
    static void applyPosterize(std::vector<uint32_t>& buffer, int width, int height, int levels = 4) {
        if (buffer.empty() || width <= 0 || height <= 0 || levels <= 1) return;

        float step = 255.0f / (levels - 1);

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t pixel = buffer[i];
            uint32_t r = (pixel >> 16) & 0xFF;
            uint32_t g = (pixel >> 8) & 0xFF;
            uint32_t b = pixel & 0xFF;

            r = static_cast<uint32_t>(std::round(r / step) * step);
            g = static_cast<uint32_t>(std::round(g / step) * step);
            b = static_cast<uint32_t>(std::round(b / step) * step);

            buffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
};

} // namespace zenith

#endif // ZENITH_POSTERIZE2D_H
