#ifndef ZENITH_SPEED_LINES2D_H
#define ZENITH_SPEED_LINES2D_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct SpeedLineSegment {
    float x1, y1;
    float x2, y2;
    uint32_t color;
};

class SpeedLines2D {
public:
    static std::vector<SpeedLineSegment> generateSpeedLines(
        float width, float height, float centerX, float centerY,
        int lineCount = 30, float innerRadius = 150.0f, uint32_t color = 0xFFFFFFFF)
    {
        std::vector<SpeedLineSegment> lines;
        lines.reserve(lineCount);

        float maxRadius = std::sqrt(width * width + height * height) * 0.6f;

        for (int i = 0; i < lineCount; ++i) {
            float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28318530718f;
            float r1 = innerRadius + (static_cast<float>(rand()) / RAND_MAX) * 50.0f;
            float r2 = r1 + 100.0f + (static_cast<float>(rand()) / RAND_MAX) * (maxRadius - r1);

            SpeedLineSegment line;
            line.x1 = centerX + std::cos(angle) * r1;
            line.y1 = centerY + std::sin(angle) * r1;
            line.x2 = centerX + std::cos(angle) * r2;
            line.y2 = centerY + std::sin(angle) * r2;
            line.color = color;

            lines.push_back(line);
        }

        return lines;
    }
};

} // namespace zenith

#endif // ZENITH_SPEED_LINES2D_H
