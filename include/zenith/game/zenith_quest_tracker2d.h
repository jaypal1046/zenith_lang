#ifndef ZENITH_QUEST_TRACKER2D_H
#define ZENITH_QUEST_TRACKER2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct QuestObjective {
    std::string text;
    bool isCompleted;
};

class QuestTracker2D {
private:
    std::string m_title;
    std::vector<QuestObjective> m_objectives;

public:
    void setQuest(const std::string& title) {
        m_title = title;
        m_objectives.clear();
    }

    void addObjective(const std::string& text, bool isCompleted = false) {
        m_objectives.push_back({ text, isCompleted });
    }

    void setObjectiveComplete(size_t index, bool isCompleted = true) {
        if (index < m_objectives.size()) {
            m_objectives[index].isCompleted = isCompleted;
        }
    }

    void drawQuestTracker(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 20) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_title.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render quest title header line
        int titleW = static_cast<int>(m_title.length() * 6 + 10);
        for (int y = posY; y < posY + 12; ++y) {
            for (int x = posX; x < posX + titleW; ++x) {
                drawPixel(x, y, 0xFFD700); // Gold title bar
            }
        }

        // Render objectives checklist
        int curY = posY + 16;
        for (const auto& obj : m_objectives) {
            uint32_t checkColor = obj.isCompleted ? 0xFF00FF00 : 0xFFFF4500;
            // Draw checkbox square
            for (int dy = 0; dy < 8; ++dy) {
                for (int dx = 0; dx < 8; ++dx) {
                    drawPixel(posX + dx, curY + dy, checkColor);
                }
            }

            // Draw objective text placeholder line
            int textW = static_cast<int>(obj.text.length() * 5);
            uint32_t textColor = obj.isCompleted ? 0xFF808080 : 0xFFFFFFFF;
            for (int dy = 2; dy < 6; ++dy) {
                for (int dx = 0; dx < textW; ++dx) {
                    drawPixel(posX + 12 + dx, curY + dy, textColor);
                }
            }

            curY += 12;
        }
    }
};

} // namespace zenith

#endif // ZENITH_QUEST_TRACKER2D_H
