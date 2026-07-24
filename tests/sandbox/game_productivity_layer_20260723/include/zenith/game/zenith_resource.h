#ifndef ZENITH_RESOURCE_H
#define ZENITH_RESOURCE_H

#include "../common/zenith_common.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace zenith {
namespace resource {

enum class AssetType {
    Unknown,
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
    AssetType type = AssetType::Unknown;
    int id = 0;
    int refCount = 0;
    bool isLoaded = false;
};

class TextureAsset {
public:
    std::string path;
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned int gpuId = 0;
    size_t memoryBytes = 0;
    int version = 1;

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
    size_t memoryBytes = 0;
    int version = 1;

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
    size_t memoryBytes = 0;
    int version = 1;

    MeshAsset(const std::string& path) : path(path) {}
};

class ShaderAsset {
public:
    std::string path;
    unsigned int programId = 0;
    size_t memoryBytes = 0;
    int version = 1;

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
    std::string callbackName;
    int triggerCount = 0;
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

    bool removeProperty(const std::string& name) {
        auto next = std::remove_if(properties.begin(), properties.end(), [&name](const MaterialProperty& property) {
            return property.name == name;
        });
        if (next == properties.end()) {
            return false;
        }
        properties.erase(next, properties.end());
        return true;
    }

    int clearProperties() {
        int removed = static_cast<int>(properties.size());
        properties.clear();
        return removed;
    }

    int copyPropertiesFrom(const MaterialAsset& source) {
        properties = source.properties;
        return static_cast<int>(properties.size());
    }

    bool renameProperty(const std::string& name, const std::string& nextName) {
        if (nextName.empty()) {
            return false;
        }
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        if (name == nextName) {
            return true;
        }
        if (findProperty(nextName) != nullptr) {
            return false;
        }
        property->name = nextName;
        return true;
    }

    bool setPropertyLabel(const std::string& name, const std::string& label) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->label = label;
        return true;
    }

    bool setPropertyKindName(const std::string& name, const std::string& kindName) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        MaterialPropertyKind kind = MaterialPropertyKind::Text;
        if (!tryParseMaterialPropertyKindName(kindName, kind)) {
            return false;
        }
        property->kind = kind;
        if (kind != MaterialPropertyKind::Radio) {
            property->options.clear();
        }
        return true;
    }

    bool setPropertyOptionsCsv(const std::string& name, const std::string& optionsCsv) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->options = parseOptions(optionsCsv);
        return true;
    }

    bool setPropertyCallback(const std::string& name, const std::string& callbackName) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->callbackName = callbackName;
        return true;
    }

    std::string propertyCallback(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->callbackName;
    }

    bool notifyProperty(const std::string& name, std::string* callbackName = nullptr, std::string* callbackValue = nullptr) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr || property->callbackName.empty()) {
            return false;
        }
        property->triggerCount += 1;
        if (callbackName != nullptr) {
            *callbackName = property->callbackName;
        }
        if (callbackValue != nullptr) {
            *callbackValue = serializePropertyValue(*property);
        }
        return true;
    }

    bool triggerButton(const std::string& name, std::string* actionValue = nullptr) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr || property->kind != MaterialPropertyKind::Button) {
            return false;
        }
        property->triggerCount += 1;
        if (actionValue != nullptr) {
            *actionValue = property->stringValue;
        }
        return true;
    }

    int propertyTriggerCount(const std::string& name) const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? 0 : property->triggerCount;
    }

    bool setPropertyTriggerCount(const std::string& name, int value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        property->triggerCount = std::max(0, value);
        return true;
    }

    std::string propertyNameAt(int index, const std::string& fallback = "") const {
        if (index < 0 || index >= static_cast<int>(properties.size())) {
            return fallback;
        }
        return properties[static_cast<size_t>(index)].name;
    }

    std::string propertyLabel(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : property->label;
    }

    std::string propertyKindName(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : materialPropertyKindName(property->kind);
    }

    std::string propertyOptionsCsv(const std::string& name, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? fallback : joinOptions(property->options);
    }

    int propertyOptionCount(const std::string& name) const {
        const MaterialProperty* property = findProperty(name);
        return property == nullptr ? 0 : static_cast<int>(property->options.size());
    }

    std::string propertyOptionLabel(const std::string& name, int index, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        if (property == nullptr || index < 0 || index >= static_cast<int>(property->options.size())) {
            return fallback;
        }
        const MaterialPropertyOption& option = property->options[static_cast<size_t>(index)];
        return option.label.empty() ? option.value : option.label;
    }

    std::string propertyOptionValue(const std::string& name, int index, const std::string& fallback = "") const {
        const MaterialProperty* property = findProperty(name);
        if (property == nullptr || index < 0 || index >= static_cast<int>(property->options.size())) {
            return fallback;
        }
        return property->options[static_cast<size_t>(index)].value;
    }

    bool setPropertyOptionLabel(const std::string& name, int index, const std::string& label) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr || index < 0 || index >= static_cast<int>(property->options.size())) {
            return false;
        }
        property->options[static_cast<size_t>(index)].label = label;
        return true;
    }

    bool setPropertyOptionValue(const std::string& name, int index, const std::string& value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr || index < 0 || index >= static_cast<int>(property->options.size())) {
            return false;
        }
        property->options[static_cast<size_t>(index)].value = value;
        return true;
    }

    bool addPropertyOption(const std::string& name, const std::string& label, const std::string& value) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return false;
        }
        std::string resolvedValue = value.empty() ? label : value;
        if (resolvedValue.empty()) {
            return false;
        }
        std::string resolvedLabel = label.empty() ? resolvedValue : label;
        property->options.push_back(MaterialPropertyOption{resolvedLabel, resolvedValue});
        return true;
    }

    bool removePropertyOption(const std::string& name, int index) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr || index < 0 || index >= static_cast<int>(property->options.size())) {
            return false;
        }
        property->options.erase(property->options.begin() + index);
        return true;
    }

    int clearPropertyOptions(const std::string& name) {
        MaterialProperty* property = findProperty(name);
        if (property == nullptr) {
            return 0;
        }
        int removed = static_cast<int>(property->options.size());
        property->options.clear();
        return removed;
    }

    int propertyCount() const {
        return static_cast<int>(properties.size());
    }

