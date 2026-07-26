#ifndef ZENITH_SHARPEN2D_H
#define ZENITH_SHARPEN2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class SharpenFilter2D {
public:
    static void applySharpen(std::vector<uint32_t>& buffer, int width, int height, float amount = 1.0f) {
        if (buffer.empty() || width < 3 || height < 3 || amount <= 0.0f) return;

        std::vector<uint32_t> original = buffer;

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                uint32_t center = original[y * width + x];
                uint32_t top    = original[(y - 1) * width + x];
                uint32_t bottom = original[(y + 1) * width + x];
                uint32_t left   = original[y * width + (x - 1)];
                uint32_t right  = original[y * width + (x + 1)];

                auto sharpenChannel = [&](int centerC, int topC, int bottomC, int leftC, int rightC) -> uint32_t {
                    float laplacian = 5.0f * centerC - (topC + bottomC + leftC + rightC);
                    float result = centerC + (laplacian - centerC) * amount;
                    return static_cast<uint32_t>(std::clamp(result, 0.0f, 255.0f));
                };

                uint32_t r = sharpenChannel((center >> 16) & 0xFF, (top >> 16) & 0xFF, (bottom >> 16) & 0xFF, (left >> 16) & 0xFF, (right >> 16) & 0xFF);
                uint32_t g = sharpenChannel((center >> 8) & 0xFF,  (top >> 8) & 0xFF,  (bottom >> 8) & 0xFF,  (left >> 8) & 0xFF,  (right >> 8) & 0xFF);
                uint32_t b = sharpenChannel(center & 0xFF,         top & 0xFF,         bottom & 0xFF,         left & 0xFF,         right & 0xFF);

                buffer[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SHARPEN2D_H
