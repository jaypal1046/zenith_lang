#include <iostream>
#include "zenith/game/zenith_game.h"

class HarnessProbeScene : public zenith::game::Scene {
public:
    zenith::game::EntityId camera;
    zenith::game::EntityId player;
    std::string materialPath;
    int fixedSteps = 0;

    void onLoad() override {
        name = "HarnessProbeScene";
        clearColor = "black";
        autoRenderWorld2D = true;
        drawEntityNames = false;
        debugDrawRuntimeStats = true;
        debugDrawTransforms2D = true;
        debugOverlayColor = "cyan";

        camera = spawnCamera2D("Camera", 12.0f, 8.0f, 1.0f, true);
        player = spawnCharacter2D("Player", "assets/textures/player.png", 12.0f, 8.0f, 2.0f, 2.0f, "green");
        attachCapsuleCollider2D(player, 1.8f, 0.7f, false);

        materialPath = createMaterial("assets/materials/player.mat", "");
        defineMaterialText(materialPath, "title", "Title", "Hero");
        defineMaterialRadio(materialPath, "mode", "Mode", "idle,run,jump", "idle");
        defineMaterialButton(materialPath, "focus", "Focus", "focusPlayer");

        inspectEntity(player);
        inspectMaterial(materialPath);
    }

    void onFixedUpdate(float) override {
        fixedSteps += 1;
        zenith::followPrimaryCamera2D(world, player, zenith::physics::Vec2(0.0f, 0.0f), 1.0f);
    }
};

int main() {
    HarnessProbeScene scene;
    zenith::GameplayHarnessConfig config;
    config.canvasWidth = 72;
    config.canvasHeight = 24;
    config.debugOverlayEnabled = true;
    config.inspectorEnabled = true;

    zenith::GameplayTestHarness harness(scene, config);
    harness.load();
    harness.renderFrame();

    if (!harness.snapshotContains("Inspector")) {
        std::cerr << "missing_inspector" << std::endl;
        return 1;
    }
    if (!harness.snapshotContains("Player")) {
        std::cerr << "missing_player" << std::endl;
        return 1;
    }
    if (!harness.snapshotContains("title=Hero")) {
        std::cerr << "missing_material_property" << std::endl;
        return 1;
    }
    if (!harness.snapshotContains("frame:")) {
        std::cerr << "missing_overlay" << std::endl;
        return 1;
    }

    harness.stepFrame(0.05f);
    if (scene.fixedSteps <= 0 || scene.totalFrames() <= 0) {
        std::cerr << "missing_frame_progress" << std::endl;
        return 1;
    }

    harness.setDebugOverlayEnabled(false);
    harness.renderFrame();
    if (harness.snapshotContains("frame:")) {
        std::cerr << "overlay_toggle_failed" << std::endl;
        return 1;
    }

    harness.setInspectorEnabled(false);
    harness.renderFrame();
    if (harness.snapshotContains("Inspector")) {
        std::cerr << "inspector_toggle_failed" << std::endl;
        return 1;
    }

    std::cout << "frames=" << scene.totalFrames() << std::endl;
    std::cout << "fixed_steps=" << scene.fixedSteps << std::endl;
    std::cout << "snapshot_ok=1" << std::endl;
    return 0;
}
