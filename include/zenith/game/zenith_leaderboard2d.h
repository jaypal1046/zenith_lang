#ifndef ZENITH_LEADERBOARD2D_H
#define ZENITH_LEADERBOARD2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct ScoreEntry {
    std::string name;
    int score;
};

class Leaderboard2D {
private:
    std::vector<ScoreEntry> m_entries;
    size_t m_maxEntries = 10;

public:
    void addEntry(const std::string& name, int score) {
        m_entries.push_back({ name, score });
        sortLeaderboard();
    }

    void sortLeaderboard() {
        std::sort(m_entries.begin(), m_entries.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.score > b.score; // High scores first
        });
        if (m_entries.size() > m_maxEntries) {
            m_entries.resize(m_maxEntries);
        }
    }

    void drawLeaderboard(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 40, int posY = 40, int lineSpacing = 16) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_entries.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_entries.size(); ++i) {
            int curY = posY + static_cast<int>(i) * lineSpacing;
            uint32_t col = (i == 0) ? 0xFFFFD700 : ((i == 1) ? 0xFFC0C0C0 : ((i == 2) ? 0xFFCD7F32 : 0xFFFFFFFF)); // Gold, Silver, Bronze, White

            // Draw rank number bar
            for (int dy = 0; dy < 10; ++dy) {
                for (int dx = 0; dx < 10; ++dx) {
                    drawPixel(posX + dx, curY + dy, col);
                }
            }

            // Draw name tag bar
            int nameW = static_cast<int>(m_entries[i].name.length() * 6);
            for (int dy = 2; dy < 8; ++dy) {
                for (int dx = 0; dx < nameW; ++dx) {
                    drawPixel(posX + 16 + dx, curY + dy, col);
                }
            }

            // Draw score value bar
            for (int dy = 2; dy < 8; ++dy) {
                for (int dx = 0; dx < 30; ++dx) {
                    drawPixel(posX + 120 + dx, curY + dy, col);
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_LEADERBOARD2D_H
