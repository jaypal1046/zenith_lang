#ifndef ZENITH_RADIAL_BLUR2D_H
#define ZENITH_RADIAL_BLUR2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class RadialBlurFilter2D {
public:
    static void applyRadialBlur(
        std::vector<uint32_t>& buffer, int width, int height,
        float originX = 0.5f, float originY = 0.5f, int samples = 5, float strength = 0.05f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || samples <= 1 || strength <= 0.0f) return;

        std::vector<uint32_t> original = buffer;
        float centerX = originX * width;
        float centerY = originY * height;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;

                float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;

                for (int s = 0; s < samples; ++s) {
                    float scale = 1.0f - strength * (static_cast<float>(s) / (samples - 1));
                    int sampleX = std::clamp(static_cast<int>(centerX + dx * scale), 0, width - 1);
                    int sampleY = std::clamp(static_cast<int>(centerY + dy * scale), 0, height - 1);

                    uint32_t p = original[sampleY * width + sampleX];
                    sumR += ((p >> 16) & 0xFF);
                    sumG += ((p >> 8) & 0xFF);
                    sumB += (p & 0xFF);
                }

                uint32_t r = static_cast<uint32_t>(sumR / samples);
                uint32_t g = static_cast<uint32_t>(sumG / samples);
                uint32_t b = static_cast<uint32_t>(sumB / samples);

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RADIAL_BLUR2D_H
