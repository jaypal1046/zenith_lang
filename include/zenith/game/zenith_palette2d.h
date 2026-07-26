#ifndef ZENITH_PALETTE2D_H
#define ZENITH_PALETTE2D_H

#include <vector>
#include <cstdint>
#include <algorithm>

namespace zenith {

class PaletteSwapper2D {
public:
    static void swapColors(
        std::vector<uint32_t>& buffer, int width, int height,
        const std::vector<uint32_t>& fromPalette,
        const std::vector<uint32_t>& toPalette)
    {
        if (buffer.empty() || fromPalette.size() != toPalette.size()) return;

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            uint32_t current = buffer[i];
            for (std::size_t p = 0; p < fromPalette.size(); ++p) {
                if (current == fromPalette[p]) {
                    buffer[i] = toPalette[p];
                    break;
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_PALETTE2D_H
