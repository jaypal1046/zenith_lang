#ifndef ZENITH_VOXEL2D_H
#define ZENITH_VOXEL2D_H

#include <vector>
#include <cmath>
#include <algorithm>

namespace zenith {

enum class VoxelType : uint8_t {
    Empty = 0,
    Dirt = 1,
    Stone = 2,
    Sand = 3
};

class VoxelGrid2D {
private:
    int m_width = 0;
    int m_height = 0;
    std::vector<uint8_t> m_voxels;

public:
    VoxelGrid2D(int width, int height)
        : m_width(width), m_height(height), m_voxels(width * height, static_cast<uint8_t>(VoxelType::Empty)) {}

    void fill(VoxelType type) {
        std::fill(m_voxels.begin(), m_voxels.end(), static_cast<uint8_t>(type));
    }

    void setVoxel(int x, int y, VoxelType type) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_voxels[y * m_width + x] = static_cast<uint8_t>(type);
        }
    }

    VoxelType getVoxel(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return VoxelType::Empty;
        return static_cast<VoxelType>(m_voxels[y * m_width + x]);
    }

    void carveCircle(int cx, int cy, int radius) {
        int rSq = radius * radius;
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx + dy * dy <= rSq) {
                    setVoxel(x, y, VoxelType::Empty);
                }
            }
        }
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
};

} // namespace zenith

#endif // ZENITH_VOXEL2D_H
