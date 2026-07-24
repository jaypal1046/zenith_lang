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

class StarterScene : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId camera;
    zenith::game::EntityId player;
    std::string playerMaterial = "";

    StarterScene()  {}
    StarterScene(zenith::game::EntityId camera, zenith::game::EntityId player, std::string playerMaterial) : camera(camera), player(player), playerMaterial(playerMaterial) {}

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

    void onLoad() {
        name = "StarterScene";
        clearColor = "black";
        fixedDeltaTime = 0.016;
        maxFrameDelta = 0.05;
        maxFixedStepsPerFrame = 6;
        autoRenderWorld2D = true;
        drawEntityNames = false;
        debugOverlayEnabled = true;
        minimalInspectorEnabled = true;
        debugDrawColliders2D = true;
        debugDrawTransforms2D = true;
        debugDrawCameraBounds2D = true;
        debugDrawRuntimeStats = true;
        debugOverlayColor = "cyan";
        camera = spawnCamera2D("Camera", 16.0, 9.0, 1.0, true);
        player = spawnCharacter2D("Player", "assets/textures/player.png", 16.0, 9.0, 2.0, 2.0, "green");
        setEntityTag(player, "hero");
        attachCapsuleCollider2D(player, 1.8, 0.7, false);
        zenith::game::Character2DView hero = character2D(player);
        hero.moveSpeed = 7.0;
        hero.jumpForce = 10.0;
        hero.isGrounded = true;
        playerMaterial = createMaterial("assets/materials/player.mat", "");
        defineMaterialText(playerMaterial, "displayName", "Display Name", "Player");
        defineMaterialRadio(playerMaterial, "mode", "Mode", "idle,run,jump", "idle");
        defineMaterialImage(playerMaterial, "portrait", "Portrait", "assets/textures/player.png");
        defineMaterialButton(playerMaterial, "focus", "Focus", "focusPlayer");
        defineMaterialColor(playerMaterial, "tint", "Tint", "#33dd88");
        defineMaterialToggle(playerMaterial, "ghost", "Ghost", false);
        defineMaterialNumber(playerMaterial, "speed", "Speed", 7.0);
        setMaterialPropertyCallback(playerMaterial, "displayName", "applyDisplayName");
        setMaterialPropertyCallback(playerMaterial, "mode", "applyMode");
        setMaterialPropertyCallback(playerMaterial, "speed", "applySpeed");
        setMaterialTextProperty(playerMaterial, "displayName", "Hero");
        setMaterialRadioProperty(playerMaterial, "mode", "idle");
        setMaterialColorProperty(playerMaterial, "tint", "#33dd88");
        setMaterialToggleProperty(playerMaterial, "ghost", false);
        setMaterialNumberProperty(playerMaterial, "speed", 7.0);
        inspectEntity(player);
        inspectMaterial(playerMaterial);
    }

    void applyDisplayName(std::string value) {
        setEntityName(player, value);
    }

    void applyMode(std::string value) {
        if (value == "run") {
            character2D(player).moveSpeed = 9.0;
        }
        if (value == "jump") {
            character2D(player).jumpForce = 12.0;
        }
        if (value == "idle") {
            character2D(player).moveSpeed = 7.0;
        }
    }

    void applySpeed(float value) {
        character2D(player).moveSpeed = value;
    }

    void focusPlayer() {
        inspectEntity(player);
        inspectMaterial(playerMaterial);
    }

    void onFixedUpdate(float dt) {
        followPrimaryCamera2D(player, 0.0, 0.0, 1.0);
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "applyDisplayName") { this->applyDisplayName(val); return; }
        if (name == "applyMode") { this->applyMode(val); return; }
        if (name == "applySpeed") { try { this->applySpeed(std::stof(val)); } catch(...) {} return; }
        if (name == "focusPlayer") { this->focusPlayer(); return; }
        if (name == "onFixedUpdate") { try { this->onFixedUpdate(std::stof(val)); } catch(...) {} return; }
    }

    void triggerEntityCallback(std::string name, zenith::game::EntityId entity) override {
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    StarterScene scene = StarterScene();
    zenith::runGameLoop(scene);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

