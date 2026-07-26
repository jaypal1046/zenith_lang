#ifndef ZENITH_RAYCAST_H
#define ZENITH_RAYCAST_H

#include "zenith_spatial.h"
#include <cmath>
#include <algorithm>

namespace zenith {

class PhysicsRaycast2D {
public:
    static spatial::RaycastHit2D castRayAgainstBox(
        const spatial::Point2D& origin,
        const spatial::Point2D& direction, // normalized
        float maxDistance,
        const spatial::Rect2D& box
    ) {
        spatial::RaycastHit2D hit;
        hit.hit = false;

        float invDirX = (fabsf(direction.x) > 0.00001f) ? (1.0f / direction.x) : 1e30f;
        float invDirY = (fabsf(direction.y) > 0.00001f) ? (1.0f / direction.y) : 1e30f;

        float t1 = (box.x - origin.x) * invDirX;
        float t2 = (box.x + box.width - origin.x) * invDirX;
        float t3 = (box.y - origin.y) * invDirY;
        float t4 = (box.y + box.height - origin.y) * invDirY;

        float tmin = std::max(std::min(t1, t2), std::min(t3, t4));
        float tmax = std::min(std::max(t1, t2), std::max(t3, t4));

        if (tmax < 0 || tmin > tmax || tmin > maxDistance) {
            return hit;
        }

        hit.hit = true;
        hit.distance = (tmin < 0.0f) ? tmax : tmin;
        hit.point.x = origin.x + direction.x * hit.distance;
        hit.point.y = origin.y + direction.y * hit.distance;
        return hit;
    }
};

} // namespace zenith

#endif // ZENITH_RAYCAST_H
