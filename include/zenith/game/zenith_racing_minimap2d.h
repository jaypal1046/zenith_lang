#ifndef ZENITH_RACING_MINIMAP2D_H
#define ZENITH_RACING_MINIMAP2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct TrackPoint {
    float x, y;
};

struct DriverMarker {
    std::string name;
    float x, y;
    uint32_t color;
    bool isPlayer;
};

class RacingMinimap2D {
private:
    std::vector<TrackPoint> m_track;
    std::vector<DriverMarker> m_drivers;

public:
    void addTrackPoint(float x, float y) {
        m_track.push_back({ x, y });
    }

    void addDriver(const std::string& name, float x, float y, uint32_t color, bool isPlayer = false) {
        m_drivers.push_back({ name, x, y, color, isPlayer });
    }

    void clearDrivers() { m_drivers.clear(); }

    void drawMinimap(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 20, int minimapSize = 100, float worldScale = 0.1f) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Draw dark translucent circular minimap disk
        int cX = posX + minimapSize / 2;
        int cY = posY + minimapSize / 2;
        int r = minimapSize / 2;

        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                if (dx * dx + dy * dy <= r * r) {
                    drawPixel(cX + dx, cY + dy, 0xA010101A);
                }
            }
        }

        // Draw track loop lines
        if (m_track.size() > 1) {
            for (size_t i = 0; i < m_track.size(); ++i) {
                const auto& p0 = m_track[i];
                const auto& p1 = m_track[(i + 1) % m_track.size()];

                int x0 = cX + static_cast<int>(p0.x * worldScale);
                int y0 = cY + static_cast<int>(p0.y * worldScale);
                int x1 = cX + static_cast<int>(p1.x * worldScale);
                int y1 = cY + static_cast<int>(p1.y * worldScale);

                int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
                if (steps > 0) {
                    float dx = static_cast<float>(x1 - x0) / steps;
                    float dy = static_cast<float>(y1 - y0) / steps;
                    for (int s = 0; s <= steps; ++s) {
                        drawPixel(static_cast<int>(x0 + s * dx), static_cast<int>(y0 + s * dy), 0xFF808080);
                    }
                }
            }
        }

        // Draw driver dots
        for (const auto& driver : m_drivers) {
            int dX = cX + static_cast<int>(driver.x * worldScale);
            int dY = cY + static_cast<int>(driver.y * worldScale);
            int dotR = driver.isPlayer ? 4 : 2;

            for (int dy = -dotR; dy <= dotR; ++dy) {
                for (int dx = -dotR; dx <= dotR; ++dx) {
                    if (dx * dx + dy * dy <= dotR * dotR) {
                        drawPixel(dX + dx, dY + dy, driver.color);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_RACING_MINIMAP2D_H
