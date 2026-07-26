#ifndef ZENITH_PATHFINDING_H
#define ZENITH_PATHFINDING_H

#include "zenith_tilemap.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace zenith {

struct PathNode2D {
    int col = 0;
    int row = 0;

    bool operator==(const PathNode2D& other) const {
        return col == other.col && row == other.row;
    }
};

struct PathNodeHash {
    std::size_t operator()(const PathNode2D& n) const {
        return (static_cast<std::size_t>(n.col) * 73856093) ^ (static_cast<std::size_t>(n.row) * 19349663);
    }
};

class GridPathfinder2D {
public:
    static std::vector<PathNode2D> findPath(int startCol, int startRow, int targetCol, int targetRow, const TileMap2D& map) {
        std::vector<PathNode2D> path;

        if (startCol < 0 || startCol >= map.getCols() || startRow < 0 || startRow >= map.getRows()) return path;
        if (targetCol < 0 || targetCol >= map.getCols() || targetRow < 0 || targetRow >= map.getRows()) return path;

        PathNode2D start{startCol, startRow};
        PathNode2D target{targetCol, targetRow};

        std::queue<PathNode2D> frontier;
        frontier.push(start);

        std::unordered_map<PathNode2D, PathNode2D, PathNodeHash> cameFrom;
        cameFrom[start] = start;

        int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        while (!frontier.empty()) {
            PathNode2D current = frontier.front();
            frontier.pop();

            if (current == target) break;

            for (int i = 0; i < 4; ++i) {
                int nextCol = current.col + directions[i][0];
                int nextRow = current.row + directions[i][1];

                if (nextCol >= 0 && nextCol < map.getCols() && nextRow >= 0 && nextRow < map.getRows()) {
                    // Consider tileId == -1 as walkable, tileId >= 0 as obstacle wall
                    if (map.getTile(nextCol, nextRow) < 0) {
                        PathNode2D next{nextCol, nextRow};
                        if (cameFrom.find(next) == cameFrom.end()) {
                            frontier.push(next);
                            cameFrom[next] = current;
                        }
                    }
                }
            }
        }

        if (cameFrom.find(target) == cameFrom.end()) return path;

        PathNode2D curr = target;
        while (!(curr == start)) {
            path.push_back(curr);
            curr = cameFrom[curr];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }
};

} // namespace zenith

#endif // ZENITH_PATHFINDING_H
