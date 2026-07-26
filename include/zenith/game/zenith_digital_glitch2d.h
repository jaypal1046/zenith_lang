#ifndef ZENITH_DIGITAL_GLITCH2D_H
#define ZENITH_DIGITAL_GLITCH2D_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

class DigitalGlitch2D {
public:
    static void applyGlitch(std::vector<uint32_t>& buffer, int width, int height, float intensity = 0.2f) {
        if (buffer.empty() || width <= 0 || height <= 0 || intensity <= 0.0f) return;

        std::vector<uint32_t> temp = buffer;
        int numSlices = static_cast<int>(height * intensity * 0.2f);

        for (int i = 0; i < numSlices; ++i) {
            int sliceY = rand() % height;
            int sliceHeight = 1 + rand() % 5;
            int offsetX = static_cast<int>(((rand() % 100) / 100.0f * 2.0f - 1.0f) * width * 0.1f * intensity);

            for (int y = sliceY; y < std::min(sliceY + sliceHeight, height); ++y) {
                for (int x = 0; x < width; ++x) {
                    int srcX = std::clamp(x - offsetX, 0, width - 1);
                    buffer[y * width + x] = temp[y * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_DIGITAL_GLITCH2D_H
