#ifndef ZENITH_IK2D_H
#define ZENITH_IK2D_H

#include <cmath>
#include <algorithm>

namespace zenith {

struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;
};

class IK2D {
public:
    static bool solveTwoBone(Vector2D root, Vector2D target, float lenA, float lenB, bool flip, Vector2D& outJoint) {
        float dx = target.x - root.x;
        float dy = target.y - root.y;
        float distSq = dx * dx + dy * dy;
        float dist = std::sqrt(distSq);

        if (dist > (lenA + lenB)) {
            // Target out of reach, stretch straight towards target
            outJoint.x = root.x + (dx / dist) * lenA;
            outJoint.y = root.y + (dy / dist) * lenA;
            return false;
        }

        float cosAngleA = (distSq + lenA * lenA - lenB * lenB) / (2.0f * dist * lenA);
        cosAngleA = std::max(-1.0f, std::min(1.0f, cosAngleA));
        float angleA = std::acos(cosAngleA);

        float baseAngle = std::atan2(dy, dx);
        float finalAngleA = flip ? (baseAngle - angleA) : (baseAngle + angleA);

        outJoint.x = root.x + std::cos(finalAngleA) * lenA;
        outJoint.y = root.y + std::sin(finalAngleA) * lenA;
        return true;
    }
};

} // namespace zenith

#endif // ZENITH_IK2D_H
