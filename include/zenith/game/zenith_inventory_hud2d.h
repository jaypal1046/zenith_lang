#ifndef ZENITH_INVENTORY_HUD2D_H
#define ZENITH_INVENTORY_HUD2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct HotbarSlot {
    uint32_t iconColor;
    int count;
};

class InventoryHUD2D {
public:
    static void drawHotbar(
        std::vector<uint32_t>& buffer, int width, int height,
        const std::vector<HotbarSlot>& slots, int selectedIndex = 0,
        int slotSize = 36, int slotSpacing = 6, int bottomMargin = 20)
    {
        if (buffer.empty() || width <= 0 || height <= 0 || slots.empty()) return;

        int totalWidth = static_cast<int>(slots.size()) * (slotSize + slotSpacing) - slotSpacing;
        int startX = (width - totalWidth) / 2;
        int startY = height - bottomMargin - slotSize;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < slots.size(); ++i) {
            int sX = startX + static_cast<int>(i) * (slotSize + slotSpacing);
            uint32_t borderCol = (static_cast<int>(i) == selectedIndex) ? 0xFFFFD700 : 0xFF808080;

            // Draw slot box & border
            for (int y = startY; y < startY + slotSize; ++y) {
                for (int x = sX; x < sX + slotSize; ++x) {
                    if (x <= sX + 1 || x >= sX + slotSize - 2 || y <= startY + 1 || y >= startY + slotSize - 2) {
                        drawPixel(x, y, borderCol);
                    } else {
                        drawPixel(x, y, 0xE0202020);
                    }
                }
            }

            // Draw item icon fill inside slot
            if (slots[i].count > 0) {
                for (int y = startY + 6; y < startY + slotSize - 6; ++y) {
                    for (int x = sX + 6; x < sX + slotSize - 6; ++x) {
                        drawPixel(x, y, slots[i].iconColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_INVENTORY_HUD2D_H
