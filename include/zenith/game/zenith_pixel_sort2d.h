#ifndef ZENITH_PIXEL_SORT2D_H
#define ZENITH_PIXEL_SORT2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class PixelSort2D {
public:
    static void applyPixelSort(std::vector<uint32_t>& buffer, int width, int height, float lumaThreshold = 0.4f) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        auto getLuma = [](uint32_t p) {
            float r = (p >> 16) & 0xFF;
            float g = (p >> 8) & 0xFF;
            float b = p & 0xFF;
            return (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
        };

        for (int y = 0; y < height; ++y) {
            int xStart = 0;
            while (xStart < width) {
                while (xStart < width && getLuma(buffer[y * width + xStart]) < lumaThreshold) {
                    xStart++;
                }

                int xEnd = xStart;
                while (xEnd < width && getLuma(buffer[y * width + xEnd]) >= lumaThreshold) {
                    xEnd++;
                }

                if (xEnd > xStart + 1) {
                    std::sort(
                        buffer.begin() + y * width + xStart,
                        buffer.begin() + y * width + xEnd,
                        [&](uint32_t a, uint32_t b) { return getLuma(a) < getLuma(b); }
                    );
                }

                xStart = xEnd + 1;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_PIXEL_SORT2D_H
