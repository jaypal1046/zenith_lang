#ifndef ZENITH_SHADOW2D_H
#define ZENITH_SHADOW2D_H

#include "zenith_spatial.h"
#include <vector>
#include <cmath>

namespace zenith {

struct ShadowPoint {
    float x = 0.0f;
    float y = 0.0f;
};

class ShadowCaster2D {
public:
    static std::vector<ShadowPoint> projectBoxShadow(float lightX, float lightY, const Rect2D& box, float shadowLength = 1000.0f) {
        std::vector<ShadowPoint> shadowVerts;

        // Box corners
        float corners[4][2] = {
            {box.x, box.y},
            {box.x + box.width, box.y},
            {box.x + box.width, box.y + box.height},
            {box.x, box.y + box.height}
        };

        for (int i = 0; i < 4; ++i) {
            float cx = corners[i][0];
            float cy = corners[i][1];
            float dx = cx - lightX;
            float dy = cy - lightY;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 0.0001f) {
                float projX = cx + (dx / dist) * shadowLength;
                float projY = cy + (dy / dist) * shadowLength;
                shadowVerts.push_back({projX, projY});
            }
        }

        return shadowVerts;
    }
};

} // namespace zenith

#endif // ZENITH_SHADOW2D_H
