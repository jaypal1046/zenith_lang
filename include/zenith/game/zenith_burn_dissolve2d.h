#ifndef ZENITH_BURN_DISSOLVE2D_H
#define ZENITH_BURN_DISSOLVE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class BurnDissolve2D {
public:
    static void applyBurnDissolve(
        std::vector<uint32_t>& buffer, int width, int height,
        float progress, uint32_t emberColor = 0xFFFF4500) // 0xFF + OrangeRed ember
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        progress = std::clamp(progress, 0.0f, 1.0f);
        if (progress <= 0.0f) return;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;

                // Simple procedural noise value based on coordinates
                float noise = std::fmod(std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f, 1.0f);
                noise = std::abs(noise);

                if (noise < progress) {
                    buffer[idx] = 0x00000000; // Completely burned away to transparent/black
                } else if (noise < progress + 0.08f) {
                    // Glowing ember edge
                    buffer[idx] = emberColor;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_BURN_DISSOLVE2D_H
