#ifndef ZENITH_ASSET_PACK_H
#define ZENITH_ASSET_PACK_H

#include "zenith_texture.h"
#include "zenith_audio.h"
#include <string>
#include <unordered_map>
#include <iostream>

namespace zenith {

class AssetPack {
private:
    std::unordered_map<std::string, Texture2D> m_textures;
    std::unordered_map<std::string, Sound> m_sounds;

public:
    AssetPack() = default;

    Texture2D getOrCreateTexture(const std::string& key) {
        auto it = m_textures.find(key);
        if (it != m_textures.end()) {
            return it->second;
        }
        Texture2D tex = TextureManager::createDummyProceduralTexture(64, 64, Color::Cyan());
        m_textures[key] = tex;
        return tex;
    }

    void unloadAll() {
        m_textures.clear();
        m_sounds.clear();
    }

    std::size_t getLoadedTextureCount() const { return m_textures.size(); }
};

} // namespace zenith

#endif // ZENITH_ASSET_PACK_H