private:
    static std::string materialPropertyKindName(MaterialPropertyKind kind) {
        switch (kind) {
            case MaterialPropertyKind::Text:
                return "Text";
            case MaterialPropertyKind::Number:
                return "Number";
            case MaterialPropertyKind::Toggle:
                return "Toggle";
            case MaterialPropertyKind::Radio:
                return "Radio";
            case MaterialPropertyKind::Image:
                return "Image";
            case MaterialPropertyKind::Button:
                return "Button";
            case MaterialPropertyKind::Color:
                return "Color";
        }
        return "Text";
    }

    static bool tryParseMaterialPropertyKindName(const std::string& kindName, MaterialPropertyKind& kind) {
        if (kindName == "Text") {
            kind = MaterialPropertyKind::Text;
            return true;
        }
        if (kindName == "Number") {
            kind = MaterialPropertyKind::Number;
            return true;
        }
        if (kindName == "Toggle") {
            kind = MaterialPropertyKind::Toggle;
            return true;
        }
        if (kindName == "Radio") {
            kind = MaterialPropertyKind::Radio;
            return true;
        }
        if (kindName == "Image") {
            kind = MaterialPropertyKind::Image;
            return true;
        }
        if (kindName == "Button") {
            kind = MaterialPropertyKind::Button;
            return true;
        }
        if (kindName == "Color") {
            kind = MaterialPropertyKind::Color;
            return true;
        }
        return false;
    }

    static std::string trim(const std::string& value) {
        size_t start = value.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }
        size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(start, end - start + 1);
    }

    static std::string joinOptions(const std::vector<MaterialPropertyOption>& options) {
        std::string csv;
        for (size_t i = 0; i < options.size(); ++i) {
            if (i > 0) {
                csv += ",";
            }
            csv += options[i].value;
        }
        return csv;
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

    static std::string serializePropertyValue(const MaterialProperty& property) {
        if (property.kind == MaterialPropertyKind::Number) {
            std::ostringstream stream;
            stream << property.numberValue;
            return stream.str();
        }
        if (property.kind == MaterialPropertyKind::Toggle) {
            return property.boolValue ? "true" : "false";
        }
        return property.stringValue;
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

struct ImportedAssetMetadata {
    std::string sourcePath;
    std::string importedPath;
    AssetType type = AssetType::Unknown;
    std::string group;
    size_t sourceBytes = 0;
    size_t estimatedMemoryBytes = 0;
    uint64_t sourceTimestamp = 0;
    uint64_t importedTimestamp = 0;
    int version = 1;
    bool hotReloadable = true;
    bool dirty = false;
};

struct AssetBundle {
    std::string name;
    std::vector<std::string> assetPaths;
};

inline std::string normalizeAssetPath(const std::string& path) {
    if (path.empty()) {
        return "";
    }
    std::filesystem::path normalized = std::filesystem::path(path).lexically_normal();
    return normalized.generic_string();
}

inline std::string assetTypeName(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "Texture";
        case AssetType::Audio: return "Audio";
        case AssetType::Mesh: return "Mesh";
        case AssetType::Shader: return "Shader";
        case AssetType::Material: return "Material";
        case AssetType::Font: return "Font";
        case AssetType::Particles: return "Particles";
        default: return "Unknown";
    }
}

inline std::string assetTypeFolderName(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "textures";
        case AssetType::Audio: return "audio";
        case AssetType::Mesh: return "meshes";
        case AssetType::Shader: return "shaders";
        case AssetType::Material: return "materials";
        case AssetType::Font: return "fonts";
        case AssetType::Particles: return "particles";
        default: return "misc";
    }
}

inline std::string assetLowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

inline bool parseAssetType(const std::string& value, AssetType& type) {
    const std::string normalized = assetLowercase(value);
    if (normalized == "texture" || normalized == "textures" || normalized == "image" || normalized == "images") {
        type = AssetType::Texture;
        return true;
    }
    if (normalized == "audio" || normalized == "sound" || normalized == "sounds" || normalized == "clip") {
        type = AssetType::Audio;
        return true;
    }
    if (normalized == "mesh" || normalized == "meshes" || normalized == "model" || normalized == "models") {
        type = AssetType::Mesh;
        return true;
    }
    if (normalized == "shader" || normalized == "shaders") {
        type = AssetType::Shader;
        return true;
    }
    if (normalized == "material" || normalized == "materials" || normalized == "mat") {
        type = AssetType::Material;
        return true;
    }
    if (normalized == "font" || normalized == "fonts") {
        type = AssetType::Font;
        return true;
    }
    if (normalized == "particle" || normalized == "particles" || normalized == "vfx") {
        type = AssetType::Particles;
        return true;
    }
    if (normalized == "unknown" || normalized == "misc") {
        type = AssetType::Unknown;
        return true;
    }
    return false;
}

inline AssetType guessAssetTypeFromPath(const std::string& path) {
    const std::string extension = assetLowercase(std::filesystem::path(path).extension().string());
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" ||
        extension == ".gif" || extension == ".tga" || extension == ".dds" || extension == ".webp") {
        return AssetType::Texture;
    }
    if (extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac") {
        return AssetType::Audio;
    }
    if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || extension == ".glb" || extension == ".dae" || extension == ".mesh") {
        return AssetType::Mesh;
    }
    if (extension == ".shader" || extension == ".vert" || extension == ".frag" || extension == ".glsl" || extension == ".hlsl") {
        return AssetType::Shader;
    }
    if (extension == ".mat" || extension == ".material") {
        return AssetType::Material;
    }
    if (extension == ".ttf" || extension == ".otf" || extension == ".fnt") {
        return AssetType::Font;
    }
    if (extension == ".particle" || extension == ".particles" || extension == ".vfx") {
        return AssetType::Particles;
    }
    return AssetType::Unknown;
}

inline size_t assetFileSizeBytes(const std::string& path) {
    if (path.empty()) {
        return 0;
    }
    std::error_code error;
    const std::filesystem::path filePath(path);
    if (!std::filesystem::exists(filePath, error)) {
        return 0;
    }
    const uintmax_t size = std::filesystem::file_size(filePath, error);
    if (error) {
        return 0;
    }
    return static_cast<size_t>(size);
}

inline uint64_t assetFileTimestamp(const std::string& path) {
    if (path.empty()) {
        return 0;
    }
    std::error_code error;
    const std::filesystem::path filePath(path);
    if (!std::filesystem::exists(filePath, error)) {
        return 0;
    }
    const auto stamp = std::filesystem::last_write_time(filePath, error);
    if (error) {
        return 0;
    }
    const auto ticks = stamp.time_since_epoch().count();
    return ticks < 0 ? static_cast<uint64_t>(-ticks) : static_cast<uint64_t>(ticks);
}

