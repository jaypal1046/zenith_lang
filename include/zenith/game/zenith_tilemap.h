#ifndef ZENITH_TILEMAP_H
#define ZENITH_TILEMAP_H

#include "zenith_window.h"
#include "zenith_texture.h"
#include <vector>
#include <iostream>
#include <cmath>

namespace zenith {

struct TileSet {
    Texture2D texture;
    int tileWidth = 32;
    int tileHeight = 32;
    int columns = 1;
    int rows = 1;

    SpriteRegion getTileRegion(int tileId) const {
        if (tileId < 0 || columns <= 0 || rows <= 0) return SpriteRegion{};
        int col = tileId % columns;
        int row = tileId / columns;

        SpriteRegion region;
        region.u0 = static_cast<float>(col * tileWidth) / static_cast<float>(texture.width > 0 ? texture.width : 1);
        region.v0 = static_cast<float>(row * tileHeight) / static_cast<float>(texture.height > 0 ? texture.height : 1);
        region.u1 = static_cast<float>((col + 1) * tileWidth) / static_cast<float>(texture.width > 0 ? texture.width : 1);
        region.v1 = static_cast<float>((row + 1) * tileHeight) / static_cast<float>(texture.height > 0 ? texture.height : 1);
        return region;
    }
};

class TileMap2D {
private:
    int m_cols = 0;
    int m_rows = 0;
    float m_tileSize = 32.0f;
    TileSet m_tileSet;
    std::vector<int> m_tiles;

public:
    TileMap2D() = default;
    TileMap2D(int cols, int rows, float tileSize, const TileSet& tileSet)
        : m_cols(cols), m_rows(rows), m_tileSize(tileSize), m_tileSet(tileSet) {
        m_tiles.resize(cols * rows, -1);
    }

    void setTile(int col, int row, int tileId) {
        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            m_tiles[row * m_cols + col] = tileId;
        }
    }

    int getTile(int col, int row) const {
        if (col >= 0 && col < m_cols && row >= 0 && row < m_rows) {
            return m_tiles[row * m_cols + col];
        }
        return -1;
    }

    void render(float viewMinX = -1000.0f, float viewMinY = -1000.0f, float viewMaxX = 2000.0f, float viewMaxY = 2000.0f) {
        int minCol = static_cast<int>(std::max(0.0f, std::floor(viewMinX / m_tileSize)));
        int maxCol = static_cast<int>(std::min(static_cast<float>(m_cols - 1), std::ceil(viewMaxX / m_tileSize)));
        int minRow = static_cast<int>(std::max(0.0f, std::floor(viewMinY / m_tileSize)));
        int maxRow = static_cast<int>(std::min(static_cast<float>(m_rows - 1), std::ceil(viewMaxY / m_tileSize)));

        for (int r = minRow; r <= maxRow; ++r) {
            for (int c = minCol; c <= maxCol; ++c) {
                int tileId = getTile(c, r);
                if (tileId >= 0) {
                    float posX = c * m_tileSize;
                    float posY = r * m_tileSize;

                    if (m_tileSet.texture.isValid()) {
                        SpriteRegion region = m_tileSet.getTileRegion(tileId);
                        TextureManager::drawSpriteRegion(m_tileSet.texture, region, posX, posY, m_tileSize, m_tileSize);
                    } else {
                        // Fallback shape drawing if texture not loaded
                        Renderer2D::drawRect(posX, posY, m_tileSize, m_tileSize, Color::Green());
                        Renderer2D::drawRectOutline(posX, posY, m_tileSize, m_tileSize, Color::Black(), 1.0f);
                    }
                }
            }
        }
    }

    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }
    float getTileSize() const { return m_tileSize; }
};

} // namespace zenith

#endif // ZENITH_TILEMAP_H
