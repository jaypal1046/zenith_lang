#ifndef ZENITH_COLLISION_SOLVER_H
#define ZENITH_COLLISION_SOLVER_H

#include "zenith_spatial.h"
#include <cmath>
#include <algorithm>

namespace zenith {

struct CollisionResolution2D {
    bool collided = false;
    float overlapX = 0.0f;
    float overlapY = 0.0f;
    Point2D normal;
};

class CollisionSolver2D {
public:
    static CollisionResolution2D checkAABBOverlap(const spatial::Rect2D& a, const spatial::Rect2D& b) {
        CollisionResolution2D res;
        if (!a.intersects(b)) {
            return res;
        }

        res.collided = true;
        float centerAX = a.x + a.width * 0.5f;
        float centerAY = a.y + a.height * 0.5f;
        float centerBX = b.x + b.width * 0.5f;
        float centerBY = b.y + b.height * 0.5f;

        float diffX = centerAX - centerBX;
        float diffY = centerAY - centerBY;

        float minDistX = (a.width + b.width) * 0.5f;
        float minDistY = (a.height + b.height) * 0.5f;

        res.overlapX = minDistX - fabsf(diffX);
        res.overlapY = minDistY - fabsf(diffY);

        if (res.overlapX < res.overlapY) {
            res.normal.x = (diffX > 0) ? 1.0f : -1.0f;
            res.normal.y = 0.0f;
        } else {
            res.normal.x = 0.0f;
            res.normal.y = (diffY > 0) ? 1.0f : -1.0f;
        }

        return res;
    }

    static void resolveAABBCollision(spatial::Rect2D& mover, Point2D& velocity, const spatial::Rect2D& obstacle, float restitution = 0.0f) {
        CollisionResolution2D res = checkAABBOverlap(mover, obstacle);
        if (!res.collided) return;

        if (res.overlapX < res.overlapY) {
            mover.x += res.normal.x * res.overlapX;
            velocity.x = -velocity.x * restitution;
        } else {
            mover.y += res.normal.y * res.overlapY;
            velocity.y = -velocity.y * restitution;
        }
    }
};

} // namespace zenith

#endif // ZENITH_COLLISION_SOLVER_H
