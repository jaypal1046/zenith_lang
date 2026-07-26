#ifndef ZENITH_NORMAL_MAP2D_H
#define ZENITH_NORMAL_MAP2D_H

#include <cmath>
#include <algorithm>

namespace zenith {

class NormalMap2D {
public:
    static float calculateLightIntensity(
        float normalX, float normalY, float normalZ,
        float lightDirX, float lightDirY, float lightDirZ)
    {
        // Dot product N dot L
        float dot = normalX * lightDirX + normalY * lightDirY + normalZ * lightDirZ;
        return std::max(0.0f, dot);
    }
};

} // namespace zenith

#endif // ZENITH_NORMAL_MAP2D_H
