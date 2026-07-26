#ifndef ZENITH_AUTOTILE_H
#define ZENITH_AUTOTILE_H

#include <cstdint>

namespace zenith {

class AutoTile2D {
public:
    static uint8_t calculateBitmask(bool up, bool down, bool left, bool right) {
        uint8_t mask = 0;
        if (up)    mask |= 1; // 0001
        if (right) mask |= 2; // 0010
        if (down)  mask |= 4; // 0100
        if (left)  mask |= 8; // 1000
        return mask;
    }

    static int getTileIndexFromBitmask(uint8_t mask) {
        // Standard 16-tile lookup map
        return static_cast<int>(mask);
    }
};

} // namespace zenith

#endif // ZENITH_AUTOTILE_H
