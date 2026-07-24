#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "zenith/game/zenith_scene.h"
#include "zenith/game/zenith_resource.h"

static bool writeTextFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    if (!output.is_open()) {
        return false;
    }
    output << contents;
    return true;
}

static bool containsText(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

int main() {
    namespace fs = std::filesystem;

    using zenith::game::Scene;
    using zenith::resource::AssetHandle;
    using zenith::resource::ResourceManager;

    ResourceManager& resources = ResourceManager::getInstance();
    resources.clearAll();

    const fs::path tempRoot = fs::path("tests/runtime/tmp_asset_pipeline");
    std::error_code error;
    fs::remove_all(tempRoot, error);
    fs::create_directories(tempRoot / "src", error);

    const fs::path sourceTexture = tempRoot / "src" / "sprite.png";
    const fs::path sourceAudio = tempRoot / "src" / "sound.wav";
    if (!writeTextFile(sourceTexture, "sprite-v1") || !writeTextFile(sourceAudio, "sound-v1")) {
        std::cerr << "failed_to_create_sources" << std::endl;
        return 1;
    }

    Scene scene;
    const std::string importedTexture = scene.importAsset(sourceTexture.generic_string(), "", "ui", "starter");
    const std::string importedAudio = scene.importAsset(sourceAudio.generic_string(), "", "sfx", "starter");

    std::cout << "imported_texture=" << importedTexture << std::endl;
    std::cout << "imported_audio=" << importedAudio << std::endl;
    std::cout << "texture_group=" << scene.assetGroup(importedTexture) << std::endl;
    std::cout << "starter_assets=" << scene.assetBundleAssetCount("starter") << std::endl;

    if (importedTexture.empty() ||
        importedAudio.empty() ||
        scene.assetGroup(importedTexture) != "ui" ||
        scene.assetBundleAssetCount("starter") != 2) {
        return 1;
    }

    scene.createAssetBundle("ui_only");
    scene.addAssetToBundle("ui_only", importedTexture);
    std::cout << "ui_only_first=" << scene.assetBundleAsset("ui_only", 0) << std::endl;
    if (scene.assetBundleAssetCount("ui_only") != 1 || scene.assetBundleAsset("ui_only", 0) != importedTexture) {
        return 1;
    }

    scene.setAssetMemoryBudget("Texture", 3000);
    AssetHandle blockedTexture = resources.loadTextureHandle(importedTexture);
    std::cout << "budget_blocked=" << blockedTexture.isLoaded << std::endl;
    if (blockedTexture.isLoaded) {
        return 1;
    }

    scene.setAssetMemoryBudget("Texture", 16384);
    AssetHandle loadedTexture = resources.loadTextureHandle(importedTexture);
    std::cout << "budget_loaded=" << loadedTexture.isLoaded << std::endl;
    std::cout << "texture_usage=" << scene.assetMemoryUsage("Texture") << std::endl;
    if (!loadedTexture.isLoaded || scene.assetMemoryUsage("Texture") <= 0) {
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    if (!writeTextFile(sourceTexture, "sprite-v2-expanded-data")) {
        std::cerr << "failed_to_modify_source" << std::endl;
        return 1;
    }

    const int dirtyAssets = scene.pollAssetChanges();
    const int reloadedAssets = scene.reloadDirtyAssets();
    std::cout << "dirty_assets=" << dirtyAssets << std::endl;
    std::cout << "reloaded_assets=" << reloadedAssets << std::endl;
    if (dirtyAssets <= 0 || reloadedAssets <= 0) {
        return 1;
    }

    const std::string assetJson = scene.assetDatabaseJson();
    std::cout << "json_has_bundle=" << containsText(assetJson, "\"starter\"") << std::endl;
    if (!containsText(assetJson, importedTexture) || !containsText(assetJson, "\"bundles\"")) {
        return 1;
    }

    const fs::path bakedJson = tempRoot / "baked_assets.json";
    if (!scene.bakeAssetMetadata(bakedJson.generic_string())) {
        std::cerr << "failed_to_bake_json" << std::endl;
        return 1;
    }

    std::ifstream bakedInput(bakedJson);
    std::string bakedContents((std::istreambuf_iterator<char>(bakedInput)), std::istreambuf_iterator<char>());
    std::cout << "baked_has_asset=" << containsText(bakedContents, importedAudio) << std::endl;
    if (!containsText(bakedContents, importedAudio)) {
        return 1;
    }

    return 0;
}
