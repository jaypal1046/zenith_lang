#ifndef ZENITH_SPLINE2D_H
#define ZENITH_SPLINE2D_H

namespace zenith {

struct SplinePoint2D {
    float x = 0.0f;
    float y = 0.0f;
};

class Spline2D {
public:
    static SplinePoint2D quadraticBezier(SplinePoint2D p0, SplinePoint2D p1, SplinePoint2D p2, float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        return {
            uu * p0.x + 2.0f * u * t * p1.x + tt * p2.x,
            uu * p0.y + 2.0f * u * t * p1.y + tt * p2.y
        };
    }

    static SplinePoint2D cubicBezier(SplinePoint2D p0, SplinePoint2D p1, SplinePoint2D p2, SplinePoint2D p3, float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;
        return {
            uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
            uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y
        };
    }
};

} // namespace zenith

#endif // ZENITH_SPLINE2D_H
