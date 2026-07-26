#ifndef ZENITH_DIALOGUE_BOX2D_H
#define ZENITH_DIALOGUE_BOX2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

class DialogueBox2D {
public:
    static void drawDialogueBox(
        std::vector<uint32_t>& buffer, int width, int height,
        const std::string& speakerName, const std::string& text,
        size_t visibleCharCount,
        const std::vector<std::string>& choices = {}, int selectedChoice = 0,
        int boxHeight = 120, uint32_t boxBgColor = 0xE0101020, uint32_t borderGold = 0xFFFFD700)
    {
        if (buffer.empty() || width <= 0 || height <= 0) return;

        int boxY = height - boxHeight - 20;
        int boxX = 30;
        int boxWidth = width - 60;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Draw Dialogue Box Background & Metallic Border
        for (int y = boxY; y < boxY + boxHeight; ++y) {
            for (int x = boxX; x < boxX + boxWidth; ++x) {
                if (x <= boxX + 2 || x >= boxX + boxWidth - 3 || y <= boxY + 2 || y >= boxY + boxHeight - 3) {
                    drawPixel(x, y, borderGold);
                } else {
                    drawPixel(x, y, boxBgColor);
                }
            }
        }

        // Draw Speaker Name Tag Frame
        if (!speakerName.empty()) {
            int nameWidth = static_cast<int>(speakerName.length() * 8 + 16);
            for (int y = boxY - 18; y <= boxY; ++y) {
                for (int x = boxX + 10; x <= boxX + 10 + nameWidth; ++x) {
                    drawPixel(x, y, 0xFF2A2A4A);
                }
            }
        }

        // Draw visible text character placeholder blocks
        size_t charsToDraw = std::min(visibleCharCount, text.length());
        int charX = boxX + 15;
        int charY = boxY + 15;

        for (size_t i = 0; i < charsToDraw; ++i) {
            for (int dy = 0; dy < 8; ++dy) {
                for (int dx = 0; dx < 6; ++dx) {
                    drawPixel(charX + dx, charY + dy, 0xFFFFFFFF);
                }
            }
            charX += 8;
            if (charX > boxX + boxWidth - 20) {
                charX = boxX + 15;
                charY += 12;
            }
        }

        // Render branching dialogue choices below text
        if (!choices.empty()) {
            int choiceY = boxY + boxHeight - 30;
            for (size_t cIdx = 0; cIdx < choices.size(); ++cIdx) {
                uint32_t choiceColor = (static_cast<int>(cIdx) == selectedChoice) ? 0xFF00FF00 : 0xFFA0A0A0;
                int cX = boxX + 20 + static_cast<int>(cIdx * 120);

                for (int dy = 0; dy < 10; ++dy) {
                    for (int dx = 0; dx < 80; ++dx) {
                        drawPixel(cX + dx, choiceY + dy, choiceColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_DIALOGUE_BOX2D_H
