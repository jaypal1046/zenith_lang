#ifndef ZENITH_MINIMAP_HUD2D_H
#define ZENITH_MINIMAP_HUD2D_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct MinimapEntity {
    float x, y;
    uint32_t color; // E.g., Red for enemy, Green for ally, Yellow for objective
};

class MinimapHUD2D {
public:
    static void drawMinimap(
        std::vector<uint32_t>& buffer, int width, int height,
        float playerX, float playerY, const std::vector<MinimapEntity>& entities,
        int mapCenterX = 70, int mapCenterY = 70, float mapRadius = 50.0f, float radarScale = 0.1f)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || mapRadius <= 0.0f) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render circular background disk and border
        int r = static_cast<int>(mapRadius);
        for (int dy = -r; dy <= r; ++dy) {
            for (int dx = -r; dx <= r; ++dx) {
                float distSq = dx * dx + dy * dy;
                if (distSq <= mapRadius * mapRadius) {
                    if (distSq >= (mapRadius - 2.0f) * (mapRadius - 2.0f)) {
                        drawPixel(mapCenterX + dx, mapCenterY + dy, 0xFF00FF00); // Green border
                    } else {
                        drawPixel(mapCenterX + dx, mapCenterY + dy, 0xC0002000); // Translucent dark green disk
                    }
                }
            }
        }

        // Draw Player center icon
        drawPixel(mapCenterX, mapCenterY, 0xFFFFFFFF);
        drawPixel(mapCenterX + 1, mapCenterY, 0xFFFFFFFF);
        drawPixel(mapCenterX, mapCenterY + 1, 0xFFFFFFFF);
        drawPixel(mapCenterX + 1, mapCenterY + 1, 0xFFFFFFFF);

        // Draw entities relative to player
        for (const auto& ent : entities) {
            float relX = (ent.x - playerX) * radarScale;
            float relY = (ent.y - playerY) * radarScale;
            float relDist = std::sqrt(relX * relX + relY * relY);

            if (relDist <= mapRadius - 4.0f) {
                int ePx = static_cast<int>(mapCenterX + relX);
                int ePy = static_cast<int>(mapCenterY + relY);
                drawPixel(ePx, ePy, ent.color);
                drawPixel(ePx + 1, ePy, ent.color);
                drawPixel(ePx, ePy + 1, ent.color);
                drawPixel(ePx + 1, ePy + 1, ent.color);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_MINIMAP_HUD2D_H
