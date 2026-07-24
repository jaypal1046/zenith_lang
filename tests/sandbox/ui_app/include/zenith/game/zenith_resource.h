#ifndef ZENITH_RESOURCE_H
#define ZENITH_RESOURCE_H

#include "../common/zenith_common.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>
#include <vector>

namespace zenith {
namespace resource {

enum class AssetType {
    Texture,
    Audio,
    Mesh,
    Shader,
    Material,
    Font,
    Particles
};

// Handle to an Asset managed by the Resource System
struct AssetHandle {
    std::string path;
    AssetType type;
    int id = 0;
    int refCount = 1;
    bool isLoaded = false;
};

class TextureAsset {
public:
    std::string path;
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned int gpuId = 0;

    TextureAsset(const std::string& path) : path(path) {
        // Mock load or bind GPU texture ID
        gpuId = 1; 
    }
};

class AudioAsset {
public:
    std::string path;
    float duration = 0.0f;
    bool isSpatial = false;

    AudioAsset(const std::string& path, bool spatial = false) 
        : path(path), isSpatial(spatial) {}
};

class MeshAsset {
public:
    std::string path;
    int vertexCount = 0;
    int triangleCount = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;

    MeshAsset(const std::string& path) : path(path) {}
};

class ShaderAsset {
public:
    std::string path;
    unsigned int programId = 0;

    ShaderAsset(const std::string& path) : path(path) {}
};

enum class MaterialPropertyKind {
    Text,
    Number,
    Toggle,
    Radio,
    Image,
    Button,
    Color
};

struct MaterialPropertyOption {
    std::string label;
    std::string value;
};

struct MaterialProperty {
    std::string name;
    std::string label;
    MaterialPropertyKind kind = MaterialPropertyKind::Text;
    std::string stringValue;
    float numberValue = 0.0f;
    bool boolValue = false;
    std::vector<MaterialPropertyOption> options;
};

class MaterialAsset {
public:
    std::string path;
    std::string shaderPath;
    std::vector<MaterialProperty> properties;

    MaterialAsset(const std::string& path, const std::string& shaderPath = "")
        : path(path), shaderPath(shaderPath) {}

    bool hasProperty(const std::string& name) const {
        return findProperty(name) != nullptr;
    }

    MaterialProperty* findProperty(const std::string& name) {
        for (auto& property : properties) {
            if (property.name == name) {
                return &property;
            }
        }
        return nullptr;
    }

    const MaterialProperty* findProperty(const std::string& name) const {
        for (const auto& property : properties) {
            if (property.name == name) {
                return &property;
            }
        }
        return nullptr;
    }

    MaterialProperty& defineText(const std::string& name, const std::string& label, const std::string& defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Text, name, label);
        property.stringValue = defaultValue;
        return property;
    }

    MaterialProperty& defineNumber(const std::string& name, const std::string& label, float defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Number, name, label);
        property.numberValue = defaultValue;
        return property;
    }

    MaterialProperty& defineToggle(const std::string& name, const std::string& label, bool defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Toggle, name, label);
        property.boolValue = defaultValue;
        return property;
    }

    MaterialProperty& defineRadio(const std::string& name, const std::string& label, const std::string& optionsCsv, const std::string& defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Radio, name, label);
        property.options = parseOptions(optionsCsv);
        property.stringValue = defaultValue;
        return property;
    }

    MaterialProperty& defineImage(const std::string& name, const std::string& label, const std::string& defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Image, name, label);
        property.stringValue = defaultValue;
        return property;
    }

    MaterialProperty& defineButton(const std::string& name, const std::string& label, const std::string& actionValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Button, name, label);
        property.stringValue = actionValue;
        return property;
    }

    MaterialProperty& defineColor(const std::string& name, const std::string& label, const std::string& defaultValue) {
        MaterialProperty& property = upsertProperty(MaterialPropertyKind::Color, name, label);
        property.stringValue = defaultValue;
        return property;
    }

    bool setText(const std::string& name, const std::string& value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->stringValue = value;
        return true;
    }

    bool setNumber(const std::string& name, float value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->numberValue = value;
        return true;
    }

    bool setToggle(const std::string& name, bool value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->boolValue = value;
        return true;
    }

    bool setRadio(const std::string& name, const std::string& value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->stringValue = value;
        return true;
    }

    bool setImage(const std::string& name, const std::string& value) {
        return setText(name, value);
    }

    bool setButton(const std::string& name, const std::string& value) {
        return setText(name, value);
    }

    bool setColor(const std::string& name, const std::string& value) {
        return setText(name, value);
    }

    std::string text(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->stringValue;
    }

    float number(const std::string& name, float fallback = 0.0f) const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->numberValue;
    }

    bool toggle(const std::string& name, bool fallback = false) const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->boolValue;
    }

    std::string radio(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->stringValue;
    }

    std::string image(const std::string& name, const std::string& fallback = "") const {
        return text(name, fallback);
    }

    std::string button(const std::string& name, const std::string& fallback = "") const {
        return text(name, fallback);
    }

    std::string color(const std::string& name, const std::string& fallback = "") const {
        return text(name, fallback);
    }

    int propertyCount() const {
        return static_cast<int>(properties.size());
    }

