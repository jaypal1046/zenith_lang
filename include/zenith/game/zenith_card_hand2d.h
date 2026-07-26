#ifndef ZENITH_CARD_HAND2D_H
#define ZENITH_CARD_HAND2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct CardItem {
    int id;
    std::string title;
    uint32_t cardColor;
    int manaCost;
};

class CardHand2D {
private:
    std::vector<CardItem> m_cards;
    int m_hoveredIndex = -1;

public:
    void addCard(int id, const std::string& title, uint32_t cardColor, int manaCost) {
        m_cards.push_back({ id, title, cardColor, manaCost });
    }

    void setHoveredCard(int index) {
        m_hoveredIndex = (index >= 0 && index < static_cast<int>(m_cards.size())) ? index : -1;
    }

    size_t getCardCount() const { return m_cards.size(); }

    void drawHand(
        std::vector<uint32_t>& buffer, int width, int height,
        int cardWidth = 40, int cardHeight = 60) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_cards.empty()) return;

        int numCards = static_cast<int>(m_cards.size());
        int startX = (width - numCards * (cardWidth - 10)) / 2;
        int baseY = height - cardHeight - 10;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (int i = 0; i < numCards; ++i) {
            int cX = startX + i * (cardWidth - 10);
            int cY = (i == m_hoveredIndex) ? (baseY - 15) : baseY; // Elevate hovered card
            uint32_t borderCol = (i == m_hoveredIndex) ? 0xFFFFD700 : 0xFFFFFFFF;

            for (int y = cY; y < cY + cardHeight; ++y) {
                for (int x = cX; x < cX + cardWidth; ++x) {
                    if (x == cX || x == cX + cardWidth - 1 || y == cY || y == cY + cardHeight - 1) {
                        drawPixel(x, y, borderCol);
                    } else {
                        drawPixel(x, y, m_cards[i].cardColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CARD_HAND2D_H
