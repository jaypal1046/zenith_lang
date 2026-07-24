#include <iostream>
#include "zenith/game/zenith_game.h"

class RuntimeStreamingScene : public zenith::game::Scene {
public:
    void onLoad() override {
        registerPrefab("enemy", [](zenith::game::Scene& scene, zenith::game::EntityId entity) {
            zenith::game::Transform2D& transform = scene.world.addTransform2D(entity);
            transform.position = zenith::physics::Vec2(0.0f, 0.0f);

            zenith::game::SpriteRenderer2D& sprite = scene.world.addSpriteRenderer2D(entity);
            sprite.size = zenith::physics::Vec2(2.0f, 2.0f);
            sprite.tintColor = "green";

            scene.attachBoxCollider2D(entity, 2.0f, 2.0f, false);
            scene.setEntityLayerMask(entity, 2, 7);
        });

        registerSceneStream("prop_cluster", [](zenith::game::Scene& scene, const std::string& instanceName) {
            zenith::game::EntityId root = scene.instantiatePrefab("enemy", instanceName + "_root");
            zenith::game::EntityId beacon = scene.world.createEntity(instanceName + "_beacon");
            scene.world.addTransform2D(beacon).position = zenith::physics::Vec2(2.0f, 0.0f);
            scene.world.addSpriteRenderer2D(beacon).size = zenith::physics::Vec2(1.0f, 1.0f);
            scene.setParent(beacon, root);
        });

        registerSceneStream("arena", [](zenith::game::Scene& scene, const std::string& instanceName) {
            zenith::game::EntityId root = scene.instantiatePrefab("enemy", instanceName + "_root");
            zenith::game::EntityId marker = scene.world.createEntity(instanceName + "_marker");
            scene.world.addTransform2D(marker).position = zenith::physics::Vec2(1.0f, 0.0f);
            scene.world.addSpriteRenderer2D(marker).size = zenith::physics::Vec2(1.0f, 1.0f);
            scene.setParent(marker, root);
        });

        registerSceneStream("arena_nested", [](zenith::game::Scene& scene, const std::string& instanceName) {
            zenith::game::EntityId root = scene.instantiatePrefab("enemy", instanceName + "_root");
            zenith::game::EntityId marker = scene.world.createEntity(instanceName + "_marker");
            scene.world.addTransform2D(marker).position = zenith::physics::Vec2(-1.0f, 0.0f);
            scene.world.addSpriteRenderer2D(marker).size = zenith::physics::Vec2(1.0f, 1.0f);
            scene.setParent(marker, root);

            const std::string nestedName = instanceName + "_props";
            scene.loadSceneStream("prop_cluster", nestedName);
            std::optional<zenith::game::EntityId> nestedRoot = scene.world.findByName(nestedName + "_root");
            if (nestedRoot.has_value()) {
                scene.setParent(nestedRoot.value(), root);
            }
        });
    }
};

static int countStyle(const zenith::Canvas& canvas, const std::string& style) {
    int count = 0;
    for (const auto& row : canvas.buffer.styles) {
        for (const std::string& cellStyle : row) {
            if (cellStyle == style) {
                ++count;
            }
        }
    }
    return count;
}