inline size_t defaultAssetMemoryBytes(AssetType type) {
    switch (type) {
        case AssetType::Texture: return 4 * 1024;
        case AssetType::Audio: return 2 * 1024;
        case AssetType::Mesh: return 6 * 1024;
        case AssetType::Shader: return 1024;
        case AssetType::Material: return 1024;
        case AssetType::Font: return 2 * 1024;
        case AssetType::Particles: return 1024;
        default: return 1024;
    }
}

inline size_t estimateAssetMemoryBytes(AssetType type, size_t sourceBytes) {
    const size_t seed = sourceBytes > 0 ? sourceBytes : defaultAssetMemoryBytes(type);
    switch (type) {
        case AssetType::Texture: return std::max<size_t>(4 * 1024, seed * 4);
        case AssetType::Audio: return std::max<size_t>(2 * 1024, seed * 2);
        case AssetType::Mesh: return std::max<size_t>(6 * 1024, seed * 3);
        case AssetType::Shader: return std::max<size_t>(1024, seed);
        case AssetType::Material: return std::max<size_t>(1024, seed);
        case AssetType::Font: return std::max<size_t>(2 * 1024, seed * 2);
        case AssetType::Particles: return std::max<size_t>(1024, seed * 2);
        default: return std::max<size_t>(1024, seed);
    }
}

inline std::string escapeAssetDatabaseField(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '\t': escaped += "\\t"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

inline std::string unescapeAssetDatabaseField(const std::string& value) {
    std::string unescaped;
    unescaped.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            ++i;
            switch (value[i]) {
                case 't': unescaped.push_back('\t'); break;
                case 'n': unescaped.push_back('\n'); break;
                case 'r': unescaped.push_back('\r'); break;
                case '\\': unescaped.push_back('\\'); break;
                default:
                    unescaped.push_back('\\');
                    unescaped.push_back(value[i]);
                    break;
            }
        } else {
            unescaped.push_back(value[i]);
        }
    }
    return unescaped;
}

