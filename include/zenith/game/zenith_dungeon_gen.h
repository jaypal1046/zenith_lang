#ifndef ZENITH_DUNGEON_GEN_H
#define ZENITH_DUNGEON_GEN_H

#include <vector>
#include <cstdlib>
#include <algorithm>

namespace zenith {

struct DungeonRoom {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    int getCenterX() const { return x + width / 2; }
    int getCenterY() const { return y + height / 2; }
};

class DungeonGenerator2D {
private:
    int m_width = 0;
    int m_height = 0;
    std::vector<int> m_grid; // 0 = Void/Wall, 1 = Floor
    std::vector<DungeonRoom> m_rooms;

public:
    DungeonGenerator2D(int width, int height)
        : m_width(width), m_height(height), m_grid(width * height, 0) {}

    void generate(int roomAttempts = 10, int minRoomSize = 4, int maxRoomSize = 10) {
        m_grid.assign(m_width * m_height, 0);
        m_rooms.clear();

        for (int i = 0; i < roomAttempts; ++i) {
            int rw = minRoomSize + std::rand() % (maxRoomSize - minRoomSize + 1);
            int rh = minRoomSize + std::rand() % (maxRoomSize - minRoomSize + 1);
            int rx = 1 + std::rand() % std::max(1, m_width - rw - 2);
            int ry = 1 + std::rand() % std::max(1, m_height - rh - 2);

            DungeonRoom newRoom{rx, ry, rw, rh};

            // Carve room floor
            for (int y = ry; y < ry + rh; ++y) {
                for (int x = rx; x < rx + rw; ++x) {
                    m_grid[y * m_width + x] = 1;
                }
            }

            // Connect to previous room center with corridor
            if (!m_rooms.empty()) {
                int prevCX = m_rooms.back().getCenterX();
                int prevCY = m_rooms.back().getCenterY();
                int newCX = newRoom.getCenterX();
                int newCY = newRoom.getCenterY();

                // Horizontal corridor
                int startX = std::min(prevCX, newCX);
                int endX = std::max(prevCX, newCX);
                for (int x = startX; x <= endX; ++x) {
                    m_grid[prevCY * m_width + x] = 1;
                }

                // Vertical corridor
                int startY = std::min(prevCY, newCY);
                int endY = std::max(prevCY, newCY);
                for (int y = startY; y <= endY; ++y) {
                    m_grid[y * m_width + newCX] = 1;
                }
            }

            m_rooms.push_back(newRoom);
        }
    }

    int getTile(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 0;
        return m_grid[y * m_width + x];
    }

    std::size_t getRoomCount() const { return m_rooms.size(); }
    const DungeonRoom& getRoom(std::size_t index) const { return m_rooms[index]; }
};

} // namespace zenith

#endif // ZENITH_DUNGEON_GEN_H
