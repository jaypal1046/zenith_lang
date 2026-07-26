#ifndef ZENITH_TURN_ORDER2D_H
#define ZENITH_TURN_ORDER2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct TurnUnit {
    std::string name;
    uint32_t iconColor;
    float initiative;
    bool isPlayer;
};

class TurnOrder2D {
private:
    std::vector<TurnUnit> m_units;

public:
    void addUnit(const std::string& name, uint32_t iconColor, float initiative, bool isPlayer = true) {
        m_units.push_back({ name, iconColor, initiative, isPlayer });
        sortTurnOrder();
    }

    void sortTurnOrder() {
        std::sort(m_units.begin(), m_units.end(), [](const TurnUnit& a, const TurnUnit& b) {
            return a.initiative > b.initiative; // Higher initiative goes first
        });
    }

    void advanceTurn() {
        if (!m_units.empty()) {
            TurnUnit first = m_units.front();
            m_units.erase(m_units.begin());
            m_units.push_back(first);
        }
    }

    void drawTurnOrder(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 20, int iconSize = 24, int spacing = 6) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_units.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_units.size(); ++i) {
            int iX = posX + static_cast<int>(i) * (iconSize + spacing);
            uint32_t borderCol = m_units[i].isPlayer ? 0xFFFFD700 : 0xFFFF0000; // Gold for Player, Red for Enemy

            for (int y = posY; y < posY + iconSize; ++y) {
                for (int x = iX; x < iX + iconSize; ++x) {
                    if (x == iX || x == iX + iconSize - 1 || y == posY || y == posY + iconSize - 1) {
                        drawPixel(x, y, borderCol);
                    } else {
                        drawPixel(x, y, m_units[i].iconColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_TURN_ORDER2D_H
