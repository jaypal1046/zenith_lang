#ifndef ZENITH_TUNNEL2D_H
#define ZENITH_TUNNEL2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class TunnelFilter2D {
public:
    static void applyTunnel(
        std::vector<uint32_t>& buffer, int width, int height,
        float time, float speed = 1.0f)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        std::vector<uint32_t> original = buffer;
        float centerX = width * 0.5f;
        float centerY = height * 0.5f;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = x - centerX;
                float dy = y - centerY;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 1.0f) {
                    float angle = std::atan2(dy, dx);
                    float u = angle / 3.14159265f;
                    float v = (width * 0.5f / dist) + time * speed;

                    int srcX = std::clamp(static_cast<int>(std::fmod(std::abs(u * width), static_cast<float>(width))), 0, width - 1);
                    int srcY = std::clamp(static_cast<int>(std::fmod(std::abs(v * height), static_cast<float>(height))), 0, height - 1);

                    buffer[y * width + x] = original[srcY * width + srcX];
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_TUNNEL2D_H
