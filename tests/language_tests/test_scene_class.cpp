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

class SceneProbe : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId player;
    zenith::game::EntityId camera;
    zenith::game::EntityId helper;
    zenith::game::EntityId ambience;
    zenith::game::EntityId listener;

    SceneProbe()  {}
    SceneProbe(zenith::game::EntityId player, zenith::game::EntityId camera, zenith::game::EntityId helper, zenith::game::EntityId ambience, zenith::game::EntityId listener) : player(player), camera(camera), helper(helper), ambience(ambience), listener(listener) {}

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

    void onLoad() {
        name = "SceneProbe";
        clearColor = "cyan";
        fixedDeltaTime = 0.02;
        maxFrameDelta = 0.1;
        maxFixedStepsPerFrame = 4;
        autoRenderWorld2D = false;
        drawEntityNames = false;
        player = spawnCharacter2D("Player", "assets/player.png", 10.0, 8.0, 4.0, 2.0, "green");
        setEntityTag(player, "hero");
        camera = spawnCamera2D("Camera", 10.0, 8.0, 1.0, true);
        ambience = spawnAudioSource2D("Ambience", "assets/player_hum.wav", 10.0, 8.0, true);
        listener = spawnAudioListener2D("Listener", 10.0, 8.0, true);
        helper = createEntity("Helper");
        setEntityName(helper, "HelperNode");
        setEntityTag(helper, "support");
        setParent(helper, player);
        setParent(ambience, player);
        setParent(listener, camera);
        zenith::game::Transform2DView player_transform = transform2D(player);
        player_transform.rotation = 0.25;
        player_transform.scaleX = 1.25;
        zenith::game::Transform2DView helper_transform = transform2D(helper);
        helper_transform.x = 11.0;
        helper_transform.y = 8.0;
        zenith::game::Sprite2DView player_sprite = sprite2D(player);
        player_sprite.anchorX = 0.25;
        player_sprite.anchorY = 0.75;
        player_sprite.sortOrder = 2;
        zenith::game::Body2DView player_body = body2D(player);
        player_body.mass = 2.0;
        player_body.gravityScale = 0.0;
        player_body.friction = 0.1;
        player_body.restitution = 0.0;
        player_body.vx = 5.0;
        player_body.vy = 0.0;
        attachCapsuleCollider2D(player, 3.5, 1.1, false);
        attachCircleCollider2D(helper, 0.9, true);
        zenith::game::CapsuleCollider2DView player_hitbox = capsuleCollider2D(player);
        player_hitbox.offsetY = 0.1;
        player_hitbox.height = 3.7;
        zenith::game::CircleCollider2DView helper_trigger = circleCollider2D(helper);
        helper_trigger.offsetX = 0.5;
        helper_trigger.radius = 1.1;
        zenith::game::Character2DView player_character = character2D(player);
        player_character.moveSpeed = 5.0;
        player_character.jumpForce = 9.5;
        player_character.isGrounded = true;
        player_character.facingRight = false;
        zenith::game::AudioSource2DView ambience_audio = audioSource2D(ambience);
        ambience_audio.volume = 0.65;
        ambience_audio.pitch = 1.1;
        ambience_audio.loop = true;
        zenith::game::AudioListener2DView main_listener = audioListener2D(listener);
        main_listener.gain = 0.85;
        zenith::game::Camera2DView main_camera = camera2D(camera);
        main_camera.viewportWidth = 0.8;
        main_camera.viewportHeight = 0.6;
    }

    void onFixedUpdate(float dt) {
        followPrimaryCamera2D(player, 0.0, 0.0, 1.0);
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "onFixedUpdate") { try { this->onFixedUpdate(std::stof(val)); } catch(...) {} return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    SceneProbe scene = SceneProbe();
    scene.load();
    if (scene.isLoaded()) {
        println(scene.name);
    }
    println(scene.clearColor);
    std::string entities = "entities=";
    entities = zenith::concat(entities, scene.entityCount());
    println(entities);
    println(scene.entityName(scene.findEntityByTag("hero")));
    println(scene.character2D(scene.player).texturePath);
    println(scene.audioSource2D(scene.ambience).clipPath);
    std::string listener_primary = "listener_primary=";
    listener_primary = zenith::concat(listener_primary, scene.audioListener2D(scene.listener).primary);
    println(listener_primary);
    std::string children = "children=";
    children = zenith::concat(children, scene.childCount(scene.player));
    println(children);
    println(scene.entityName(scene.parentOf(scene.helper)));
    zenith::game::Transform2DView player_transform = scene.transform2D(scene.player);
    zenith::game::Body2DView player_body = scene.body2D(scene.player);
    zenith::game::Camera2DView main_camera = scene.camera2D(scene.camera);
    std::string start_x = "start_x=";
    start_x = zenith::concat(start_x, player_transform.x);
    println(start_x);
    scene.updateFrame(0.05);
    std::string moved_x = "moved_x=";
    moved_x = zenith::concat(moved_x, scene.transform2D(scene.player).x);
    println(moved_x);
    std::string velocity_x = "vx=";
    velocity_x = zenith::concat(velocity_x, player_body.vx);
    println(velocity_x);
    std::string move_speed = "move_speed=";
    move_speed = zenith::concat(move_speed, scene.character2D(scene.player).moveSpeed);
    println(move_speed);
    std::string jump_force = "jump_force=";
    jump_force = zenith::concat(jump_force, scene.character2D(scene.player).jumpForce);
    println(jump_force);
    std::string audio_volume = "audio_volume=";
    audio_volume = zenith::concat(audio_volume, scene.audioSource2D(scene.ambience).volume);
    println(audio_volume);
    std::string listener_gain = "listener_gain=";
    listener_gain = zenith::concat(listener_gain, scene.audioListener2D(scene.listener).gain);
    println(listener_gain);
    std::string grounded = "grounded=";
    grounded = zenith::concat(grounded, scene.character2D(scene.player).isGrounded);
    println(grounded);
    std::string audio_playing = "audio_playing=";
    audio_playing = zenith::concat(audio_playing, scene.audioSource2D(scene.ambience).isPlaying);
    println(audio_playing);
    std::string facing = "facing_right=";
    facing = zenith::concat(facing, scene.character2D(scene.player).facingRight);
    println(facing);
    std::string zoom = "zoom=";
    zoom = zenith::concat(zoom, main_camera.zoom);
    println(zoom);
    std::string viewport = "viewport_width=";
    viewport = zenith::concat(viewport, main_camera.viewportWidth);
    println(viewport);
    std::string capsule_height = "capsule_height=";
    capsule_height = zenith::concat(capsule_height, scene.capsuleCollider2D(scene.player).height);
    println(capsule_height);
    std::string capsule_radius = "capsule_radius=";
    capsule_radius = zenith::concat(capsule_radius, scene.capsuleCollider2D(scene.player).radius);
    println(capsule_radius);
    std::string trigger_radius = "trigger_radius=";
    trigger_radius = zenith::concat(trigger_radius, scene.circleCollider2D(scene.helper).radius);
    println(trigger_radius);
    std::string overlap = "overlap=";
    overlap = zenith::concat(overlap, scene.overlaps2D(scene.player, scene.helper));
    println(overlap);
    std::string contains = "contains_center=";
    contains = zenith::concat(contains, scene.containsPoint2D(scene.player, scene.transform2D(scene.player).x, scene.transform2D(scene.player).y));
    println(contains);
    zenith::game::RaycastHit2DResult pick = scene.raycast2D(2.0, 8.0, 1.0, 0.0, 20.0);
    std::string ray_hit = "ray_hit=";
    ray_hit = zenith::concat(ray_hit, pick.hit);
    println(ray_hit);
    std::string ray_name = "ray_name=";
    ray_name = zenith::concat(ray_name, scene.entityName(pick.entity));
    println(ray_name);
    std::string ray_distance = "ray_distance=";
    ray_distance = zenith::concat(ray_distance, pick.distance);
    println(ray_distance);
    std::string ray_point_x = "ray_point_x=";
    ray_point_x = zenith::concat(ray_point_x, pick.pointX);
    println(ray_point_x);
    std::string ray_normal_x = "ray_normal_x=";
    ray_normal_x = zenith::concat(ray_normal_x, pick.normalX);
    println(ray_normal_x);
    if (scene.isEntityAlive(scene.camera)) {
        println("camera-alive");
    }
    scene.clearParent(scene.helper);
    std::string cleared = "children_after_clear=";
    cleared = zenith::concat(cleared, scene.childCount(scene.player));
    println(cleared);
    scene.stopAudio(scene.ambience);
    std::string audio_stopped = "audio_stopped=";
    audio_stopped = zenith::concat(audio_stopped, scene.audioSource2D(scene.ambience).isPlaying);
    println(audio_stopped);
    std::string listener_enabled = "listener_enabled=";
    listener_enabled = zenith::concat(listener_enabled, scene.audioListener2D(scene.listener).enabled);
    println(listener_enabled);
    scene.destroyEntity(scene.camera);
    if (!scene.isEntityAlive(scene.camera)) {
        println("camera-destroyed");
    }

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