private:
    static std::string trim(const std::string& value) {
        size_t start = value.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }
        size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(start, end - start + 1);
    }

    static std::vector<MaterialPropertyOption> parseOptions(const std::string& optionsCsv) {
        std::vector<MaterialPropertyOption> options;
        std::stringstream stream(optionsCsv);
        std::string item;
        while (std::getline(stream, item, ',')) {
            std::string trimmed = trim(item);
            if (!trimmed.empty()) {
                options.push_back(MaterialPropertyOption{trimmed, trimmed});
            }
        }
        return options;
    }

    MaterialProperty& upsertProperty(MaterialPropertyKind kind, const std::string& name, const std::string& label) {
        MaterialProperty* existing = findProperty(name);
        if (existing != nullptr) {
            existing->kind = kind;
            existing->label = label;
            existing->options.clear();
            return *existing;
        }

        properties.push_back(MaterialProperty{});
        MaterialProperty& property = properties.back();
        property.name = name;
        property.label = label;
        property.kind = kind;
        return property;
    }
};

// Unified Code-First Resource Pipeline
class ResourceManager {
private:
    std::unordered_map<std::string, std::shared_ptr<TextureAsset>> textures;
    std::unordered_map<std::string, std::shared_ptr<AudioAsset>> audioClips;
    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
    std::unordered_map<std::string, std::shared_ptr<ShaderAsset>> shaders;
    std::unordered_map<std::string, std::shared_ptr<MaterialAsset>> materials;

public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    std::shared_ptr<TextureAsset> loadTexture(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) {
            return it->second;
        }
        auto tex = std::make_shared<TextureAsset>(path);
        textures[path] = tex;
        std::cout << "[Resource] Loaded Texture: " << path << std::endl;
        return tex;
    }

    std::shared_ptr<AudioAsset> loadAudio(const std::string& path, bool isSpatial = false) {
        auto it = audioClips.find(path);
        if (it != audioClips.end()) {
            return it->second;
        }
        auto audio = std::make_shared<AudioAsset>(path, isSpatial);
        audioClips[path] = audio;
        std::cout << "[Resource] Loaded Audio: " << path << std::endl;
        return audio;
    }

    std::shared_ptr<MeshAsset> loadMesh(const std::string& path) {
        auto it = meshes.find(path);
        if (it != meshes.end()) {
            return it->second;
        }
        auto mesh = std::make_shared<MeshAsset>(path);
        meshes[path] = mesh;
        std::cout << "[Resource] Loaded 3D Mesh: " << path << std::endl;
        return mesh;
    }

    std::shared_ptr<ShaderAsset> loadShader(const std::string& path) {
        auto it = shaders.find(path);
        if (it != shaders.end()) {
            return it->second;
        }
        auto shader = std::make_shared<ShaderAsset>(path);
        shaders[path] = shader;
        std::cout << "[Resource] Loaded Shader: " << path << std::endl;
        return shader;
    }

    bool hasMaterial(const std::string& path) const {
        return materials.find(path) != materials.end();
    }

    std::shared_ptr<MaterialAsset> loadMaterial(const std::string& path, const std::string& shaderPath = "") {
        auto it = materials.find(path);
        if (it != materials.end()) {
            if (!shaderPath.empty()) {
                it->second->shaderPath = shaderPath;
            }
            return it->second;
        }
        auto material = std::make_shared<MaterialAsset>(path, shaderPath);
        materials[path] = material;
        std::cout << "[Resource] Loaded Material: " << path << std::endl;
        return material;
    }

    void clearCache() {
        textures.clear();
        audioClips.clear();
        meshes.clear();
        shaders.clear();
        materials.clear();
    }
};

} // namespace resource
} // namespace zenith

#endif // ZENITH_RESOURCE_H
