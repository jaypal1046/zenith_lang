#ifndef ZENITH_HOLOGRAM2D_H
#define ZENITH_HOLOGRAM2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class HologramFilter2D {
public:
    static void applyHologram(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, uint32_t holoColor = 0xFF00E5FF) // 0xFF + Cyan Holo
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 0; y < height; ++y) {
            // Horizontal scanline intensity
            float scanline = (y % 4 == 0) ? 0.4f : 1.0f;
            // Vertical jitter wave
            float jitter = std::sin(time * 15.0f + y * 0.2f) * 2.0f;

            for (int x = 0; x < width; ++x) {
                int srcX = std::clamp(static_cast<int>(x + jitter), 0, width - 1);
                int idx = y * width + x;
                uint32_t orig = original[y * width + srcX];

                float luma = (((orig >> 16) & 0xFF) * 0.299f + ((orig >> 8) & 0xFF) * 0.587f + (orig & 0xFF) * 0.114f) / 255.0f;
                luma *= scanline;

                uint32_t r = static_cast<uint32_t>(((holoColor >> 16) & 0xFF) * luma);
                uint32_t g = static_cast<uint32_t>(((holoColor >> 8) & 0xFF)  * luma);
                uint32_t b = static_cast<uint32_t>((holoColor & 0xFF)         * luma);

                buffer[idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_HOLOGRAM2D_H
