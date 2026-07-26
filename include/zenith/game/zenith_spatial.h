#ifndef ZENITH_SPATIAL_H
#define ZENITH_SPATIAL_H

#include "../common/zenith_common.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {
namespace spatial {

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
};

struct Rect2D {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    bool contains(const Point2D& point) const {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }

    bool intersects(const Rect2D& other) const {
        return !(x + width < other.x || other.x + other.width < x ||
                 y + height < other.y || other.y + other.height < y);
    }
};

struct Ray2D {
    Point2D origin;
    Point2D direction; // normalized
};

struct RaycastHit2D {
    bool hit = false;
    float distance = 0.0f;
    Point2D point;
    std::uint32_t entityID = 0;
};

// High-performance 2D Spatial Hash Grid for O(N log N) spatial queries
template <typename T>
class SpatialHashGrid2D {
private:
    float m_cellSize;
    std::unordered_map<std::int64_t, std::vector<T>> m_grid;

    std::int64_t getCellHash(int cellX, int cellY) const {
        return (static_cast<std::int64_t>(cellX) * 73856093) ^ (static_cast<std::int64_t>(cellY) * 19349663);
    }

public:
    explicit SpatialHashGrid2D(float cellSize = 64.0f) : m_cellSize(cellSize) {}

    void clear() {
        m_grid.clear();
    }

    void insert(float x, float y, float width, float height, const T& item) {
        int minCellX = static_cast<int>(std::floor(x / m_cellSize));
        int maxCellX = static_cast<int>(std::floor((x + width) / m_cellSize));
        int minCellY = static_cast<int>(std::floor(y / m_cellSize));
        int maxCellY = static_cast<int>(std::floor((y + height) / m_cellSize));

        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            for (int cy = minCellY; cy <= maxCellY; ++cy) {
                std::int64_t key = getCellHash(cx, cy);
                m_grid[key].push_back(item);
            }
        }
    }

    std::vector<T> query(float x, float y, float width, float height) const {
        std::vector<T> result;
        int minCellX = static_cast<int>(std::floor(x / m_cellSize));
        int maxCellX = static_cast<int>(std::floor((x + width) / m_cellSize));
        int minCellY = static_cast<int>(std::floor(y / m_cellSize));
        int maxCellY = static_cast<int>(std::floor((y + height) / m_cellSize));

        for (int cx = minCellX; cx <= maxCellX; ++cx) {
            for (int cy = minCellY; cy <= maxCellY; ++cy) {
                std::int64_t key = getCellHash(cx, cy);
                auto it = m_grid.find(key);
                if (it != m_grid.end()) {
                    for (const auto& item : it->second) {
                        if (std::find(result.begin(), result.end(), item) == result.end()) {
                            result.push_back(item);
                        }
                    }
                }
            }
        }
        return result;
    }
};

} // namespace spatial
} // namespace zenith

#endif // ZENITH_SPATIAL_H