int main() {
    RuntimeStreamingScene scene;
    scene.load();

    using zenith::game::EntityId;

    EntityId templateRoot = scene.instantiatePrefab("enemy", "TemplateRoot");
    EntityId templateChild = scene.world.createEntity("TemplateChild");
    scene.world.addTransform2D(templateChild).position = zenith::physics::Vec2(1.0f, 0.0f);
    scene.world.addSpriteRenderer2D(templateChild).size = zenith::physics::Vec2(1.0f, 1.0f);
    scene.setParent(templateChild, templateRoot);

    EntityId archetypeClone = scene.instantiateArchetype(templateRoot, "CloneRoot");
    std::cout << "clone_alive=" << scene.world.isAlive(archetypeClone) << std::endl;
    std::cout << "clone_children=" << scene.world.childrenOf(archetypeClone).size() << std::endl;
    std::cout << "clone_layer=" << scene.entityLayer(archetypeClone) << std::endl;

    EntityId templateCamera = scene.spawnCamera2D("TemplateCamera", 0.0f, 0.0f, 1.0f, true);
    EntityId clonedCamera = scene.instantiateArchetype(templateCamera, "ClonedCamera");
    std::cout << "clone_camera_primary=" << scene.camera2D(clonedCamera).primary << std::endl;

    bool streamLoaded = scene.loadSceneStream("arena", "arena_01");
    std::cout << "stream_loaded=" << streamLoaded << std::endl;
    std::cout << "stream_entities=" << scene.sceneStreamEntityCount("arena_01") << std::endl;
    std::cout << "stream_loaded_again=" << scene.loadSceneStream("arena", "arena_01") << std::endl;

    std::optional<EntityId> streamRoot = scene.world.findByName("arena_01_root");
    std::cout << "stream_root_alive=" << (streamRoot.has_value() && scene.world.isAlive(streamRoot.value())) << std::endl;
    std::cout << "stream_unloaded=" << scene.unloadSceneStream("arena_01") << std::endl;
    std::cout << "stream_loaded_after=" << scene.isSceneStreamLoaded("arena_01") << std::endl;
    std::cout << "stream_root_after=" << (streamRoot.has_value() && scene.world.isAlive(streamRoot.value())) << std::endl;

    bool nestedLoaded = scene.loadSceneStream("arena_nested", "arena_nested_01");
    std::cout << "nested_loaded=" << nestedLoaded << std::endl;
    std::cout << "nested_outer_entities=" << scene.sceneStreamEntityCount("arena_nested_01") << std::endl;
    std::cout << "nested_inner_entities=" << scene.sceneStreamEntityCount("arena_nested_01_props") << std::endl;
    std::cout << "nested_inner_loaded=" << scene.isSceneStreamLoaded("arena_nested_01_props") << std::endl;

    std::optional<EntityId> nestedOuterRoot = scene.world.findByName("arena_nested_01_root");
    std::optional<EntityId> nestedInnerRoot = scene.world.findByName("arena_nested_01_props_root");
    EntityId adoptedChild = scene.world.createEntity("ArenaDynamicChild");
    scene.world.addTransform2D(adoptedChild).position = zenith::physics::Vec2(3.0f, 0.0f);
    scene.world.addSpriteRenderer2D(adoptedChild).size = zenith::physics::Vec2(1.0f, 1.0f);
    if (nestedOuterRoot.has_value()) {
        scene.setParent(adoptedChild, nestedOuterRoot.value());
    }
    std::cout << "nested_outer_entities_after_adopt=" << scene.sceneStreamEntityCount("arena_nested_01") << std::endl;

    std::cout << "nested_outer_unloaded=" << scene.unloadSceneStream("arena_nested_01") << std::endl;
    std::cout << "nested_outer_loaded_after=" << scene.isSceneStreamLoaded("arena_nested_01") << std::endl;
    std::cout << "nested_outer_root_after=" << (nestedOuterRoot.has_value() && scene.world.isAlive(nestedOuterRoot.value())) << std::endl;
    std::cout << "nested_dynamic_child_after=" << scene.world.isAlive(adoptedChild) << std::endl;
    std::cout << "nested_inner_loaded_after_outer=" << scene.isSceneStreamLoaded("arena_nested_01_props") << std::endl;
    std::cout << "nested_inner_root_after_outer=" << (nestedInnerRoot.has_value() && scene.world.isAlive(nestedInnerRoot.value())) << std::endl;
    std::cout << "nested_inner_unloaded=" << scene.unloadSceneStream("arena_nested_01_props") << std::endl;
    std::cout << "nested_inner_loaded_final=" << scene.isSceneStreamLoaded("arena_nested_01_props") << std::endl;

    EntityId actor = scene.spawnSprite("Actor", 0.0f, 0.0f, 2.0f, 2.0f, "white");
    EntityId target = scene.spawnSprite("Target", 0.0f, 0.0f, 2.0f, 2.0f, "white");
    scene.attachBoxCollider2D(actor, 2.0f, 2.0f, false);
    scene.attachBoxCollider2D(target, 2.0f, 2.0f, false);
    scene.setEntityLayerMask(actor, 1, 4);
    scene.setEntityLayerMask(target, 2, 1);
    std::cout << "mask_before=" << scene.overlaps2D(actor, target) << std::endl;
    scene.setEntityLayerMask(target, 4, 1);
    std::cout << "mask_after=" << scene.overlaps2D(actor, target) << std::endl;

    EntityId actor3D = scene.spawnMesh("Actor3D", "", "", 0.0f, 0.0f, 0.0f);
    EntityId target3D = scene.spawnMesh("Target3D", "", "", 0.0f, 0.0f, 0.0f);
    scene.attachBoxCollider3D(actor3D, 2.0f, 2.0f, 2.0f, false);
    scene.attachBoxCollider3D(target3D, 2.0f, 2.0f, 2.0f, false);
    scene.setEntityLayerMask(actor3D, 8, 32);
    scene.setEntityLayerMask(target3D, 16, 8);
    std::cout << "mask3d_before=" << scene.overlaps3D(actor3D, target3D) << std::endl;
    scene.setEntityLayerMask(target3D, 32, 8);
    std::cout << "mask3d_after=" << scene.overlaps3D(actor3D, target3D) << std::endl;

    zenith::game::RaycastHit2DResult rayMiss = scene.raycast2DMask(-5.0f, 0.0f, 1.0f, 0.0f, 20.0f, 32);
    zenith::game::RaycastHit2DResult rayHit = scene.raycast2DMask(-5.0f, 0.0f, 1.0f, 0.0f, 20.0f, 4);
    std::cout << "ray_mask_miss=" << rayMiss.hit << std::endl;
    std::cout << "ray_mask_hit=" << rayHit.hit << std::endl;

    zenith::game::RaycastHit3DResult rayMiss3D = scene.raycast3DMask(-5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f, 16);
    zenith::game::RaycastHit3DResult rayHit3D = scene.raycast3DMask(-5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 20.0f, 32);
    std::cout << "ray3d_mask_miss=" << rayMiss3D.hit << std::endl;
    std::cout << "ray3d_mask_hit=" << rayHit3D.hit << std::endl;

    EntityId camera = scene.world.createEntity("MainCamera");
    scene.world.addTransform2D(camera).position = zenith::physics::Vec2(0.0f, 0.0f);
    scene.world.addCamera2D(camera).primary = true;
    scene.setEntityMask(camera, 8);
    EntityId visibleSprite = scene.spawnSprite("Visible", 0.0f, 0.0f, 2.0f, 2.0f, "green");
    EntityId hiddenSprite = scene.spawnSprite("Hidden", 5.0f, 0.0f, 2.0f, 2.0f, "red");
    scene.setEntityLayer(visibleSprite, 8);
    scene.setEntityLayer(hiddenSprite, 16);

    zenith::Canvas canvas(24, 12, false);
    zenith::drawWorld2D(canvas, scene.world, false);
    std::cout << "draw_green=" << countStyle(canvas, "\033[42m") << std::endl;
    std::cout << "draw_red=" << countStyle(canvas, "\033[41m") << std::endl;

    return 0;
}
