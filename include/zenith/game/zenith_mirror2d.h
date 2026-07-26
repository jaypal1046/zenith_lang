#ifndef ZENITH_MIRROR2D_H
#define ZENITH_MIRROR2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class MirrorFilter2D {
public:
    enum class Axis {
        HorizontalLeftToRight,
        HorizontalRightToLeft,
        VerticalTopToBottom,
        VerticalBottomToTop,
        QuadSymmetry
    };

    static void applyMirror(std::vector<uint32_t>& buffer, int width, int height, Axis axis = Axis::HorizontalLeftToRight) {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        int halfW = width / 2;
        int halfH = height / 2;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int srcX = x;
                int srcY = y;

                switch (axis) {
                    case Axis::HorizontalLeftToRight:
                        if (x >= halfW) srcX = width - 1 - x;
                        break;
                    case Axis::HorizontalRightToLeft:
                        if (x < halfW) srcX = width - 1 - x;
                        break;
                    case Axis::VerticalTopToBottom:
                        if (y >= halfH) srcY = height - 1 - y;
                        break;
                    case Axis::VerticalBottomToTop:
                        if (y < halfH) srcY = height - 1 - y;
                        break;
                    case Axis::QuadSymmetry:
                        if (x >= halfW) srcX = width - 1 - x;
                        if (y >= halfH) srcY = height - 1 - y;
                        break;
                }

                buffer[y * width + x] = buffer[srcY * width + srcX];
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_MIRROR2D_H
