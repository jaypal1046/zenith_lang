#ifndef ZENITH_OIL_PAINT2D_H
#define ZENITH_OIL_PAINT2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class OilPaintFilter2D {
public:
    static void applyOilPaint(std::vector<uint32_t>& buffer, int width, int height, int radius = 2) {
        if (buffer.empty() || width <= radius * 2 || height <= radius * 2 || radius <= 0) return;

        std::vector<uint32_t> original = buffer;

        for (int y = radius; y < height - radius; ++y) {
            for (int x = radius; x < width - radius; ++x) {
                // Kuwahara 4-quadrant variance check
                float minVariance = 1e9f;
                uint32_t bestMeanColor = original[y * width + x];

                auto evalQuadrant = [&](int xStart, int xEnd, int yStart, int yEnd) {
                    float sumR = 0, sumG = 0, sumB = 0;
                    float sumSqR = 0, sumSqG = 0, sumSqB = 0;
                    int count = 0;

                    for (int py = yStart; py <= yEnd; ++py) {
                        for (int px = xStart; px <= xEnd; ++px) {
                            uint32_t p = original[py * width + px];
                            float r = (p >> 16) & 0xFF;
                            float g = (p >> 8) & 0xFF;
                            float b = p & 0xFF;

                            sumR += r; sumG += g; sumB += b;
                            sumSqR += r * r; sumSqG += g * g; sumSqB += b * b;
                            count++;
                        }
                    }

                    if (count == 0) return;

                    float meanR = sumR / count;
                    float meanG = sumG / count;
                    float meanB = sumB / count;

                    float varR = (sumSqR / count) - (meanR * meanR);
                    float varG = (sumSqG / count) - (meanG * meanG);
                    float varB = (sumSqB / count) - (meanB * meanB);
                    float totalVar = varR + varG + varB;

                    if (totalVar < minVariance) {
                        minVariance = totalVar;
                        bestMeanColor = (0xFF << 24) |
                                        (static_cast<uint32_t>(meanR) << 16) |
                                        (static_cast<uint32_t>(meanG) << 8) |
                                        static_cast<uint32_t>(meanB);
                    }
                };

                evalQuadrant(x - radius, x, y - radius, y);
                evalQuadrant(x, x + radius, y - radius, y);
                evalQuadrant(x - radius, x, y, y + radius);
                evalQuadrant(x, x + radius, y, y + radius);

                buffer[y * width + x] = bestMeanColor;
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_OIL_PAINT2D_H