inline std::string escapeAssetJson(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// Unified Code-First Resource Pipeline
class ResourceManager {
private:
    std::unordered_map<std::string, std::shared_ptr<TextureAsset>> textures;
    std::unordered_map<std::string, std::shared_ptr<AudioAsset>> audioClips;
    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
    std::unordered_map<std::string, std::shared_ptr<ShaderAsset>> shaders;
    std::unordered_map<std::string, std::shared_ptr<MaterialAsset>> materials;
    std::unordered_map<std::string, AssetHandle> textureHandles;
    std::unordered_map<std::string, AssetHandle> audioHandles;
    std::unordered_map<std::string, AssetHandle> meshHandles;
    std::unordered_map<std::string, AssetHandle> shaderHandles;
    std::unordered_map<std::string, AssetHandle> materialHandles;
    std::unordered_map<std::string, ImportedAssetMetadata> importedMetadata;
    std::unordered_map<std::string, AssetBundle> bundles;
    std::unordered_map<int, size_t> memoryBudgets;
    bool assetHotReloadEnabled = true;
    int nextHandleId = 1;

    AssetHandle& ensureHandle(
        std::unordered_map<std::string, AssetHandle>& handles,
        const std::string& path,
        AssetType type) {
        auto it = handles.find(path);
        if (it == handles.end()) {
            AssetHandle handle;
            handle.path = path;
            handle.type = type;
            handle.id = nextHandleId++;
            handle.refCount = 1;
            handle.isLoaded = false;
            it = handles.emplace(path, handle).first;
        }
        it->second.path = path;
        it->second.type = type;
        return it->second;
    }

    AssetHandle copyHandle(
        std::unordered_map<std::string, AssetHandle>& handles,
        const std::string& path,
        AssetType type,
        bool isLoaded) {
        if (path.empty()) {
            return AssetHandle{};
        }
        AssetHandle& handle = ensureHandle(handles, path, type);
        handle.isLoaded = isLoaded;
        handle.refCount = isLoaded ? 1 : 0;
        return handle;
    }

    ImportedAssetMetadata* findMetadataRecord(const std::string& assetPath) {
        const std::string normalized = normalizeAssetPath(assetPath);
        auto direct = importedMetadata.find(normalized);
        if (direct != importedMetadata.end()) {
            return &direct->second;
        }
        for (auto& entry : importedMetadata) {
            if (entry.second.sourcePath == normalized) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    const ImportedAssetMetadata* findMetadataRecord(const std::string& assetPath) const {
        const std::string normalized = normalizeAssetPath(assetPath);
        auto direct = importedMetadata.find(normalized);
        if (direct != importedMetadata.end()) {
            return &direct->second;
        }
        for (const auto& entry : importedMetadata) {
            if (entry.second.sourcePath == normalized) {
                return &entry.second;
            }
        }
        return nullptr;
    }

    ImportedAssetMetadata& ensureMetadataRecord(const std::string& assetPath, AssetType requestedType) {
        const std::string normalized = normalizeAssetPath(assetPath);
        ImportedAssetMetadata* existing = findMetadataRecord(normalized);
        if (existing != nullptr) {
            if (existing->type == AssetType::Unknown && requestedType != AssetType::Unknown) {
                existing->type = requestedType;
            }
            if (existing->sourcePath.empty()) {
                existing->sourcePath = normalized;
            }
            if (existing->importedPath.empty()) {
                existing->importedPath = normalized;
            }
            if (existing->sourceBytes == 0) {
                existing->sourceBytes = assetFileSizeBytes(existing->sourcePath);
            }
            if (existing->estimatedMemoryBytes == 0) {
                existing->estimatedMemoryBytes = estimateAssetMemoryBytes(existing->type, existing->sourceBytes);
            }
            if (existing->sourceTimestamp == 0) {
                existing->sourceTimestamp = assetFileTimestamp(existing->sourcePath);
            }
            if (existing->importedTimestamp == 0) {
                existing->importedTimestamp = assetFileTimestamp(existing->importedPath);
            }
            return *existing;
        }

        ImportedAssetMetadata metadata;
        metadata.sourcePath = normalized;
        metadata.importedPath = normalized;
        metadata.type = requestedType == AssetType::Unknown ? guessAssetTypeFromPath(normalized) : requestedType;
        metadata.sourceBytes = assetFileSizeBytes(normalized);
        metadata.estimatedMemoryBytes = estimateAssetMemoryBytes(metadata.type, metadata.sourceBytes);
        metadata.sourceTimestamp = assetFileTimestamp(normalized);
        metadata.importedTimestamp = metadata.sourceTimestamp;
        auto inserted = importedMetadata.emplace(normalized, metadata);
        return inserted.first->second;
    }

    size_t memoryUsageForType(AssetType type) const {
        size_t total = 0;
        if (type == AssetType::Texture) {
            for (const auto& entry : textures) {
                total += entry.second ? entry.second->memoryBytes : 0;
            }
        } else if (type == AssetType::Audio) {
            for (const auto& entry : audioClips) {
                total += entry.second ? entry.second->memoryBytes : 0;
            }
        } else if (type == AssetType::Mesh) {
            for (const auto& entry : meshes) {
                total += entry.second ? entry.second->memoryBytes : 0;
            }
        } else if (type == AssetType::Shader) {
            for (const auto& entry : shaders) {
                total += entry.second ? entry.second->memoryBytes : 0;
            }
        } else if (type == AssetType::Material) {
            for (const auto& entry : materials) {
                total += entry.second ? entry.second->properties.size() * sizeof(MaterialProperty) : 0;
            }
        }
        return total;
    }

    size_t loadedAssetBytes(AssetType type, const std::string& path) const {
        if (type == AssetType::Texture) {
            auto it = textures.find(path);
            return it == textures.end() || !it->second ? 0 : it->second->memoryBytes;
        }
        if (type == AssetType::Audio) {
            auto it = audioClips.find(path);
            return it == audioClips.end() || !it->second ? 0 : it->second->memoryBytes;
        }
        if (type == AssetType::Mesh) {
            auto it = meshes.find(path);
            return it == meshes.end() || !it->second ? 0 : it->second->memoryBytes;
        }
        if (type == AssetType::Shader) {
            auto it = shaders.find(path);
            return it == shaders.end() || !it->second ? 0 : it->second->memoryBytes;
        }
        if (type == AssetType::Material) {
            auto it = materials.find(path);
            return it == materials.end() || !it->second ? 0 : it->second->properties.size() * sizeof(MaterialProperty);
        }
        return 0;
    }

    bool canFitBudget(AssetType type, const std::string& path, size_t requestedBytes) const {
        const size_t budget = memoryBudget(type);
        if (budget == 0) {
            return true;
        }
        const size_t currentUsage = memoryUsageForType(type);
        const size_t existingUsage = loadedAssetBytes(type, path);
        return currentUsage - existingUsage + requestedBytes <= budget;
    }

    void warnBudgetExceeded(AssetType type, const std::string& path, size_t requestedBytes) const {
        std::cout
            << "[Resource] Skipped "
            << assetTypeName(type)
            << " load for "
            << path
            << " because it would exceed the "
            << assetTypeName(type)
            << " memory budget (requested="
            << requestedBytes
            << ", budget="
            << memoryBudget(type)
            << ")."
            << std::endl;
    }

    void refreshMetadataAfterLoad(const std::string& path, AssetType type, size_t memoryBytes) {
        ImportedAssetMetadata& metadata = ensureMetadataRecord(path, type);
        metadata.type = type;
        if (metadata.sourceBytes == 0) {
            metadata.sourceBytes = assetFileSizeBytes(metadata.sourcePath);
        }
        metadata.estimatedMemoryBytes = memoryBytes > 0 ? memoryBytes : estimateAssetMemoryBytes(type, metadata.sourceBytes);
        metadata.importedTimestamp = assetFileTimestamp(metadata.importedPath);
        metadata.sourceTimestamp = assetFileTimestamp(metadata.sourcePath);
        metadata.dirty = false;
        if (metadata.version < 1) {
            metadata.version = 1;
        }
    }

    bool copyImportedFile(const std::filesystem::path& sourcePath, const std::filesystem::path& importedPath) const {
        std::error_code error;
        if (!importedPath.parent_path().empty()) {
            std::filesystem::create_directories(importedPath.parent_path(), error);
            if (error) {
                return false;
            }
        }

        error.clear();
        if (std::filesystem::exists(importedPath, error)) {
            error.clear();
            std::filesystem::remove(importedPath, error);
            if (error) {
                return false;
            }
        }

        error.clear();
        std::filesystem::copy_file(sourcePath, importedPath, std::filesystem::copy_options::none, error);
        return !error;
    }

    bool syncImportedCopy(ImportedAssetMetadata& metadata) {
        if (metadata.sourcePath.empty() || metadata.importedPath.empty()) {
            return false;
        }
        if (metadata.sourcePath == metadata.importedPath) {
            metadata.importedTimestamp = assetFileTimestamp(metadata.importedPath);
            return true;
        }

        std::error_code error;
        const std::filesystem::path sourcePath(metadata.sourcePath);
        const std::filesystem::path importedPath(metadata.importedPath);
        if (!std::filesystem::exists(sourcePath, error)) {
            return false;
        }
        if (!copyImportedFile(sourcePath, importedPath)) {
            return false;
        }
        metadata.importedTimestamp = assetFileTimestamp(metadata.importedPath);
        return true;
    }

public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    std::string importAsset(
        const std::string& sourcePath,
        const std::string& importedPath = "",
        const std::string& group = "",
        const std::string& bundleName = "") {
        const std::string normalizedSource = normalizeAssetPath(sourcePath);
        if (normalizedSource.empty()) {
            return "";
        }

        std::error_code error;
        const std::filesystem::path sourceFile(normalizedSource);
        if (!std::filesystem::exists(sourceFile, error)) {
            return "";
        }

        const AssetType type = guessAssetTypeFromPath(normalizedSource);
        std::string resolvedImportedPath = normalizeAssetPath(importedPath);
        if (resolvedImportedPath.empty()) {
            resolvedImportedPath = normalizeAssetPath(
                (std::filesystem::path("assets") /
                 "imported" /
                 assetTypeFolderName(type) /
                 sourceFile.filename()).generic_string());
        }

        ImportedAssetMetadata metadata;
        if (const ImportedAssetMetadata* existing = findMetadataRecord(normalizedSource)) {
            metadata = *existing;
            if (!existing->importedPath.empty() && existing->importedPath != resolvedImportedPath) {
                importedMetadata.erase(existing->importedPath);
            }
        }

        metadata.sourcePath = normalizedSource;
        metadata.importedPath = resolvedImportedPath;
        metadata.type = type;
        metadata.group = !group.empty() ? group : metadata.group;
        metadata.sourceBytes = assetFileSizeBytes(normalizedSource);
        metadata.estimatedMemoryBytes = estimateAssetMemoryBytes(type, metadata.sourceBytes);
        metadata.sourceTimestamp = assetFileTimestamp(normalizedSource);
        metadata.importedTimestamp = metadata.sourceTimestamp;
        metadata.version = std::max(1, metadata.version);
        metadata.hotReloadable = true;
        metadata.dirty = false;

        if (normalizedSource != resolvedImportedPath) {
            std::filesystem::path importedFile(resolvedImportedPath);
            if (!copyImportedFile(sourceFile, importedFile)) {
                return "";
            }
            metadata.importedTimestamp = assetFileTimestamp(resolvedImportedPath);
        }

        importedMetadata[resolvedImportedPath] = metadata;
        if (!bundleName.empty()) {
            addAssetToBundle(bundleName, resolvedImportedPath);
        }

        std::cout << "[Resource] Imported " << assetTypeName(type) << ": " << normalizedSource << " -> " << resolvedImportedPath << std::endl;
        return resolvedImportedPath;
    }

    bool hasImportedAsset(const std::string& assetPath) const {
        return findMetadataRecord(assetPath) != nullptr;
    }

    std::string importedAssetPath(const std::string& assetPath) const {
        const ImportedAssetMetadata* metadata = findMetadataRecord(assetPath);
        return metadata == nullptr ? "" : metadata->importedPath;
    }

    std::string assetGroup(const std::string& assetPath) const {
        const ImportedAssetMetadata* metadata = findMetadataRecord(assetPath);
        return metadata == nullptr ? "" : metadata->group;
    }

    bool createBundle(const std::string& bundleName) {
        if (bundleName.empty()) {
            return false;
        }
        auto it = bundles.find(bundleName);
        if (it == bundles.end()) {
            AssetBundle bundle;
            bundle.name = bundleName;
            bundles[bundleName] = bundle;
        }
        return true;
    }

    bool addAssetToBundle(const std::string& bundleName, const std::string& assetPath) {
        if (bundleName.empty() || assetPath.empty()) {
            return false;
        }
        createBundle(bundleName);
        AssetBundle& bundle = bundles[bundleName];
        const std::string storedPath = importedAssetPath(assetPath).empty() ? normalizeAssetPath(assetPath) : importedAssetPath(assetPath);
        ensureMetadataRecord(storedPath, guessAssetTypeFromPath(storedPath));
        if (std::find(bundle.assetPaths.begin(), bundle.assetPaths.end(), storedPath) == bundle.assetPaths.end()) {
            bundle.assetPaths.push_back(storedPath);
        }
        return true;
    }

    int bundleAssetCount(const std::string& bundleName) const {
        auto it = bundles.find(bundleName);
        return it == bundles.end() ? 0 : static_cast<int>(it->second.assetPaths.size());
    }

    std::string bundleAsset(const std::string& bundleName, int index) const {
        auto it = bundles.find(bundleName);
        if (it == bundles.end() || index < 0 || index >= static_cast<int>(it->second.assetPaths.size())) {
            return "";
        }
        return it->second.assetPaths[static_cast<size_t>(index)];
    }

    std::vector<ImportedAssetMetadata> importedAssets() const {
        std::vector<ImportedAssetMetadata> assets;
        assets.reserve(importedMetadata.size());
        for (const auto& entry : importedMetadata) {
            assets.push_back(entry.second);
        }
        std::sort(assets.begin(), assets.end(), [](const ImportedAssetMetadata& a, const ImportedAssetMetadata& b) {
            return a.importedPath < b.importedPath;
        });
        return assets;
    }

    std::vector<AssetBundle> assetBundles() const {
        std::vector<AssetBundle> output;
        output.reserve(bundles.size());
        for (const auto& entry : bundles) {
            output.push_back(entry.second);
        }
        std::sort(output.begin(), output.end(), [](const AssetBundle& a, const AssetBundle& b) {
            return a.name < b.name;
        });
        return output;
    }

    bool setMemoryBudgetByName(const std::string& typeName, size_t bytes) {
        AssetType type = AssetType::Unknown;
        if (!parseAssetType(typeName, type) || type == AssetType::Unknown) {
            return false;
        }
        memoryBudgets[static_cast<int>(type)] = bytes;
        return true;
    }

    size_t memoryBudgetByName(const std::string& typeName) const {
        AssetType type = AssetType::Unknown;
        return parseAssetType(typeName, type) ? memoryBudget(type) : 0;
    }

    size_t memoryBudget(AssetType type) const {
        auto it = memoryBudgets.find(static_cast<int>(type));
        return it == memoryBudgets.end() ? 0 : it->second;
    }

    size_t memoryUsageByName(const std::string& typeName) const {
        AssetType type = AssetType::Unknown;
        return parseAssetType(typeName, type) ? memoryUsageForType(type) : 0;
    }

    void setHotReloadEnabled(bool enabled) {
        assetHotReloadEnabled = enabled;
    }

    bool hotReloadEnabled() const {
        return assetHotReloadEnabled;
    }

    int pollForDirtyAssets() {
        int changed = 0;
        for (auto& entry : importedMetadata) {
            ImportedAssetMetadata& metadata = entry.second;
            if (!metadata.hotReloadable || metadata.sourcePath.empty()) {
                continue;
            }
            const uint64_t currentTimestamp = assetFileTimestamp(metadata.sourcePath);
            const size_t currentBytes = assetFileSizeBytes(metadata.sourcePath);
            const bool changedOnDisk =
                (currentBytes != metadata.sourceBytes) ||
                (currentTimestamp != 0 && currentTimestamp != metadata.sourceTimestamp);
            if (changedOnDisk && !metadata.dirty) {
                metadata.dirty = true;
                ++changed;
            }
        }
        return changed;
    }

    bool markAssetDirty(const std::string& assetPath) {
        ImportedAssetMetadata& metadata = ensureMetadataRecord(assetPath, guessAssetTypeFromPath(assetPath));
        metadata.dirty = true;
        return true;
    }

    bool reloadAsset(const std::string& assetPath) {
        ImportedAssetMetadata& metadata = ensureMetadataRecord(assetPath, guessAssetTypeFromPath(assetPath));
        const std::string targetPath = metadata.importedPath.empty() ? normalizeAssetPath(assetPath) : metadata.importedPath;
        const AssetType type = metadata.type == AssetType::Unknown ? guessAssetTypeFromPath(targetPath) : metadata.type;
        const size_t sourceBytes = metadata.sourcePath.empty() ? assetFileSizeBytes(targetPath) : assetFileSizeBytes(metadata.sourcePath);
        const size_t estimatedBytes = estimateAssetMemoryBytes(type, sourceBytes);
        if (!canFitBudget(type, targetPath, estimatedBytes)) {
            warnBudgetExceeded(type, targetPath, estimatedBytes);
            metadata.dirty = true;
            return false;
        }

        syncImportedCopy(metadata);
        metadata.type = type;
        metadata.sourceBytes = sourceBytes;
        metadata.estimatedMemoryBytes = estimatedBytes;
        metadata.sourceTimestamp = assetFileTimestamp(metadata.sourcePath);
        metadata.importedTimestamp = assetFileTimestamp(metadata.importedPath);
        metadata.version = std::max(1, metadata.version + 1);
        metadata.dirty = false;

        if (type == AssetType::Texture) {
            auto it = textures.find(targetPath);
            if (it != textures.end() && it->second) {
                it->second->memoryBytes = estimatedBytes;
                it->second->version = metadata.version;
            }
        } else if (type == AssetType::Audio) {
            auto it = audioClips.find(targetPath);
            if (it != audioClips.end() && it->second) {
                it->second->memoryBytes = estimatedBytes;
                it->second->version = metadata.version;
            }
        } else if (type == AssetType::Mesh) {
            auto it = meshes.find(targetPath);
            if (it != meshes.end() && it->second) {
                it->second->memoryBytes = estimatedBytes;
                it->second->version = metadata.version;
            }
        } else if (type == AssetType::Shader) {
            auto it = shaders.find(targetPath);
            if (it != shaders.end() && it->second) {
                it->second->memoryBytes = estimatedBytes;
                it->second->version = metadata.version;
            }
        } else if (type == AssetType::Material) {
            auto it = materials.find(targetPath);
            if (it != materials.end() && it->second) {
                it->second->path = targetPath;
            }
        }

        std::cout << "[Resource] Reloaded " << assetTypeName(type) << ": " << targetPath << std::endl;
        return true;
    }

    int reloadDirtyAssets() {
        if (!assetHotReloadEnabled) {
            return 0;
        }
        std::vector<std::string> dirtyAssets;
        for (const auto& entry : importedMetadata) {
            if (entry.second.dirty) {
                dirtyAssets.push_back(entry.second.importedPath);
            }
        }
        int reloaded = 0;
        for (const std::string& assetPath : dirtyAssets) {
            if (reloadAsset(assetPath)) {
                ++reloaded;
            }
        }
        return reloaded;
    }

    bool saveAssetDatabase(const std::string& path) const {
        if (path.empty()) {
            return false;
        }
        std::ofstream output(path);
        if (!output.is_open()) {
            return false;
        }
        output << "ZENITH_ASSET_DB\t1\n";
        output << "HOT_RELOAD\t" << (assetHotReloadEnabled ? 1 : 0) << "\n";
        for (const auto& budget : memoryBudgets) {
            output << "BUDGET\t" << assetTypeName(static_cast<AssetType>(budget.first)) << "\t" << budget.second << "\n";
        }
        for (const ImportedAssetMetadata& asset : importedAssets()) {
            output
                << "META\t"
                << escapeAssetDatabaseField(asset.importedPath) << "\t"
                << escapeAssetDatabaseField(asset.sourcePath) << "\t"
                << assetTypeName(asset.type) << "\t"
                << escapeAssetDatabaseField(asset.group) << "\t"
                << asset.sourceBytes << "\t"
                << asset.estimatedMemoryBytes << "\t"
                << asset.sourceTimestamp << "\t"
                << asset.importedTimestamp << "\t"
                << asset.version << "\t"
                << (asset.hotReloadable ? 1 : 0) << "\t"
                << (asset.dirty ? 1 : 0)
                << "\n";
        }
        for (const AssetBundle& bundle : assetBundles()) {
            for (const std::string& assetPath : bundle.assetPaths) {
                output
                    << "BUNDLE\t"
                    << escapeAssetDatabaseField(bundle.name) << "\t"
                    << escapeAssetDatabaseField(assetPath)
                    << "\n";
            }
        }
        return true;
    }

    bool loadAssetDatabase(const std::string& path) {
        if (path.empty()) {
            return false;
        }
        std::ifstream input(path);
        if (!input.is_open()) {
            return false;
        }

        importedMetadata.clear();
        bundles.clear();
        memoryBudgets.clear();
        assetHotReloadEnabled = true;

        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::stringstream stream(line);
            std::string field;
            while (std::getline(stream, field, '\t')) {
                fields.push_back(field);
            }
            if (fields.empty()) {
                continue;
            }

            if (fields[0] == "HOT_RELOAD" && fields.size() >= 2) {
                assetHotReloadEnabled = fields[1] == "1";
            } else if (fields[0] == "BUDGET" && fields.size() >= 3) {
                setMemoryBudgetByName(fields[1], static_cast<size_t>(std::stoull(fields[2])));
            } else if (fields[0] == "META" && fields.size() >= 12) {
                ImportedAssetMetadata metadata;
                metadata.importedPath = normalizeAssetPath(unescapeAssetDatabaseField(fields[1]));
                metadata.sourcePath = normalizeAssetPath(unescapeAssetDatabaseField(fields[2]));
                parseAssetType(fields[3], metadata.type);
                metadata.group = unescapeAssetDatabaseField(fields[4]);
                metadata.sourceBytes = static_cast<size_t>(std::stoull(fields[5]));
                metadata.estimatedMemoryBytes = static_cast<size_t>(std::stoull(fields[6]));
                metadata.sourceTimestamp = static_cast<uint64_t>(std::stoull(fields[7]));
                metadata.importedTimestamp = static_cast<uint64_t>(std::stoull(fields[8]));
                metadata.version = std::max(1, std::stoi(fields[9]));
                metadata.hotReloadable = fields[10] == "1";
                metadata.dirty = fields[11] == "1";
                if (!metadata.importedPath.empty()) {
                    importedMetadata[metadata.importedPath] = metadata;
                }
            } else if (fields[0] == "BUNDLE" && fields.size() >= 3) {
                addAssetToBundle(
                    unescapeAssetDatabaseField(fields[1]),
                    unescapeAssetDatabaseField(fields[2]));
            }
        }
        return true;
    }

    std::string metadataJson() const {
        std::ostringstream output;
        output << "{";
        output << "\"hotReloadEnabled\":" << (assetHotReloadEnabled ? "true" : "false") << ",";
        output << "\"assets\":[";
        const std::vector<ImportedAssetMetadata> assets = importedAssets();
        for (size_t i = 0; i < assets.size(); ++i) {
            const ImportedAssetMetadata& asset = assets[i];
            if (i > 0) {
                output << ",";
            }
            output << "{";
            output << "\"sourcePath\":\"" << escapeAssetJson(asset.sourcePath) << "\",";
            output << "\"importedPath\":\"" << escapeAssetJson(asset.importedPath) << "\",";
            output << "\"type\":\"" << assetTypeName(asset.type) << "\",";
            output << "\"group\":\"" << escapeAssetJson(asset.group) << "\",";
            output << "\"sourceBytes\":" << asset.sourceBytes << ",";
            output << "\"estimatedMemoryBytes\":" << asset.estimatedMemoryBytes << ",";
            output << "\"sourceTimestamp\":" << asset.sourceTimestamp << ",";
            output << "\"importedTimestamp\":" << asset.importedTimestamp << ",";
            output << "\"version\":" << asset.version << ",";
            output << "\"hotReloadable\":" << (asset.hotReloadable ? "true" : "false") << ",";
            output << "\"dirty\":" << (asset.dirty ? "true" : "false");
            output << "}";
        }
        output << "],";
        output << "\"bundles\":[";
        const std::vector<AssetBundle> bundleList = assetBundles();
        for (size_t i = 0; i < bundleList.size(); ++i) {
            const AssetBundle& bundle = bundleList[i];
            if (i > 0) {
                output << ",";
            }
            output << "{";
            output << "\"name\":\"" << escapeAssetJson(bundle.name) << "\",";
            output << "\"assets\":[";
            for (size_t j = 0; j < bundle.assetPaths.size(); ++j) {
                if (j > 0) {
                    output << ",";
                }
                output << "\"" << escapeAssetJson(bundle.assetPaths[j]) << "\"";
            }
            output << "]}";
        }
        output << "],";
        output << "\"budgets\":{";
        bool firstBudget = true;
        for (const auto& budget : memoryBudgets) {
            if (!firstBudget) {
                output << ",";
            }
            firstBudget = false;
            output << "\"" << assetTypeName(static_cast<AssetType>(budget.first)) << "\":" << budget.second;
        }
        output << "}";
        output << "}";
        return output.str();
    }

    bool bakeMetadata(const std::string& outputPath) const {
        if (outputPath.empty()) {
            return false;
        }
        const std::filesystem::path path(outputPath);
        std::error_code error;
        if (!path.parent_path().empty()) {
            std::filesystem::create_directories(path.parent_path(), error);
        }
        if (assetLowercase(path.extension().string()) == ".json") {
            std::ofstream output(outputPath);
            if (!output.is_open()) {
                return false;
            }
            output << metadataJson();
            return true;
        }
        return saveAssetDatabase(outputPath);
    }

    std::shared_ptr<TextureAsset> loadTexture(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        if (normalizedPath.empty()) {
            return nullptr;
        }
        auto it = textures.find(normalizedPath);
        if (it != textures.end()) {
            copyHandle(textureHandles, normalizedPath, AssetType::Texture, true);
            return it->second;
        }
        ImportedAssetMetadata& metadata = ensureMetadataRecord(normalizedPath, AssetType::Texture);
        const size_t memoryBytes = metadata.estimatedMemoryBytes > 0
            ? metadata.estimatedMemoryBytes
            : estimateAssetMemoryBytes(AssetType::Texture, metadata.sourceBytes);
        if (!canFitBudget(AssetType::Texture, normalizedPath, memoryBytes)) {
            warnBudgetExceeded(AssetType::Texture, normalizedPath, memoryBytes);
            copyHandle(textureHandles, normalizedPath, AssetType::Texture, false);
            return nullptr;
        }
        auto texture = std::make_shared<TextureAsset>(normalizedPath);
        texture->memoryBytes = memoryBytes;
        texture->version = std::max(1, metadata.version);
        textures[normalizedPath] = texture;
        copyHandle(textureHandles, normalizedPath, AssetType::Texture, true);
        refreshMetadataAfterLoad(normalizedPath, AssetType::Texture, memoryBytes);
        std::cout << "[Resource] Loaded Texture: " << normalizedPath << std::endl;
        return texture;
    }

    std::shared_ptr<AudioAsset> loadAudio(const std::string& path, bool isSpatial = false) {
        const std::string normalizedPath = normalizeAssetPath(path);
        if (normalizedPath.empty()) {
            return nullptr;
        }
        auto it = audioClips.find(normalizedPath);
        if (it != audioClips.end()) {
            if (isSpatial) {
                it->second->isSpatial = true;
            }
            copyHandle(audioHandles, normalizedPath, AssetType::Audio, true);
            return it->second;
        }
        ImportedAssetMetadata& metadata = ensureMetadataRecord(normalizedPath, AssetType::Audio);
        const size_t memoryBytes = metadata.estimatedMemoryBytes > 0
            ? metadata.estimatedMemoryBytes
            : estimateAssetMemoryBytes(AssetType::Audio, metadata.sourceBytes);
        if (!canFitBudget(AssetType::Audio, normalizedPath, memoryBytes)) {
            warnBudgetExceeded(AssetType::Audio, normalizedPath, memoryBytes);
            copyHandle(audioHandles, normalizedPath, AssetType::Audio, false);
            return nullptr;
        }
        auto audio = std::make_shared<AudioAsset>(normalizedPath, isSpatial);
        audio->memoryBytes = memoryBytes;
        audio->version = std::max(1, metadata.version);
        audioClips[normalizedPath] = audio;
        copyHandle(audioHandles, normalizedPath, AssetType::Audio, true);
        refreshMetadataAfterLoad(normalizedPath, AssetType::Audio, memoryBytes);
        std::cout << "[Resource] Loaded Audio: " << normalizedPath << std::endl;
        return audio;
    }

    std::shared_ptr<MeshAsset> loadMesh(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        if (normalizedPath.empty()) {
            return nullptr;
        }
        auto it = meshes.find(normalizedPath);
        if (it != meshes.end()) {
            copyHandle(meshHandles, normalizedPath, AssetType::Mesh, true);
            return it->second;
        }
        ImportedAssetMetadata& metadata = ensureMetadataRecord(normalizedPath, AssetType::Mesh);
        const size_t memoryBytes = metadata.estimatedMemoryBytes > 0
            ? metadata.estimatedMemoryBytes
            : estimateAssetMemoryBytes(AssetType::Mesh, metadata.sourceBytes);
        if (!canFitBudget(AssetType::Mesh, normalizedPath, memoryBytes)) {
            warnBudgetExceeded(AssetType::Mesh, normalizedPath, memoryBytes);
            copyHandle(meshHandles, normalizedPath, AssetType::Mesh, false);
            return nullptr;
        }
        auto mesh = std::make_shared<MeshAsset>(normalizedPath);
        mesh->memoryBytes = memoryBytes;
        mesh->version = std::max(1, metadata.version);
        meshes[normalizedPath] = mesh;
        copyHandle(meshHandles, normalizedPath, AssetType::Mesh, true);
        refreshMetadataAfterLoad(normalizedPath, AssetType::Mesh, memoryBytes);
        std::cout << "[Resource] Loaded 3D Mesh: " << normalizedPath << std::endl;
        return mesh;
    }

    std::shared_ptr<ShaderAsset> loadShader(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        if (normalizedPath.empty()) {
            return nullptr;
        }
        auto it = shaders.find(normalizedPath);
        if (it != shaders.end()) {
            copyHandle(shaderHandles, normalizedPath, AssetType::Shader, true);
            return it->second;
        }
        ImportedAssetMetadata& metadata = ensureMetadataRecord(normalizedPath, AssetType::Shader);
        const size_t memoryBytes = metadata.estimatedMemoryBytes > 0
            ? metadata.estimatedMemoryBytes
            : estimateAssetMemoryBytes(AssetType::Shader, metadata.sourceBytes);
        if (!canFitBudget(AssetType::Shader, normalizedPath, memoryBytes)) {
            warnBudgetExceeded(AssetType::Shader, normalizedPath, memoryBytes);
            copyHandle(shaderHandles, normalizedPath, AssetType::Shader, false);
            return nullptr;
        }
        auto shader = std::make_shared<ShaderAsset>(normalizedPath);
        shader->memoryBytes = memoryBytes;
        shader->version = std::max(1, metadata.version);
        shaders[normalizedPath] = shader;
        copyHandle(shaderHandles, normalizedPath, AssetType::Shader, true);
        refreshMetadataAfterLoad(normalizedPath, AssetType::Shader, memoryBytes);
        std::cout << "[Resource] Loaded Shader: " << normalizedPath << std::endl;
        return shader;
    }

    bool hasMaterial(const std::string& path) const {
        return materials.find(normalizeAssetPath(path)) != materials.end();
    }

    std::shared_ptr<MaterialAsset> loadMaterial(const std::string& path, const std::string& shaderPath = "") {
        const std::string normalizedPath = normalizeAssetPath(path);
        if (normalizedPath.empty()) {
            return nullptr;
        }
        auto it = materials.find(normalizedPath);
        if (it != materials.end()) {
            if (!shaderPath.empty()) {
                it->second->shaderPath = shaderPath;
            }
            copyHandle(materialHandles, normalizedPath, AssetType::Material, true);
            return it->second;
        }
        ImportedAssetMetadata& metadata = ensureMetadataRecord(normalizedPath, AssetType::Material);
        const size_t memoryBytes = metadata.estimatedMemoryBytes > 0
            ? metadata.estimatedMemoryBytes
            : estimateAssetMemoryBytes(AssetType::Material, metadata.sourceBytes);
        if (!canFitBudget(AssetType::Material, normalizedPath, memoryBytes)) {
            warnBudgetExceeded(AssetType::Material, normalizedPath, memoryBytes);
            copyHandle(materialHandles, normalizedPath, AssetType::Material, false);
            return nullptr;
        }
        auto material = std::make_shared<MaterialAsset>(normalizedPath, shaderPath);
        materials[normalizedPath] = material;
        copyHandle(materialHandles, normalizedPath, AssetType::Material, true);
        refreshMetadataAfterLoad(normalizedPath, AssetType::Material, memoryBytes);
        std::cout << "[Resource] Loaded Material: " << normalizedPath << std::endl;
        return material;
    }

    AssetHandle loadTextureHandle(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        return normalizedPath.empty()
            ? AssetHandle{}
            : copyHandle(textureHandles, normalizedPath, AssetType::Texture, loadTexture(normalizedPath) != nullptr);
    }

    AssetHandle loadAudioHandle(const std::string& path, bool isSpatial = false) {
        const std::string normalizedPath = normalizeAssetPath(path);
        return normalizedPath.empty()
            ? AssetHandle{}
            : copyHandle(audioHandles, normalizedPath, AssetType::Audio, loadAudio(normalizedPath, isSpatial) != nullptr);
    }

    AssetHandle loadMeshHandle(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        return normalizedPath.empty()
            ? AssetHandle{}
            : copyHandle(meshHandles, normalizedPath, AssetType::Mesh, loadMesh(normalizedPath) != nullptr);
    }

    AssetHandle loadShaderHandle(const std::string& path) {
        const std::string normalizedPath = normalizeAssetPath(path);
        return normalizedPath.empty()
            ? AssetHandle{}
            : copyHandle(shaderHandles, normalizedPath, AssetType::Shader, loadShader(normalizedPath) != nullptr);
    }

    AssetHandle loadMaterialHandle(const std::string& path, const std::string& shaderPath = "") {
        const std::string normalizedPath = normalizeAssetPath(path);
        return normalizedPath.empty()
            ? AssetHandle{}
            : copyHandle(materialHandles, normalizedPath, AssetType::Material, loadMaterial(normalizedPath, shaderPath) != nullptr);
    }

    void clearCache() {
        textures.clear();
        audioClips.clear();
        meshes.clear();
        shaders.clear();
        materials.clear();
        textureHandles.clear();
        audioHandles.clear();
        meshHandles.clear();
        shaderHandles.clear();
        materialHandles.clear();
        nextHandleId = 1;
    }

    void clearAssetDatabase() {
        importedMetadata.clear();
        bundles.clear();
        memoryBudgets.clear();
        assetHotReloadEnabled = true;
    }

    void clearAll() {
        clearCache();
        clearAssetDatabase();
    }
};

} // namespace resource
} // namespace zenith

#endif // ZENITH_RESOURCE_H
