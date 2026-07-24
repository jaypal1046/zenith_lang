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

extern "C" bool _zenith_builtin_isKeyPressed(const char*);

bool isKeyPressed(std::string key) {
    return ::_zenith_builtin_isKeyPressed(key.c_str());
}

extern "C" bool _zenith_builtin_wasKeyPressed(const char*);

bool wasKeyPressed(std::string key) {
    return ::_zenith_builtin_wasKeyPressed(key.c_str());
}

extern "C" float _zenith_builtin_getAxis(const char*, const char*);

float getAxis(std::string negativeKey, std::string positiveKey) {
    return ::_zenith_builtin_getAxis(negativeKey.c_str(), positiveKey.c_str());
}

class PlayableSliceScene : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId arena;
    zenith::game::EntityId player;
    zenith::game::EntityId camera;
    zenith::game::EntityId listener;
    zenith::game::EntityId humAudio;
    zenith::game::EntityId pickupAudio;
    zenith::game::EntityId exitGate;
    zenith::game::EntityId battery;
    zenith::game::EntityId wallNorth;
    zenith::game::EntityId wallSouth;
    zenith::game::EntityId wallWest;
    zenith::game::EntityId wallEast;
    zenith::game::EntityId blockA;
    zenith::game::EntityId blockB;
    float startX = 8.0;
    float startY = 8.0;
    float playerSpeed = 18.0;
    float moveX = 0.0;
    float moveY = 0.0;
    float lookX = 1.0;
    float lookY = 0.0;
    float elapsed = 0.0;
    float fixedElapsed = 0.0;
    bool batteryCollected = false;
    bool exitUnlocked = false;
    bool victory = false;
    int inspectSlot = 0;
    std::string inspectorStatus = "player";
    std::string status = "Collect the battery, then reach the exit.";
    std::string collisionStatus = "Idle";

    PlayableSliceScene()  {}
    PlayableSliceScene(zenith::game::EntityId arena, zenith::game::EntityId player, zenith::game::EntityId camera, zenith::game::EntityId listener, zenith::game::EntityId humAudio, zenith::game::EntityId pickupAudio, zenith::game::EntityId exitGate, zenith::game::EntityId battery, zenith::game::EntityId wallNorth, zenith::game::EntityId wallSouth, zenith::game::EntityId wallWest, zenith::game::EntityId wallEast, zenith::game::EntityId blockA, zenith::game::EntityId blockB, float startX, float startY, float playerSpeed, float moveX, float moveY, float lookX, float lookY, float elapsed, float fixedElapsed, bool batteryCollected, bool exitUnlocked, bool victory, int inspectSlot, std::string inspectorStatus, std::string status, std::string collisionStatus) : arena(arena), player(player), camera(camera), listener(listener), humAudio(humAudio), pickupAudio(pickupAudio), exitGate(exitGate), battery(battery), wallNorth(wallNorth), wallSouth(wallSouth), wallWest(wallWest), wallEast(wallEast), blockA(blockA), blockB(blockB), startX(startX), startY(startY), playerSpeed(playerSpeed), moveX(moveX), moveY(moveY), lookX(lookX), lookY(lookY), elapsed(elapsed), fixedElapsed(fixedElapsed), batteryCollected(batteryCollected), exitUnlocked(exitUnlocked), victory(victory), inspectSlot(inspectSlot), inspectorStatus(inspectorStatus), status(status), collisionStatus(collisionStatus) {}

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
        int col = 0;
        while (col < 16) {
            setTilemapCell(arena, col, 1, 2);
            if (col > 8) {
                setTilemapCell(arena, col, 2, 2);
            }
            col = col + 1;
        }
        setTilemapCell(arena, 13, 1, 3);
        setTilemapCell(arena, 14, 1, 3);
        setTilemapCell(arena, 13, 2, 3);
        setTilemapCell(arena, 14, 2, 3);
        setTilemapCell(arena, 7, 5, 2);
        setTilemapCell(arena, 8, 5, 2);
        setTilemapCell(arena, 9, 5, 2);
        setTilemapCell(arena, 11, 8, 2);
        setTilemapCell(arena, 12, 8, 2);
        zenith::game::Tilemap2DView arenaView = tilemap2D(arena);
        arenaView.sortOrder = -8;
    }

    void resetRound() {
        zenith::game::Body2DView playerBody = body2D(player);
        zenith::game::Character2DView playerCharacter = character2D(player);
        zenith::game::Sprite2DView exitSprite = sprite2D(exitGate);
        zenith::game::Sprite2DView batterySprite = sprite2D(battery);
        zenith::game::CircleCollider2DView batteryTrigger = circleCollider2D(battery);
        zenith::game::AudioSource2DView hum = audioSource2D(humAudio);
        zenith::game::AudioSource2DView pickup = audioSource2D(pickupAudio);
        playerBody.x = startX;
        playerBody.y = startY;
        playerBody.vx = 0.0;
        playerBody.vy = 0.0;
        playerCharacter.color = "#8ef9f3";
        playerCharacter.facingRight = true;
        hum.x = startX;
        hum.y = startY;
        hum.loop = true;
        hum.volume = 0.55;
        hum.pitch = 0.95;
        pickup.x = startX;
        pickup.y = startY;
        pickup.volume = 0.9;
        batteryCollected = false;
        exitUnlocked = false;
        victory = false;
        moveX = 0.0;
        moveY = 0.0;
        lookX = 1.0;
        lookY = 0.0;
        collisionStatus = "Idle";
        status = "Collect the battery, then reach the exit.";
        batterySprite.visible = true;
        batteryTrigger.enabled = true;
        exitSprite.color = "#ffb347";
        inspectSlot = 0;
        inspectorStatus = "player";
        inspectEntity(player);
        stopAudio(humAudio);
        stopAudio(pickupAudio);
    }

    void cycleInspectorTarget() {
        inspectSlot = inspectSlot + 1;
        if (inspectSlot > 2) {
            inspectSlot = 0;
        }
        if (inspectSlot == 0) {
            inspectEntity(player);
            inspectorStatus = "player";
        } else {
            if (inspectSlot == 1) {
                inspectEntity(battery);
                inspectorStatus = "battery";
            } else {
                inspectEntity(exitGate);
                inspectorStatus = "exit";
            }
        }
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
        collisionStatus = "Clear";
        if (deltaX != 0.0) {
            playerBody.x = originalX + deltaX;
            if (playerTouchesSolid()) {
                playerBody.x = originalX;
                collisionStatus = "Blocked X";
            }
        }
        if (deltaY != 0.0) {
            playerBody.y = originalY + deltaY;
            if (playerTouchesSolid()) {
                playerBody.y = originalY;
                if (collisionStatus == "Blocked X") {
                    collisionStatus = "Corner";
                } else {
                    collisionStatus = "Blocked Y";
                }
            }
        }
        if (deltaX == 0.0) {
            if (deltaY == 0.0) {
                collisionStatus = "Idle";
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
                status = "Battery secured. Reach the exit.";
                zenith::game::Body2DView playerBody = body2D(player);
                zenith::game::AudioSource2DView pickup = audioSource2D(pickupAudio);
                pickup.x = playerBody.x;
                pickup.y = playerBody.y;
                playAudio(pickupAudio);
            }
        }
        if (overlaps2D(player, exitGate)) {
            if (exitUnlocked) {
                if (victory == false) {
                    victory = true;
                    status = "Exit reached. Press R to restart.";
                    character2D(player).color = "#ffffff";
                    stopAudio(humAudio);
                }
            } else {
                status = "Exit locked. Collect the battery first.";
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
        name = "PlayableSliceScene";
        clearColor = "#08121d";
        fixedDeltaTime = 0.0166667;
        autoRenderWorld2D = true;
        autoSimulatePhysics = false;
        drawEntityNames = false;
        debugDrawGrid2D = true;
        debugDrawColliders2D = true;
        debugDrawTransforms2D = true;
        debugDrawCameraBounds2D = true;
        debugDrawRuntimeStats = true;
        debugGridCellWidth = 4.0;
        debugGridCellHeight = 4.0;
        debugOverlayColor = "#7de2ff";
        debugOverlayEnabled = true;
        minimalInspectorEnabled = true;
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
        character2D(player).moveSpeed = playerSpeed;
        attachCapsuleCollider2D(player, 5.0, 1.5, false);
        setEntityTag(player, "player");
        setEntityLayerMask(player, 2, 5);
        body2D(player).gravityScale = 0.0;
        body2D(player).friction = 0.0;
        sprite2D(player).sortOrder = 4;
        exitGate = spawnSprite("ExitGate", 56.0, 8.0, 6.0, 6.0, "#ffb347");
        sprite2D(exitGate).texture = exitTexture;
        sprite2D(exitGate).sortOrder = 3;
        attachBoxCollider2D(exitGate, 6.0, 6.0, true);
        setEntityTag(exitGate, "goal");
        setEntityLayerMask(exitGate, 4, 2);
        battery = spawnSprite("BatteryCell", 30.0, 8.0, 4.0, 4.0, "#ffe066");
        sprite2D(battery).texture = batteryTexture;
        sprite2D(battery).sortOrder = 3;
        attachCircleCollider2D(battery, 2.5, true);
        setEntityTag(battery, "pickup");
        setEntityLayerMask(battery, 4, 2);
        camera = spawnCamera2D("ArenaCamera", startX, startY, 1.0, true);
        listener = spawnAudioListener2D("ArenaListener", startX, startY, true);
        humAudio = spawnAudioSource2D("RunnerHum", "", startX, startY, false);
        pickupAudio = spawnAudioSource2D("PickupChime", "", startX, startY, false);
        audioSource2D(humAudio).clip = humClip;
        audioSource2D(pickupAudio).clip = pickupClip;
        audioListener2D(listener).gain = 0.95;
        inspectEntity(player);
        resetRound();
    }

    void onFrame(float dt) {
        elapsed = elapsed + dt;
        if (wasKeyPressed("R")) {
            resetRound();
        }
        if (wasKeyPressed("Space")) {
            if (debugOverlayEnabled) {
                debugOverlayEnabled = false;
            } else {
                debugOverlayEnabled = true;
            }
        }
        if (wasKeyPressed("Tab")) {
            cycleInspectorTarget();
        }
        moveX = getAxis("a", "d");
        moveY = getAxis("w", "s");
        if (isKeyPressed("ArrowLeft")) {
            moveX = moveX - 1.0;
        }
        if (isKeyPressed("ArrowRight")) {
            moveX = moveX + 1.0;
        }
        if (isKeyPressed("ArrowUp")) {
            moveY = moveY - 1.0;
        }
        if (isKeyPressed("ArrowDown")) {
            moveY = moveY + 1.0;
        }
        if (victory) {
            moveX = 0.0;
            moveY = 0.0;
        }
        normalizeMoveInput();
        if (moveX != 0.0) {
            lookX = moveX;
            lookY = moveY;
            if (victory == false) {
                playAudio(humAudio);
            }
        } else {
            if (moveY != 0.0) {
                lookX = moveX;
                lookY = moveY;
                if (victory == false) {
                    playAudio(humAudio);
                }
            } else {
                stopAudio(humAudio);
            }
        }
        zenith::game::Body2DView playerBody = body2D(player);
        zenith::game::AudioSource2DView hum = audioSource2D(humAudio);
        hum.x = playerBody.x;
        hum.y = playerBody.y;
        zenith::game::Camera2DView mainCamera = camera2D(camera);
        zenith::game::AudioListener2DView mainListener = audioListener2D(listener);
        mainListener.x = mainCamera.x;
        mainListener.y = mainCamera.y;
        if (exitUnlocked) {
            mainCamera.zoom = 1.05;
        } else {
            mainCamera.zoom = 1.0;
        }
    }

    void onFixedUpdate(float dt) {
        fixedElapsed = fixedElapsed + dt;
        zenith::game::Body2DView playerBody = body2D(player);
        float desiredVx = 0.0;
        float desiredVy = 0.0;
        if (victory == false) {
            desiredVx = moveX * playerSpeed;
            desiredVy = moveY * playerSpeed;
        }
        playerBody.vx = desiredVx;
        playerBody.vy = desiredVy;
        movePlayerBy(desiredVx * dt, 0.0);
        movePlayerBy(0.0, desiredVy * dt);
        updateObjectiveState();
        followPrimaryCamera2D(player, 0.0, 0.0, 0.18);
    }

    void onDraw(zenith::Canvas& canvas, float alpha) {
        zenith::game::Body2DView playerBody = body2D(player);
        zenith::game::Camera2DView mainCamera = camera2D(camera);
        zenith::game::RaycastHit2DResult lookHit = raycast2DMask(playerBody.x, playerBody.y, lookX, lookY, 18.0, 1);
        std::string hitName = "none";
        if (lookHit.hit) {
            hitName = entityName(lookHit.entity);
        }
        std::string title = "Zenith Playable Slice";
        std::string statusText = "Status: ";
        statusText = zenith::concat(statusText, status);
        std::string positionText = "Player: ";
        positionText = zenith::concat(positionText, playerBody.x);
        positionText = zenith::concat(positionText, ", ");
        positionText = zenith::concat(positionText, playerBody.y);
        std::string velocityText = "Velocity: ";
        velocityText = zenith::concat(velocityText, playerBody.vx);
        velocityText = zenith::concat(velocityText, ", ");
        velocityText = zenith::concat(velocityText, playerBody.vy);
        std::string objectiveText = "Battery / Exit / Win: ";
        objectiveText = zenith::concat(objectiveText, batteryCollected);
        objectiveText = zenith::concat(objectiveText, " / ");
        objectiveText = zenith::concat(objectiveText, exitUnlocked);
        objectiveText = zenith::concat(objectiveText, " / ");
        objectiveText = zenith::concat(objectiveText, victory);
        std::string collisionText = "Collision: ";
        collisionText = zenith::concat(collisionText, collisionStatus);
        std::string rayText = "Ray Hit: ";
        rayText = zenith::concat(rayText, hitName);
        rayText = zenith::concat(rayText, " @ ");
        rayText = zenith::concat(rayText, lookHit.distance);
        std::string inspectorText = "Inspector Target: ";
        inspectorText = zenith::concat(inspectorText, inspectorStatus);
        std::string assetText = "Bundle Assets / Hot Reload: ";
        assetText = zenith::concat(assetText, assetBundleAssetCount("playable_slice"));
        assetText = zenith::concat(assetText, " / ");
        assetText = zenith::concat(assetText, assetHotReloadEnabled());
        std::string memoryText = "Texture Memory: ";
        memoryText = zenith::concat(memoryText, assetMemoryUsage("texture"));
        memoryText = zenith::concat(memoryText, " / ");
        memoryText = zenith::concat(memoryText, assetMemoryBudget("texture"));
        std::string timingText = "Elapsed / Fixed / Alpha: ";
        timingText = zenith::concat(timingText, elapsed);
        timingText = zenith::concat(timingText, " / ");
        timingText = zenith::concat(timingText, fixedElapsed);
        timingText = zenith::concat(timingText, " / ");
        timingText = zenith::concat(timingText, alpha);
        std::string cameraText = "Camera / Interact: ";
        cameraText = zenith::concat(cameraText, mainCamera.x);
        cameraText = zenith::concat(cameraText, ", ");
        cameraText = zenith::concat(cameraText, mainCamera.y);
        cameraText = zenith::concat(cameraText, " / ");
        cameraText = zenith::concat(cameraText, canEntitiesInteract(player, exitGate));
        canvas.drawText(title, 2.0, 1.0, "yellow");
        canvas.drawText(statusText, 2.0, 3.0, "white");
        canvas.drawText(positionText, 2.0, 5.0, "#8ef9f3");
        canvas.drawText(velocityText, 2.0, 7.0, "#7de2ff");
        canvas.drawText(objectiveText, 2.0, 9.0, "#ffe066");
        canvas.drawText(collisionText, 2.0, 11.0, "#ff8fab");
        canvas.drawText(rayText, 2.0, 13.0, "#c4b5fd");
        canvas.drawText(inspectorText, 2.0, 15.0, "#ffd6a5");
        canvas.drawText(assetText, 2.0, 17.0, "#b8f2e6");
        canvas.drawText(memoryText, 2.0, 19.0, "#bde0fe");
        canvas.drawText(timingText, 2.0, 21.0, "#ffffff");
        canvas.drawText(cameraText, 2.0, 23.0, "#9bf6ff");
        canvas.drawText("WASD/Arrows move. Space toggles debug. Tab cycles inspector. R resets.", 2.0, 26.0, "white");
        canvas.drawLine(playerBody.x, playerBody.y, playerBody.x + lookX * 6.0, playerBody.y + lookY * 6.0, "white");
        if (lookHit.hit) {
            canvas.drawPoint(lookHit.pointX, lookHit.pointY, "#ff6b6b");
        }
        if (!batteryCollected) {
            canvas.drawCircleOutline(transform2D(battery).x, transform2D(battery).y, 3.0, "#ffe066");
        }
        canvas.drawFrameRect(transform2D(exitGate).x - 3.0, transform2D(exitGate).y - 3.0, 6.0, 6.0, sprite2D(exitGate).color);
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "configureArenaTiles") { this->configureArenaTiles(); return; }
        if (name == "resetRound") { this->resetRound(); return; }
        if (name == "cycleInspectorTarget") { this->cycleInspectorTarget(); return; }
        if (name == "playerTouchesSolid") { this->playerTouchesSolid(); return; }
        if (name == "updateObjectiveState") { this->updateObjectiveState(); return; }
        if (name == "normalizeMoveInput") { this->normalizeMoveInput(); return; }
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "onFrame") { try { this->onFrame(std::stof(val)); } catch(...) {} return; }
        if (name == "onFixedUpdate") { try { this->onFixedUpdate(std::stof(val)); } catch(...) {} return; }
    }

    void triggerEntityCallback(std::string name, zenith::game::EntityId entity) override {
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    PlayableSliceScene scene = PlayableSliceScene();
    zenith::runGameLoop(scene);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

