#ifndef ZENITH_LIGHTNING2D_H
#define ZENITH_LIGHTNING2D_H

#include <vector>
#include <cmath>
#include <cstdlib>

namespace zenith {

struct LightningSegment2D {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
};

class LightningGenerator2D {
public:
    static std::vector<LightningSegment2D> generateBolt(
        float startX, float startY, float endX, float endY,
        int generations = 4, float maxDisplacement = 30.0f)
    {
        std::vector<LightningSegment2D> segments;
        segments.push_back({startX, startY, endX, endY});

        float currentDisplacement = maxDisplacement;

        for (int g = 0; g < generations; ++g) {
            std::vector<LightningSegment2D> nextGen;
            for (const auto& seg : segments) {
                float midX = (seg.x1 + seg.x2) * 0.5f;
                float midY = (seg.y1 + seg.y2) * 0.5f;

                // Perpendicular vector
                float dx = seg.x2 - seg.x1;
                float dy = seg.y2 - seg.y1;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.0001f) {
                    float nx = -dy / len;
                    float ny = dx / len;

                    float offset = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * currentDisplacement;
                    midX += nx * offset;
                    midY += ny * offset;
                }

                nextGen.push_back({seg.x1, seg.y1, midX, midY});
                nextGen.push_back({midX, midY, seg.x2, seg.y2});
            }
            segments = nextGen;
            currentDisplacement *= 0.5f;
        }

        return segments;
    }
};

} // namespace zenith

#endif // ZENITH_LIGHTNING2D_H
