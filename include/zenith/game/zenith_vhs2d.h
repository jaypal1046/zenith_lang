#ifndef ZENITH_VHS2D_H
#define ZENITH_VHS2D_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

class VHSFilter2D {
public:
    static void applyVHS(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float noiseIntensity = 0.15f, float trackingError = 0.05f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;

        int trackingY = static_cast<int>(std::fmod(time * 30.0f, static_cast<float>(height)));
        int trackingHeight = static_cast<int>(height * trackingError);

        for (int y = 0; y < height; ++y) {
            int jitterX = 0;
            if (y >= trackingY && y < trackingY + trackingHeight) {
                jitterX = static_cast<int>((rand() % 10 - 5) * trackingError * 2.0f);
            }

            for (int x = 0; x < width; ++x) {
                int srcX = std::clamp(x + jitterX, 0, width - 1);
                int chromaShiftX = std::clamp(srcX + 2, 0, width - 1);

                uint32_t pMain = original[y * width + srcX];
                uint32_t pShift = original[y * width + chromaShiftX];

                uint32_t r = (pMain >> 16) & 0xFF;
                uint32_t g = (pMain >> 8) & 0xFF;
                uint32_t b = pShift & 0xFF; // Color bleed/chroma shift

                // Tape grain noise
                float noise = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * noiseIntensity * 255.0f;
                r = static_cast<uint32_t>(std::clamp(r + noise, 0.0f, 255.0f));
                g = static_cast<uint32_t>(std::clamp(g + noise, 0.0f, 255.0f));
                b = static_cast<uint32_t>(std::clamp(b + noise, 0.0f, 255.0f));

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_VHS2D_H
