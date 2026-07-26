#ifndef ZENITH_LOOT_DROP2D_H
#define ZENITH_LOOT_DROP2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

enum class LootRarity {
    Common,     // White 0xFFFFFFFF
    Uncommon,   // Green 0xFF00FF00
    Rare,       // Blue 0xFF0080FF
    Epic,       // Purple 0xFFA000FF
    Legendary   // Orange 0xFFFF8000
};

struct WorldLootItem {
    std::string name;
    LootRarity rarity;
    float x, y;
    float pulseTime;
};

class LootDrop2D {
private:
    std::vector<WorldLootItem> m_drops;

public:
    void addDrop(const std::string& name, LootRarity rarity, float x, float y) {
        m_drops.push_back({ name, rarity, x, y, 0.0f });
    }

    void update(float dt) {
        for (auto& drop : m_drops) {
            drop.pulseTime += dt * 3.0f;
        }
    }

    static uint32_t getRarityColor(LootRarity rarity) {
        switch (rarity) {
            case LootRarity::Common:    return 0xFFFFFFFF;
            case LootRarity::Uncommon:  return 0xFF00FF00;
            case LootRarity::Rare:      return 0xFF0080FF;
            case LootRarity::Epic:      return 0xFFA000FF;
            case LootRarity::Legendary: return 0xFFFF8000;
            default:                    return 0xFFFFFFFF;
        }
    }

    void drawDrops(std::vector<uint32_t>& buffer, int width, int height) const {
        if (buffer.empty() || width <= 0 || height <= 0 || m_drops.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (const auto& drop : m_drops) {
            int dX = static_cast<int>(drop.x);
            int dY = static_cast<int>(drop.y);
            uint32_t col = getRarityColor(drop.rarity);

            // Render vertical light beam column extending upwards
            float pulse = 0.7f + 0.3f * std::sin(drop.pulseTime);
            int beamWidth = static_cast<int>(4.0f * pulse);

            for (int y = std::max(0, dY - 40); y <= dY; ++y) {
                for (int x = dX - beamWidth; x <= dX + beamWidth; ++x) {
                    drawPixel(x, y, col);
                }
            }

            // Render ground drop dot
            for (int dy = -3; dy <= 3; ++dy) {
                for (int dx = -3; dx <= 3; ++dx) {
                    if (dx * dx + dy * dy <= 9) {
                        drawPixel(dX + dx, dY + dy, col);
                    }
                }
            }

            // Render floating name tag bar
            int nameLen = static_cast<int>(drop.name.length() * 6);
            int tagY = dY - 45;
            for (int y = tagY; y < tagY + 8; ++y) {
                for (int x = dX - nameLen / 2; x < dX + nameLen / 2; ++x) {
                    drawPixel(x, y, col);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LOOT_DROP2D_H
