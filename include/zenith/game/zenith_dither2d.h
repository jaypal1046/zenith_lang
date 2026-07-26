#ifndef ZENITH_DITHER2D_H
#define ZENITH_DITHER2D_H

#include <vector>
#include <cstdint>
#include <algorithm>

namespace zenith {

class Dither2D {
private:
    static constexpr float bayer4x4[4][4] = {
        { 0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f },
        { 12.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f },
        { 3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f },
        { 15.0f / 16.0f, 7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f }
    };

public:
    static void applyBayerDither(std::vector<uint32_t>& buffer, int width, int height, int levels = 4) {
        if (buffer.empty() || width <= 0 || height <= 0 || levels <= 1) return;

        float step = 255.0f / (levels - 1);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint32_t pixel = buffer[y * width + x];
                float r = static_cast<float>((pixel >> 16) & 0xFF);
                float g = static_cast<float>((pixel >> 8) & 0xFF);
                float b = static_cast<float>(pixel & 0xFF);

                float threshold = bayer4x4[y % 4][x % 4] * step - (step * 0.5f);

                uint32_t dr = static_cast<uint32_t>(std::clamp(std::round((r + threshold) / step) * step, 0.0f, 255.0f));
                uint32_t dg = static_cast<uint32_t>(std::clamp(std::round((g + threshold) / step) * step, 0.0f, 255.0f));
                uint32_t db = static_cast<uint32_t>(std::clamp(std::round((b + threshold) / step) * step, 0.0f, 255.0f));

                buffer[y * width + x] = (0xFF << 24) | (dr << 16) | (dg << 8) | db;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_DITHER2D_H
