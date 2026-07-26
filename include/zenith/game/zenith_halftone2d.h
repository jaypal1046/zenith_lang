#ifndef ZENITH_HALFTONE2D_H
#define ZENITH_HALFTONE2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class HalftoneFilter2D {
public:
    static void applyHalftone(std::vector<uint32_t>& buffer, int width, int height, int dotScale = 6) {
        if (buffer.empty() || width <= 0 || height <= 0 || dotScale <= 1) return;

        float halfScale = dotScale * 0.5f;

        for (int y = 0; y < height; y += dotScale) {
            for (int x = 0; x < width; x += dotScale) {
                // Calculate average luminance in cell
                float totalLuma = 0.0f;
                int count = 0;

                for (int py = y; py < std::min(y + dotScale, height); ++py) {
                    for (int px = x; px < std::min(x + dotScale, width); ++px) {
                        uint32_t p = buffer[py * width + px];
                        float r = (p >> 16) & 0xFF;
                        float g = (p >> 8) & 0xFF;
                        float b = p & 0xFF;
                        totalLuma += (r * 0.299f + g * 0.587f + b * 0.114f) / 255.0f;
                        count++;
                    }
                }

                float avgLuma = count > 0 ? totalLuma / count : 0.0f;
                float dotRadius = avgLuma * halfScale;

                float cellCenterX = x + halfScale;
                float cellCenterY = y + halfScale;

                for (int py = y; py < std::min(y + dotScale, height); ++py) {
                    for (int px = x; px < std::min(x + dotScale, width); ++px) {
                        float dx = px - cellCenterX;
                        float dy = py - cellCenterY;
                        float dist = std::sqrt(dx * dx + dy * dy);

                        if (dist <= dotRadius) {
                            buffer[py * width + px] = 0xFF000000; // Black ink dot
                        } else {
                            buffer[py * width + px] = 0xFFFFFFFF; // White paper background
                        }
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_HALFTONE2D_H
