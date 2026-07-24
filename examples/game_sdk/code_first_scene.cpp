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

extern "C" float _zenith_builtin_getAxis(const char*, const char*);

float getAxis(std::string negativeKey, std::string positiveKey) {
    return ::_zenith_builtin_getAxis(negativeKey.c_str(), positiveKey.c_str());
}

class CodeFirstScene : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId player;
    zenith::game::EntityId camera;
    zenith::game::EntityId reticle;
    zenith::game::EntityId playerHum;
    zenith::game::EntityId listener;
    float elapsed = 0.0;

    CodeFirstScene()  {}
    CodeFirstScene(zenith::game::EntityId player, zenith::game::EntityId camera, zenith::game::EntityId reticle, zenith::game::EntityId playerHum, zenith::game::EntityId listener, float elapsed) : player(player), camera(camera), reticle(reticle), playerHum(playerHum), listener(listener), elapsed(elapsed) {}

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
        name = "CodeFirstScene";
        clearColor = "black";
        fixedDeltaTime = 0.0166667;
        autoRenderWorld2D = true;
        drawEntityNames = false;
        player = spawnCharacter2D("Player", "assets/player.png", 10.0, 9.0, 6.0, 3.0, "green");
        setEntityTag(player, "hero");
        camera = spawnCamera2D("MainCamera", 10.0, 9.0, 1.0, true);
        reticle = spawnSprite("Reticle", 13.0, 5.0, 2.0, 1.0, "white");
        playerHum = spawnAudioSource2D("PlayerHum", "assets/player_hum.wav", 10.0, 9.0, true);
        listener = spawnAudioListener2D("MainListener", 10.0, 9.0, true);
        setParent(reticle, player);
        setParent(playerHum, player);
        setParent(listener, camera);
        attachBody2D(player, 1.0, 0.0, 0.2, 0.0);
        attachCapsuleCollider2D(player, 4.2, 1.4, false);
        attachCircleCollider2D(playerHum, 1.5, true);
        zenith::game::Character2DView player_character = character2D(player);
        zenith::game::AudioSource2DView hum_audio = audioSource2D(playerHum);
        zenith::game::AudioListener2DView main_listener = audioListener2D(listener);
        zenith::game::CapsuleCollider2DView player_hitbox = capsuleCollider2D(player);
        zenith::game::CircleCollider2DView hum_trigger = circleCollider2D(playerHum);
        player_character.moveSpeed = 20.0;
        player_character.jumpForce = 14.0;
        player_character.isGrounded = true;
        player_hitbox.offsetY = 0.1;
        player_hitbox.height = 4.4;
        player_hitbox.radius = 1.5;
        hum_trigger.offsetX = 0.75;
        hum_trigger.radius = 1.25;
        hum_audio.volume = 0.55;
        hum_audio.pitch = 0.95;
        hum_audio.loop = true;
        main_listener.gain = 0.9;
        sprite2D(player).anchorX = 0.5;
        sprite2D(player).anchorY = 0.5;
        sprite2D(player).sortOrder = 1;
        camera2D(camera).viewportWidth = 1.0;
        camera2D(camera).viewportHeight = 1.0;
        println("Scene loaded from pure Zenith code");
    }

    void onFrame(float dt) {
        elapsed = elapsed + dt;
        float axis_x = getAxis("a", "d");
        float axis_y = getAxis("w", "s");
        if (isKeyPressed("ArrowLeft")) {
            axis_x = axis_x - 1.0;
        }
        if (isKeyPressed("ArrowRight")) {
            axis_x = axis_x + 1.0;
        }
        if (isKeyPressed("ArrowUp")) {
            axis_y = axis_y - 1.0;
        }
        if (isKeyPressed("ArrowDown")) {
            axis_y = axis_y + 1.0;
        }
        zenith::game::Body2DView player_body = body2D(player);
        zenith::game::Character2DView player_character = character2D(player);
        zenith::game::Transform2DView player_transform = transform2D(player);
        zenith::game::Transform2DView reticle_transform = transform2D(reticle);
        zenith::game::Camera2DView main_camera = camera2D(camera);
        zenith::game::AudioSource2DView hum_audio = audioSource2D(playerHum);
        zenith::game::AudioListener2DView main_listener = audioListener2D(listener);
        player_body.vx = axis_x * player_character.moveSpeed;
        player_body.vy = axis_y * player_character.moveSpeed;
        player_transform.rotation += axis_x * 0.05;
        reticle_transform.x = player_transform.x + 4.0;
        reticle_transform.y = player_transform.y - 3.0;
        hum_audio.x = player_transform.x;
        hum_audio.y = player_transform.y;
        main_camera.zoom = 1.0 + player_body.vx / 160.0;
        main_listener.x = main_camera.x;
        main_listener.y = main_camera.y;
        if (axis_x < 0.0) {
            player_character.facingRight = false;
        }
        if (axis_x > 0.0) {
            player_character.facingRight = true;
        }
        if (axis_x < 0.0) {
            playAudio(playerHum);
            hum_audio.pitch = 1.05;
        } else {
            if (axis_x > 0.0) {
                playAudio(playerHum);
                hum_audio.pitch = 1.05;
            } else {
                if (axis_y < 0.0) {
                    playAudio(playerHum);
                    hum_audio.pitch = 1.05;
                } else {
                    if (axis_y > 0.0) {
                        playAudio(playerHum);
                        hum_audio.pitch = 1.05;
                    } else {
                        stopAudio(playerHum);
                        hum_audio.pitch = 0.9;
                    }
                }
            }
        }
        if (isKeyPressed("Space")) {
            player_character.color = "yellow";
        } else {
            player_character.color = "green";
        }
    }

    void onFixedUpdate(float dt) {
        followPrimaryCamera2D(player, 0.0, 0.0, 0.2);
    }

    void onDraw(zenith::Canvas& canvas, float alpha) {
        std::string title = "Zenith Scene Runtime";
        std::string alpha_text = "Interpolation alpha: ";
        alpha_text = zenith::concat(alpha_text, alpha);
        std::string time_text = "Elapsed: ";
        time_text = zenith::concat(time_text, elapsed);
        zenith::game::Transform2DView player_transform = transform2D(player);
        zenith::game::Body2DView player_body = body2D(player);
        zenith::game::Character2DView player_character = character2D(player);
        zenith::game::Camera2DView main_camera = camera2D(camera);
        zenith::game::AudioSource2DView hum_audio = audioSource2D(playerHum);
        zenith::game::AudioListener2DView main_listener = audioListener2D(listener);
        zenith::game::CapsuleCollider2DView player_hitbox = capsuleCollider2D(player);
        zenith::game::CircleCollider2DView hum_trigger = circleCollider2D(playerHum);
        zenith::game::RaycastHit2DResult pick = raycast2D(main_camera.x - 12.0, main_camera.y, 1.0, 0.0, 24.0);
        std::string position_text = "Player X: ";
        position_text = zenith::concat(position_text, player_transform.x);
        position_text = zenith::concat(position_text, ", Y: ");
        position_text = zenith::concat(position_text, player_transform.y);
        std::string velocity_text = "Velocity X: ";
        velocity_text = zenith::concat(velocity_text, player_body.vx);
        velocity_text = zenith::concat(velocity_text, ", Y: ");
        velocity_text = zenith::concat(velocity_text, player_body.vy);
        std::string tag_text = "Tag: ";
        tag_text = zenith::concat(tag_text, entityTag(player));
        std::string texture_text = "Texture: ";
        texture_text = zenith::concat(texture_text, player_character.texturePath);
        std::string child_text = "Children: ";
        child_text = zenith::concat(child_text, childCount(player));
        std::string move_text = "Move Speed / Facing: ";
        move_text = zenith::concat(move_text, player_character.moveSpeed);
        move_text = zenith::concat(move_text, " / ");
        move_text = zenith::concat(move_text, player_character.facingRight);
        std::string audio_text = "Audio Clip / Playing: ";
        audio_text = zenith::concat(audio_text, hum_audio.clipPath);
        audio_text = zenith::concat(audio_text, " / ");
        audio_text = zenith::concat(audio_text, hum_audio.isPlaying);
        std::string listener_text = "Listener Gain / Primary: ";
        listener_text = zenith::concat(listener_text, main_listener.gain);
        listener_text = zenith::concat(listener_text, " / ");
        listener_text = zenith::concat(listener_text, main_listener.primary);
        std::string zoom_text = "Camera Zoom: ";
        zoom_text = zenith::concat(zoom_text, main_camera.zoom);
        std::string collider_text = "Capsule / Trigger: ";
        collider_text = zenith::concat(collider_text, player_hitbox.height);
        collider_text = zenith::concat(collider_text, " x ");
        collider_text = zenith::concat(collider_text, player_hitbox.radius);
        collider_text = zenith::concat(collider_text, " / ");
        collider_text = zenith::concat(collider_text, hum_trigger.radius);
        std::string overlap_text = "Overlap / Contains: ";
        overlap_text = zenith::concat(overlap_text, overlaps2D(player, playerHum));
        overlap_text = zenith::concat(overlap_text, " / ");
        overlap_text = zenith::concat(overlap_text, containsPoint2D(player, player_transform.x, player_transform.y));
        std::string ray_text = "Ray Hit / Distance: ";
        ray_text = zenith::concat(ray_text, pick.hit);
        ray_text = zenith::concat(ray_text, " / ");
        ray_text = zenith::concat(ray_text, pick.distance);
        canvas.drawText(title, 2.0, 1.0, "yellow");
        canvas.drawText(alpha_text, 2.0, 3.0, "cyan");
        canvas.drawText(time_text, 2.0, 5.0, "white");
        canvas.drawText(position_text, 2.0, 7.0, "green");
        canvas.drawText(velocity_text, 2.0, 9.0, "magenta");
        canvas.drawText(tag_text, 2.0, 11.0, "white");
        canvas.drawText(texture_text, 2.0, 13.0, "cyan");
        canvas.drawText(child_text, 2.0, 15.0, "yellow");
        canvas.drawText(move_text, 2.0, 17.0, "green");
        canvas.drawText(audio_text, 2.0, 19.0, "cyan");
        canvas.drawText(listener_text, 2.0, 21.0, "yellow");
        canvas.drawText(zoom_text, 2.0, 23.0, "green");
        canvas.drawText(collider_text, 2.0, 25.0, "magenta");
        canvas.drawText(overlap_text, 2.0, 27.0, "white");
        canvas.drawText(ray_text, 2.0, 29.0, "cyan");
        canvas.drawText("Move with WASD or arrows. Space flashes yellow.", 2.0, 32.0, "white");
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "onFrame") { try { this->onFrame(std::stof(val)); } catch(...) {} return; }
        if (name == "onFixedUpdate") { try { this->onFixedUpdate(std::stof(val)); } catch(...) {} return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    CodeFirstScene scene = CodeFirstScene();
    zenith::runGameLoop(scene);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

