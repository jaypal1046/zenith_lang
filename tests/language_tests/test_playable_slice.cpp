#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"


#ifdef __ANDROID__
const bool isAndroid = true;
#else
const bool isAndroid = false;
#endif

#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
    const bool isIos = true;
    const bool isMac = false;
  #else
    const bool isIos = false;
    const bool isMac = true;
  #endif
#else
  const bool isIos = false;
  const bool isMac = false;
#endif

#ifdef __linux__
  #ifndef __ANDROID__
    const bool isLinux = true;
  #else
    const bool isLinux = false;
  #endif
#else
  const bool isLinux = false;
#endif

#ifdef _WIN32
const bool isWindows = true;
#else
const bool isWindows = false;
#endif

const bool isWeb = false;

template <typename T>
inline void print(const T& msg) { std::cout << msg; }
template <typename T>
inline void println(const T& msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

class PlayableSliceTestScene : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId arena;
    zenith::game::EntityId player;
    zenith::game::EntityId camera;
    zenith::game::EntityId exitGate;
    zenith::game::EntityId battery;
    zenith::game::EntityId wallNorth;
    zenith::game::EntityId wallSouth;
    zenith::game::EntityId wallWest;
    zenith::game::EntityId wallEast;
    zenith::game::EntityId blockA;
    zenith::game::EntityId blockB;
    zenith::game::EntityId humAudio;
    zenith::game::EntityId pickupAudio;
    zenith::game::EntityId listener;
    float startX = 8.0;
    float startY = 8.0;
    float playerSpeed = 18.0;
    float moveX = 0.0;
    float moveY = 0.0;
    float lookX = 1.0;
    float lookY = 0.0;
    bool batteryCollected = false;
    bool exitUnlocked = false;
    bool victory = false;

    PlayableSliceTestScene()  {}
    PlayableSliceTestScene(zenith::game::EntityId arena, zenith::game::EntityId player, zenith::game::EntityId camera, zenith::game::EntityId exitGate, zenith::game::EntityId battery, zenith::game::EntityId wallNorth, zenith::game::EntityId wallSouth, zenith::game::EntityId wallWest, zenith::game::EntityId wallEast, zenith::game::EntityId blockA, zenith::game::EntityId blockB, zenith::game::EntityId humAudio, zenith::game::EntityId pickupAudio, zenith::game::EntityId listener, float startX, float startY, float playerSpeed, float moveX, float moveY, float lookX, float lookY, bool batteryCollected, bool exitUnlocked, bool victory) : arena(arena), player(player), camera(camera), exitGate(exitGate), battery(battery), wallNorth(wallNorth), wallSouth(wallSouth), wallWest(wallWest), wallEast(wallEast), blockA(blockA), blockB(blockB), humAudio(humAudio), pickupAudio(pickupAudio), listener(listener), startX(startX), startY(startY), playerSpeed(playerSpeed), moveX(moveX), moveY(moveY), lookX(lookX), lookY(lookY), batteryCollected(batteryCollected), exitUnlocked(exitUnlocked), victory(victory) {}

    zenith::game::EntityId createEntity(std::string name) {
        return world.createEntity(name);
    }

    void setEntityName(zenith::game::EntityId entity, std::string name) {
        if (!world.isAlive(entity)) {
            return;
        }
        if (zenith::game::NameComponent* component = world.getName(entity)) {
            component->value = name;
        } else {
            world.addName(entity, name);
        }
    }

    std::string entityName(zenith::game::EntityId entity) {
        if (const zenith::game::NameComponent* component = world.getName(entity)) {
            return component->value;
        }
        return "";
    }

    void setEntityTag(zenith::game::EntityId entity, std::string tag) {
        if (!world.isAlive(entity)) {
            return;
        }
        if (zenith::game::TagComponent* component = world.getTag(entity)) {
            component->value = tag;
        } else {
            world.addTag(entity, tag);
        }
    }

    std::string entityTag(zenith::game::EntityId entity) {
        if (const zenith::game::TagComponent* component = world.getTag(entity)) {
            return component->value;
        }
        return "";
    }

    zenith::game::EntityId findEntityByName(std::string name) {
        std::optional<zenith::game::EntityId> entity = world.findByName(name);
        return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();
    }

    zenith::game::EntityId findEntityByTag(std::string tag) {
        std::optional<zenith::game::EntityId> entity = world.findByTag(tag);
        return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();
    }

    bool setParent(zenith::game::EntityId child, zenith::game::EntityId parent) {
        return world.setParent(child, parent);
    }

    bool clearParent(zenith::game::EntityId child) {
        return world.clearParent(child);
    }

    zenith::game::EntityId parentOf(zenith::game::EntityId child) {
        return world.parentOf(child);
    }

    int childCount(zenith::game::EntityId parent) {
        return static_cast<int>(world.childrenOf(parent).size());
    }

    zenith::game::EntityId childAt(zenith::game::EntityId parent, int index) {
        std::vector<zenith::game::EntityId> children = world.childrenOf(parent);
        if (index < 0 || static_cast<size_t>(index) >= children.size()) {
            return zenith::game::EntityId::invalid();
        }
        return children[static_cast<size_t>(index)];
    }

    zenith::game::EntityId spawnSprite(std::string name, float x, float y, float w, float h, std::string color) {
        zenith::game::EntityId entity = world.createEntity(name);
        zenith::game::Transform2D& transform = world.addTransform2D(entity);
        transform.position = zenith::physics::Vec2(x, y);
        zenith::game::SpriteRenderer2D& sprite = world.addSpriteRenderer2D(entity);
        sprite.size = zenith::physics::Vec2(w, h);
        sprite.tintColor = color;
        return entity;
    }

    zenith::game::EntityId spawnTexturedSprite(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {
        zenith::game::EntityId entity = spawnSprite(name, x, y, w, h, color);
        if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(texturePath);
        }
        return entity;
    }

    zenith::game::EntityId spawnCamera2D(std::string name, float x, float y, float zoom, bool primary) {
        zenith::game::EntityId entity = world.createEntity(name);
        zenith::game::Transform2D& transform = world.addTransform2D(entity);
        transform.position = zenith::physics::Vec2(x, y);
        if (primary) {
            std::optional<zenith::game::EntityId> current = world.primaryCamera2D();
            if (current.has_value()) {
                if (zenith::game::Camera2DComponent* existing = world.getCamera2D(current.value())) {
                    existing->primary = false;
                }
            }
        }
        zenith::game::Camera2DComponent& camera = world.addCamera2D(entity);
        camera.zoom = zoom;
        camera.primary = primary;
        return entity;
    }

    bool destroyEntity(zenith::game::EntityId entity) {
        return world.destroyEntity(entity);
    }

    bool isEntityAlive(zenith::game::EntityId entity) {
        return world.isAlive(entity);
    }

    int entityCount() {
        return static_cast<int>(world.entityCount());
    }

    void setEntityPosition2D(zenith::game::EntityId entity, float x, float y) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::game::Transform2D* transform = world.getTransform2D(entity);
        if (transform == nullptr) {
            transform = &world.addTransform2D(entity);
        }
        transform->position = zenith::physics::Vec2(x, y);
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->position = transform->position;
        }
    }

    void moveEntity2D(zenith::game::EntityId entity, float dx, float dy) {
        setEntityPosition2D(entity, entityPositionX(entity) + dx, entityPositionY(entity) + dy);
    }

    float entityPositionX(zenith::game::EntityId entity) {
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            return transform->position.x;
        }
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->position.x;
        }
        return 0.0f;
    }

    float entityPositionY(zenith::game::EntityId entity) {
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            return transform->position.y;
        }
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->position.y;
        }
        return 0.0f;
    }

    void setSpriteColor(zenith::game::EntityId entity, std::string color) {
        if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            sprite->tintColor = color;
        }
    }

    void setSpriteTexture(zenith::game::EntityId entity, std::string texturePath) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity);
        if (sprite == nullptr) {
            sprite = &world.addSpriteRenderer2D(entity);
        }
        sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(texturePath);
    }

    std::string spriteTexturePath(zenith::game::EntityId entity) {
        if (const zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            if (sprite->texture) {
                return sprite->texture->path;
            }
        }
        return "";
    }

    void attachBody2D(zenith::game::EntityId entity, float mass, float gravityScale, float friction, float restitution) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity);
        if (body == nullptr) {
            body = &world.addRigidBody2D(entity);
        }
        body->mass = (mass <= 0.0f) ? 1.0f : mass;
        body->gravityScale = gravityScale;
        body->friction = friction;
        body->restitution = restitution;
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            body->position = transform->position;
        }
    }

    zenith::game::BoxCollider2DView boxCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::boxCollider2D(entity);
    }

    zenith::game::CircleCollider2DView circleCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::circleCollider2D(entity);
    }

    zenith::game::CapsuleCollider2DView capsuleCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::capsuleCollider2D(entity);
    }

    void attachBoxCollider2D(zenith::game::EntityId entity, float width, float height, bool isTrigger) {
        zenith::game::Scene::attachBoxCollider2D(entity, width, height, isTrigger);
    }

    void attachCircleCollider2D(zenith::game::EntityId entity, float radius, bool isTrigger) {
        zenith::game::Scene::attachCircleCollider2D(entity, radius, isTrigger);
    }

    void attachCapsuleCollider2D(zenith::game::EntityId entity, float height, float radius, bool isTrigger) {
        zenith::game::Scene::attachCapsuleCollider2D(entity, height, radius, isTrigger);
    }

    void setBodyVelocity2D(zenith::game::EntityId entity, float vx, float vy) {
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->velocity = zenith::physics::Vec2(vx, vy);
        }
    }

    void applyBodyImpulse2D(zenith::game::EntityId entity, float ix, float iy) {
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->applyImpulse(zenith::physics::Vec2(ix, iy));
        }
    }

    float bodyVelocityX(zenith::game::EntityId entity) {
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->velocity.x;
        }
        return 0.0f;
    }

    float bodyVelocityY(zenith::game::EntityId entity) {
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->velocity.y;
        }
        return 0.0f;
    }

    bool overlaps2D(zenith::game::EntityId first, zenith::game::EntityId second) {
        return zenith::game::Scene::overlaps2D(first, second);
    }

    bool containsPoint2D(zenith::game::EntityId entity, float x, float y) {
        return zenith::game::Scene::containsPoint2D(entity, x, y);
    }

    zenith::game::RaycastHit2DResult raycast2D(float originX, float originY, float directionX, float directionY, float maxDistance) {
        return zenith::game::Scene::raycast2D(originX, originY, directionX, directionY, maxDistance);
    }

    zenith::game::BoxCollider3DView boxCollider3D(zenith::game::EntityId entity) {
        return zenith::game::Scene::boxCollider3D(entity);
    }

    zenith::game::SphereCollider3DView sphereCollider3D(zenith::game::EntityId entity) {
        return zenith::game::Scene::sphereCollider3D(entity);
    }

    void attachBoxCollider3D(zenith::game::EntityId entity, float width, float height, float depth, bool isTrigger) {
        zenith::game::Scene::attachBoxCollider3D(entity, width, height, depth, isTrigger);
    }

    void attachSphereCollider3D(zenith::game::EntityId entity, float radius, bool isTrigger) {
        zenith::game::Scene::attachSphereCollider3D(entity, radius, isTrigger);
    }

    bool overlaps3D(zenith::game::EntityId first, zenith::game::EntityId second) {
        return zenith::game::Scene::overlaps3D(first, second);
    }

    bool containsPoint3D(zenith::game::EntityId entity, float x, float y, float z) {
        return zenith::game::Scene::containsPoint3D(entity, x, y, z);
    }

    zenith::game::RaycastHit3DResult raycast3D(float originX, float originY, float originZ, float directionX, float directionY, float directionZ, float maxDistance) {
        return zenith::game::Scene::raycast3D(originX, originY, originZ, directionX, directionY, directionZ, maxDistance);
    }

    bool followPrimaryCamera2D(zenith::game::EntityId target, float offsetX, float offsetY, float smoothing) {
        return zenith::followPrimaryCamera2D(world, target, zenith::physics::Vec2(offsetX, offsetY), smoothing);
    }

    bool followPrimaryCamera3D(zenith::game::EntityId target, float offsetX, float offsetY, float offsetZ, float smoothing) {
        return zenith::followPrimaryCamera3D(world, target, zenith::physics::Vec3(offsetX, offsetY, offsetZ), smoothing);
    }

    zenith::game::EntityId spawnWallEntity(std::string name, float x, float y, float w, float h, std::string color) {
        zenith::game::EntityId wall = spawnSprite(name, x, y, w, h, color);
        attachBoxCollider2D(wall, w, h, false);
        setEntityLayerMask(wall, 1, 2);
        sprite2D(wall).sortOrder = 2;
        return wall;
    }

    void configureArenaTiles() {
        fillTilemap(arena, 1);
        setTilemapPaletteColor(arena, 1, "#122033");
        setTilemapPaletteColor(arena, 2, "#17344f");
        setTilemapPaletteColor(arena, 3, "#214d24");
        setTilemapCell(arena, 13, 1, 3);
        setTilemapCell(arena, 14, 1, 3);
    }

    void resetRound() {
        zenith::game::Body2DView playerBody = body2D(player);
        playerBody.x = startX;
        playerBody.y = startY;
        playerBody.vx = 0.0;
        playerBody.vy = 0.0;
        batteryCollected = false;
        exitUnlocked = false;
        victory = false;
        moveX = 0.0;
        moveY = 0.0;
        lookX = 1.0;
        lookY = 0.0;
        sprite2D(exitGate).color = "#ffb347";
        sprite2D(battery).visible = true;
        circleCollider2D(battery).enabled = true;
        stopAudio(humAudio);
        stopAudio(pickupAudio);
    }

    bool playerTouchesSolid() {
        if (overlaps2D(player, wallNorth)) {
            return true;
        }
        if (overlaps2D(player, wallSouth)) {
            return true;
        }
        if (overlaps2D(player, wallWest)) {
            return true;
        }
        if (overlaps2D(player, wallEast)) {
            return true;
        }
        if (overlaps2D(player, blockA)) {
            return true;
        }
        if (overlaps2D(player, blockB)) {
            return true;
        }
        return false;
    }

    void movePlayerBy(float deltaX, float deltaY) {
        zenith::game::Body2DView playerBody = body2D(player);
        float originalX = playerBody.x;
        float originalY = playerBody.y;
        if (deltaX != 0.0) {
            playerBody.x = originalX + deltaX;
            if (playerTouchesSolid()) {
                playerBody.x = originalX;
            }
        }
        if (deltaY != 0.0) {
            playerBody.y = originalY + deltaY;
            if (playerTouchesSolid()) {
                playerBody.y = originalY;
            }
        }
    }

    void updateObjectiveState() {
        if (batteryCollected == false) {
            if (overlaps2D(player, battery)) {
                batteryCollected = true;
                exitUnlocked = true;
                sprite2D(battery).visible = false;
                circleCollider2D(battery).enabled = false;
                sprite2D(exitGate).color = "#7cff6b";
                playAudio(pickupAudio);
            }
        }
        if (overlaps2D(player, exitGate)) {
            if (exitUnlocked) {
                victory = true;
                stopAudio(humAudio);
            }
        }
    }

    void normalizeMoveInput() {
        if (moveX != 0.0) {
            if (moveY != 0.0) {
                moveX = moveX * 0.7071;
                moveY = moveY * 0.7071;
            }
        }
    }

    void onLoad() {
        clearColor = "#08121d";
        fixedDeltaTime = 0.0166667;
        autoRenderWorld2D = true;
        autoSimulatePhysics = false;
        debugDrawGrid2D = true;
        debugDrawColliders2D = true;
        debugDrawTransforms2D = true;
        debugDrawCameraBounds2D = true;
        debugDrawRuntimeStats = true;
        debugOverlayEnabled = true;
        zenith::game::TextureHandleView playerTexture = loadTexture("assets/game_sdk/player_runner.png");
        zenith::game::TextureHandleView exitTexture = loadTexture("assets/game_sdk/exit_gate.png");
        zenith::game::TextureHandleView batteryTexture = loadTexture("assets/game_sdk/battery_cell.png");
        zenith::game::AudioHandleView humClip = loadAudio("assets/game_sdk/runner_loop.wav", false);
        zenith::game::AudioHandleView pickupClip = loadAudio("assets/game_sdk/battery_pickup.wav", false);
        createAssetBundle("playable_slice");
        addAssetToBundle("playable_slice", playerTexture.path);
        addAssetToBundle("playable_slice", exitTexture.path);
        addAssetToBundle("playable_slice", batteryTexture.path);
        addAssetToBundle("playable_slice", humClip.path);
        addAssetToBundle("playable_slice", pickupClip.path);
        setAssetMemoryBudget("texture", 524288);
        setAssetMemoryBudget("audio", 262144);
        setAssetHotReload(true);
        arena = spawnTilemap2D("Arena", 0.0, 0.0, 16, 12, 4.0, 4.0, "#122033");
        configureArenaTiles();
        wallNorth = spawnWallEntity("WallNorth", 32.0, 2.0, 64.0, 4.0, "#314b63");
        wallSouth = spawnWallEntity("WallSouth", 32.0, 46.0, 64.0, 4.0, "#314b63");
        wallWest = spawnWallEntity("WallWest", 2.0, 24.0, 4.0, 48.0, "#314b63");
        wallEast = spawnWallEntity("WallEast", 62.0, 24.0, 4.0, 48.0, "#314b63");
        blockA = spawnWallEntity("BlockA", 28.0, 24.0, 8.0, 12.0, "#35506a");
        blockB = spawnWallEntity("BlockB", 46.0, 32.0, 12.0, 8.0, "#35506a");
        player = spawnCharacter2D("Runner", "", startX, startY, 4.0, 4.0, "#8ef9f3");
        character2D(player).texture = playerTexture;
        attachCapsuleCollider2D(player, 5.0, 1.5, false);
        setEntityLayerMask(player, 2, 5);
        body2D(player).gravityScale = 0.0;
        body2D(player).friction = 0.0;
        exitGate = spawnSprite("ExitGate", 56.0, 8.0, 6.0, 6.0, "#ffb347");
        sprite2D(exitGate).texture = exitTexture;
        attachBoxCollider2D(exitGate, 6.0, 6.0, true);
        setEntityLayerMask(exitGate, 4, 2);
        battery = spawnSprite("BatteryCell", 30.0, 8.0, 4.0, 4.0, "#ffe066");
        sprite2D(battery).texture = batteryTexture;
        attachCircleCollider2D(battery, 2.5, true);
        setEntityLayerMask(battery, 4, 2);
        camera = spawnCamera2D("ArenaCamera", startX, startY, 1.0, true);
        listener = spawnAudioListener2D("ArenaListener", startX, startY, true);
        humAudio = spawnAudioSource2D("RunnerHum", "", startX, startY, false);
        pickupAudio = spawnAudioSource2D("PickupChime", "", startX, startY, false);
        audioSource2D(humAudio).clip = humClip;
        audioSource2D(pickupAudio).clip = pickupClip;
        resetRound();
    }

    void onFixedUpdate(float dt) {
        normalizeMoveInput();
        zenith::game::Body2DView playerBody = body2D(player);
        float desiredVx = 0.0;
        float desiredVy = 0.0;
        if (victory == false) {
            desiredVx = moveX * playerSpeed;
            desiredVy = moveY * playerSpeed;
        }
        playerBody.vx = desiredVx;
        playerBody.vy = desiredVy;
        if (desiredVx != 0.0) {
            playAudio(humAudio);
            lookX = moveX;
            lookY = moveY;
        } else {
            if (desiredVy != 0.0) {
                playAudio(humAudio);
                lookX = moveX;
                lookY = moveY;
            }
        }
        movePlayerBy(desiredVx * dt, 0.0);
        movePlayerBy(0.0, desiredVy * dt);
        updateObjectiveState();
        followPrimaryCamera2D(player, 0.0, 0.0, 0.18);
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "configureArenaTiles") { this->configureArenaTiles(); return; }
        if (name == "resetRound") { this->resetRound(); return; }
        if (name == "playerTouchesSolid") { this->playerTouchesSolid(); return; }
        if (name == "updateObjectiveState") { this->updateObjectiveState(); return; }
        if (name == "normalizeMoveInput") { this->normalizeMoveInput(); return; }
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "onFixedUpdate") { try { this->onFixedUpdate(std::stof(val)); } catch(...) {} return; }
    }

    void triggerEntityCallback(std::string name, zenith::game::EntityId entity) override {
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    PlayableSliceTestScene scene = PlayableSliceTestScene();
    scene.load();
    int i = 0;
    scene.moveX = -1.0;
    while (i < 40) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
        i = i + 1;
    }
    zenith::game::Body2DView playerBody = scene.body2D(scene.player);
    println(zenith::concat("wall_block=", playerBody.x >= 5.0));
    scene.moveX = 1.0;
    scene.moveY = 0.0;
    i = 0;
    while (i < 90) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
        i = i + 1;
    }
    println(zenith::concat("battery=", scene.batteryCollected));
    println(zenith::concat("unlocked=", scene.exitUnlocked));
    i = 0;
    while (i < 90) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
        i = i + 1;
    }
    println(zenith::concat("victory=", scene.victory));
    println(zenith::concat("camera_follow=", scene.camera2D(scene.camera).x > 20.0));
    println(zenith::concat("bundle_count=", scene.assetBundleAssetCount("playable_slice")));
    println(zenith::concat("hot_reload=", scene.assetHotReloadEnabled()));
    println(zenith::concat("debug_overlay=", scene.debugOverlayEnabled));
    playerBody = scene.body2D(scene.player);
    zenith::game::RaycastHit2DResult hit = scene.raycast2DMask(playerBody.x, playerBody.y, 1.0, 0.0, 10.0, 1);
    println(zenith::concat("ray_hit=", hit.hit));

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

