#ifndef ZENITH_ASCII2D_H
#define ZENITH_ASCII2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ASCIIFilter2D {
public:
    static void applyASCII(
        std::vector<uint32_t>& buffer, int width, int height,
        int blockWidth = 8, int blockHeight = 8)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || blockWidth <= 1 || blockHeight <= 1) return;

        static const std::string asciiRamp = " .:-=+*#%@";

        for (int y = 0; y < height; y += blockHeight) {
            for (int x = 0; x < width; x += blockWidth) {
                float totalLuma = 0.0f;
                int count = 0;

                for (int py = y; py < std::min(y + blockHeight, height); ++py) {
                    for (int px = x; px < std::min(x + blockWidth, width); ++px) {
                        uint32_t p = buffer[py * width + px];
                        float r = (p >> 16) & 0xFF;
                        float g = (p >> 8) & 0xFF;
                        float b = p & 0xFF;
                        totalLuma += (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
                        count++;
                    }
                }

                float avgLuma = count > 0 ? totalLuma / count : 0.0f;
                int rampIndex = static_cast<int>(avgLuma * (asciiRamp.size() - 1));
                rampIndex = std::clamp(rampIndex, 0, static_cast<int>(asciiRamp.size() - 1));

                uint32_t charColor = static_cast<uint32_t>(avgLuma * 255.0f);
                uint32_t packedColor = (0xFF << 24) | (charColor << 16) | (charColor << 8) | charColor;

                for (int py = y; py < std::min(y + blockHeight, height); ++py) {
                    for (int px = x; px < std::min(x + blockWidth, width); ++px) {
                        buffer[py * width + px] = packedColor;
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_ASCII2D_H
