#ifndef ZENITH_EQUIPMENT_DOLL2D_H
#define ZENITH_EQUIPMENT_DOLL2D_H

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cstdint>

namespace zenith {

enum class EquipSlot {
    Helmet,
    Chestplate,
    Leggings,
    Boots,
    WeaponMain,
    WeaponOffhand
};

struct EquipItem {
    std::string name;
    uint32_t color;
    int armorValue;
};

class EquipmentDoll2D {
private:
    std::map<EquipSlot, EquipItem> m_equipped;

public:
    void setEquippedItem(EquipSlot slot, const std::string& name, uint32_t color, int armorValue) {
        m_equipped[slot] = { name, color, armorValue };
    }

    void unequipSlot(EquipSlot slot) {
        m_equipped.erase(slot);
    }

    int getTotalArmor() const {
        int total = 0;
        for (const auto& kv : m_equipped) {
            total += kv.second.armorValue;
        }
        return total;
    }

    void drawDoll(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 50, int posY = 50, int slotSize = 30) const
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render central character silhouette representation
        for (int y = posY + 35; y < posY + 115; ++y) {
            for (int x = posX + 40; x < posX + 70; ++x) {
                drawPixel(x, y, 0xFF303045);
            }
        }

        // Slot offsets around paper doll
        std::map<EquipSlot, std::pair<int, int>> slotOffsets = {
            { EquipSlot::Helmet,        { posX + 40, posY + 0 } },
            { EquipSlot::Chestplate,    { posX + 40, posY + 40 } },
            { EquipSlot::Leggings,      { posX + 40, posY + 80 } },
            { EquipSlot::Boots,         { posX + 40, posY + 120 } },
            { EquipSlot::WeaponMain,    { posX + 0,  posY + 40 } },
            { EquipSlot::WeaponOffhand, { posX + 80, posY + 40 } }
        };

        for (const auto& kv : slotOffsets) {
            EquipSlot slot = kv.first;
            int sX = kv.second.first;
            int sY = kv.second.second;

            bool isEquipped = m_equipped.find(slot) != m_equipped.end();
            uint32_t borderCol = isEquipped ? 0xFFFFD700 : 0xFF606060;

            for (int y = sY; y < sY + slotSize; ++y) {
                for (int x = sX; x < sX + slotSize; ++x) {
                    if (x <= sX + 1 || x >= sX + slotSize - 2 || y <= sY + 1 || y >= sY + slotSize - 2) {
                        drawPixel(x, y, borderCol);
                    } else if (isEquipped) {
                        drawPixel(x, y, m_equipped.at(slot).color);
                    } else {
                        drawPixel(x, y, 0xFF151520);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_EQUIPMENT_DOLL2D_H
