#ifndef ZENITH_NARRATIVE_CHOICE2D_H
#define ZENITH_NARRATIVE_CHOICE2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct DialogueChoice {
    std::string text;
    int nextNodeId;
};

class NarrativeChoice2D {
private:
    std::vector<DialogueChoice> m_choices;
    int m_selectedIndex = 0;

public:
    void setChoices(const std::vector<DialogueChoice>& choices) {
        m_choices = choices;
        m_selectedIndex = 0;
    }

    void selectNext() {
        if (!m_choices.empty()) {
            m_selectedIndex = (m_selectedIndex + 1) % m_choices.size();
        }
    }

    void selectPrev() {
        if (!m_choices.empty()) {
            m_selectedIndex = (m_selectedIndex - 1 + m_choices.size()) % m_choices.size();
        }
    }

    int getSelectedChoiceIndex() const { return m_selectedIndex; }

    int getSelectedNextNodeId() const {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_choices.size())) {
            return m_choices[m_selectedIndex].nextNodeId;
        }
        return -1;
    }

    void drawChoices(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 50, int startY = 120, int choiceW = 200, int choiceH = 24, int spacing = 8) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_choices.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_choices.size(); ++i) {
            int cY = startY + static_cast<int>(i) * (choiceH + spacing);
            bool isSelected = (static_cast<int>(i) == m_selectedIndex);

            uint32_t bgCol = isSelected ? 0xFF4169E1 : 0xFF202030;     // Royal Blue for active / Dark Slate for idle
            uint32_t borderCol = isSelected ? 0xFFFFD700 : 0xFF606060; // Gold for active / Gray for idle

            for (int y = cY; y < cY + choiceH; ++y) {
                for (int x = posX; x < posX + choiceW; ++x) {
                    if (x == posX || x == posX + choiceW - 1 || y == cY || y == cY + choiceH - 1) {
                        drawPixel(x, y, borderCol);
                    } else {
                        drawPixel(x, y, bgCol);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_NARRATIVE_CHOICE2D_H
