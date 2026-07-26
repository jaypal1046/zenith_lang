#ifndef ZENITH_FOV2D_H
#define ZENITH_FOV2D_H

#include "zenith_raycast.h"
#include <vector>
#include <cmath>

namespace zenith {

struct VisionPoint {
    float x = 0.0f;
    float y = 0.0f;
};

class FOV2D {
public:
    static std::vector<VisionPoint> computeVisionCone(
        float originX, float originY,
        float lookAngleRad, float fovAngleRad,
        float radius, int rayCount = 16)
    {
        std::vector<VisionPoint> points;
        points.push_back({originX, originY});

        float startAngle = lookAngleRad - fovAngleRad * 0.5f;
        float step = fovAngleRad / static_cast<float>(rayCount - 1);

        for (int i = 0; i < rayCount; ++i) {
            float angle = startAngle + step * i;
            float dirX = std::cos(angle);
            float dirY = std::sin(angle);

            float hitX = originX + dirX * radius;
            float hitY = originY + dirY * radius;
            points.push_back({hitX, hitY});
        }

        return points;
    }
};

} // namespace zenith

#endif // ZENITH_FOV2D_H
