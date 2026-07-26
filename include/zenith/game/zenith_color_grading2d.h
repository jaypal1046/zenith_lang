#ifndef ZENITH_COLOR_GRADING2D_H
#define ZENITH_COLOR_GRADING2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ColorGrading2D {
public:
    static void applyGrading(
        std::vector<uint32_t>& buffer, int width, int height,
        float brightness = 1.0f, float contrast = 1.0f, float saturation = 1.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t pixel = buffer[i];
            float r = static_cast<float>((pixel >> 16) & 0xFF) / 255.0f;
            float g = static_cast<float>((pixel >> 8) & 0xFF) / 255.0f;
            float b = static_cast<float>(pixel & 0xFF) / 255.0f;

            // Brightness adjustment
            r *= brightness;
            g *= brightness;
            b *= brightness;

            // Contrast adjustment
            r = (r - 0.5f) * contrast + 0.5f;
            g = (g - 0.5f) * contrast + 0.5f;
            b = (b - 0.5f) * contrast + 0.5f;

            // Saturation adjustment
            float gray = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            r = gray + (r - gray) * saturation;
            g = gray + (g - gray) * saturation;
            b = gray + (b - gray) * saturation;

            uint32_t finalR = static_cast<uint32_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
            uint32_t finalG = static_cast<uint32_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
            uint32_t finalB = static_cast<uint32_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));

            buffer[i] = (0xFF << 24) | (finalR << 16) | (finalG << 8) | finalB;
        }
    }
};

} // namespace zenith

#endif // ZENITH_COLOR_GRADING2D_H
