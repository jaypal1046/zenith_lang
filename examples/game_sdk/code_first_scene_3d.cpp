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

class CodeFirstScene3D : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId ship;
    zenith::game::EntityId camera;
    zenith::game::EntityId engineAudio;
    zenith::game::EntityId listener;
    zenith::game::EntityId beaconLight;
    zenith::game::EntityId sunLight;
    float elapsed = 0.0;
    std::string shipMaterial = "";
    std::string shipMaterialVariant = "";
    int inspectCount = 0;
    std::string inspectStatus = "Idle";
    std::string surfaceModeStatus = "";
    std::string tintStatus = "";
    bool boostTrailStatus = true;
    float roughnessStatus = 0.0;

    CodeFirstScene3D()  {}
    CodeFirstScene3D(zenith::game::EntityId ship, zenith::game::EntityId camera, zenith::game::EntityId engineAudio, zenith::game::EntityId listener, zenith::game::EntityId beaconLight, zenith::game::EntityId sunLight, float elapsed, std::string shipMaterial, std::string shipMaterialVariant, int inspectCount, std::string inspectStatus, std::string surfaceModeStatus, std::string tintStatus, bool boostTrailStatus, float roughnessStatus) : ship(ship), camera(camera), engineAudio(engineAudio), listener(listener), beaconLight(beaconLight), sunLight(sunLight), elapsed(elapsed), shipMaterial(shipMaterial), shipMaterialVariant(shipMaterialVariant), inspectCount(inspectCount), inspectStatus(inspectStatus), surfaceModeStatus(surfaceModeStatus), tintStatus(tintStatus), boostTrailStatus(boostTrailStatus), roughnessStatus(roughnessStatus) {}

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
        name = "CodeFirstScene3D";
        clearColor = "navy";
        fixedDeltaTime = 0.05;
        autoRenderWorld2D = false;
        drawEntityNames = false;
        zenith::game::MeshHandleView ship_mesh_asset = loadMesh("assets/ship.glb");
        zenith::game::ShaderHandleView ship_shader_asset = loadShader("assets/ship.shader");
        zenith::game::MaterialHandleView ship_material_asset = loadMaterial("assets/ship.mat", "assets/ship.shader");
        zenith::game::AudioHandleView engine_clip_asset = loadAudio("assets/engine.wav", true);
        ship = spawnCharacter3DHandle("Ship", ship_mesh_asset, ship_shader_asset, ship_material_asset, 0.0, 1.0, -6.0);
        camera = spawnCamera3D("OrbitCamera", 0.0, 3.0, 8.0, 75.0, true);
        engineAudio = spawnAudioSource3DHandle("EngineAudio", engine_clip_asset, 0.0, 1.0, -6.0, true);
        listener = spawnAudioListener3D("MainListener3D", 0.0, 3.0, 8.0, true);
        beaconLight = spawnPointLight3D("BeaconLight", 0.0, 1.5, -5.0, "#55d4ff", 1.6, 18.0);
        sunLight = spawnDirectionalLight3D("SunLight", 0.0, 6.0, 0.0, -0.2, -1.0, 0.1, "#fff2c4", 1.2, true);
        setEntityTag(ship, "hero3d");
        setParent(engineAudio, ship);
        setParent(listener, camera);
        setParent(beaconLight, ship);
        zenith::game::Body3DView ship_body = body3D(ship);
        ship_body.mass = 4.0;
        ship_body.vx = 3.0;
        ship_body.vz = -1.0;
        attachSphereCollider3D(ship, 1.6, false);
        attachBoxCollider3D(engineAudio, 2.0, 1.4, 3.0, true);
        zenith::game::Character3DView ship_character = character3D(ship);
        zenith::game::AudioSource3DView engine_audio = audioSource3D(engineAudio);
        zenith::game::AudioListener3DView main_listener = audioListener3D(listener);
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        zenith::game::DirectionalLight3DView sun_light = directionalLight3D(sunLight);
        zenith::game::SphereCollider3DView ship_hull = sphereCollider3D(ship);
        zenith::game::BoxCollider3DView audio_trigger = boxCollider3D(engineAudio);
        ship_character.moveSpeed = 8.0;
        ship_character.turnSpeed = 2.0;
        ship_character.useGravity = false;
        ship_character.isGrounded = true;
        ship_hull.offsetZ = 0.2;
        audio_trigger.offsetX = 0.4;
        audio_trigger.depth = 3.4;
        engine_audio.volume = 0.7;
        engine_audio.loop = true;
        engine_audio.minDistance = 2.0;
        engine_audio.maxDistance = 30.0;
        main_listener.gain = 0.95;
        beacon_light.range = 20.0;
        beacon_light.intensity = 1.8;
        sun_light.directionY = -1.0;
        sun_light.castShadows = true;
        shipMaterial = ship_material_asset.path;
        defineMaterialText(shipMaterial, "displayName", "Display Name", "Sky Runner");
        defineMaterialRadio(shipMaterial, "surfaceMode", "Surface Mode", "opaque,cutout,transparent", "opaque");
        defineMaterialImage(shipMaterial, "albedo", "Albedo", "assets/ship_albedo.png");
        defineMaterialButton(shipMaterial, "inspect", "Inspect", "inspectShip");
        defineMaterialColor(shipMaterial, "tint", "Tint", "#55d4ff");
        defineMaterialToggle(shipMaterial, "boostTrail", "Boost Trail", true);
        defineMaterialNumber(shipMaterial, "roughness", "Roughness", 0.35);
        zenith::game::MaterialPropertyView display_property = materialProperty(shipMaterial, "displayName");
        display_property.stringValue = "Sky Runner";
        display_property.callback = "applyDisplayName";
        zenith::game::MaterialPropertyView surface_property = materialProperty(shipMaterial, "surfaceMode");
        surface_property.options = "opaque,cutout,transparent";
        surface_property.callback = "applySurfaceMode";
        zenith::game::MaterialPropertyOptionView first_surface_option = materialPropertyOption(shipMaterial, "surfaceMode", 0);
        first_surface_option.label = "Opaque";
        zenith::game::MaterialPropertyView tint_property = materialProperty(shipMaterial, "tint");
        tint_property.callback = "applyTint";
        zenith::game::MaterialPropertyView boost_property = materialProperty(shipMaterial, "boostTrail");
        boost_property.callback = "applyBoostTrail";
        zenith::game::MaterialPropertyView roughness_property = materialProperty(shipMaterial, "roughness");
        roughness_property.callback = "applyRoughness";
        shipMaterialVariant = cloneMaterial(shipMaterial, "assets/ship_variant.mat");
        zenith::game::MaterialPropertyView variant_tint = materialProperty(shipMaterialVariant, "tint");
        variant_tint.stringValue = "#ffb347";
        addMaterialPropertyOption(shipMaterialVariant, "surfaceMode", "Hologram", "hologram");
        zenith::game::Mesh3DView ship_mesh = mesh3D(ship);
        ship_mesh.mesh = ship_mesh_asset;
        ship_mesh.shader = ship_shader_asset;
        ship_mesh.material = ship_material_asset;
        ship_mesh.visible = true;
        ship_mesh.castShadows = true;
        notifyMaterialProperty(shipMaterial, "displayName");
        notifyMaterialProperty(shipMaterial, "surfaceMode");
        notifyMaterialProperty(shipMaterial, "tint");
        notifyMaterialProperty(shipMaterial, "boostTrail");
        notifyMaterialProperty(shipMaterial, "roughness");
    }

    void inspectShip() {
        inspectCount = inspectCount + 1;
        inspectStatus = "Inspecting ";
        inspectStatus = zenith::concat(inspectStatus, entityName(ship));
    }

    void applyDisplayName(std::string value) {
        setEntityName(ship, value);
    }

    void applySurfaceMode(std::string value) {
        surfaceModeStatus = value;
        zenith::game::Mesh3DView ship_mesh = mesh3D(ship);
        if (value == "transparent") {
            ship_mesh.castShadows = false;
        } else {
            ship_mesh.castShadows = true;
        }
    }

    void applyTint(std::string value) {
        tintStatus = value;
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        beacon_light.color = value;
    }

    void applyBoostTrail(bool value) {
        boostTrailStatus = value;
        zenith::game::AudioSource3DView engine_audio = audioSource3D(engineAudio);
        engine_audio.loop = value;
    }

    void applyRoughness(float value) {
        roughnessStatus = value;
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        beacon_light.intensity = 1.2 + value;
    }

    void onFrame(float dt) {
        elapsed = elapsed + dt;
        zenith::game::Transform3DView ship_transform = transform3D(ship);
        zenith::game::Camera3DView main_camera = camera3D(camera);
        zenith::game::AudioSource3DView engine_audio = audioSource3D(engineAudio);
        zenith::game::AudioListener3DView main_listener = audioListener3D(listener);
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        zenith::game::DirectionalLight3DView sun_light = directionalLight3D(sunLight);
        zenith::game::MaterialPropertyView surface_property = materialProperty(shipMaterial, "surfaceMode");
        zenith::game::MaterialPropertyView tint_property = materialProperty(shipMaterial, "tint");
        zenith::game::MaterialPropertyView boost_property = materialProperty(shipMaterial, "boostTrail");
        zenith::game::MaterialPropertyView roughness_property = materialProperty(shipMaterial, "roughness");
        ship_transform.rotationY += dt * 0.6;
        ship_transform.rotationX = 0.1;
        if (elapsed > 1.0) {
            if (surfaceModeStatus != "transparent") {
                surface_property.stringValue = "transparent";
                notifyMaterialProperty(shipMaterial, "surfaceMode");
            }
            if (tintStatus != "#9ef7ff") {
                tint_property.stringValue = "#9ef7ff";
                notifyMaterialProperty(shipMaterial, "tint");
            }
        }
        if (elapsed > 1.5) {
            if (boostTrailStatus == true) {
                boost_property.boolValue = false;
                notifyMaterialProperty(shipMaterial, "boostTrail");
            }
        }
        if (elapsed > 2.0) {
            if (roughnessStatus < 0.65) {
                roughness_property.numberValue = 0.65;
                notifyMaterialProperty(shipMaterial, "roughness");
            }
        }
        if (elapsed > 0.75) {
            if (inspectCount == 0) {
                triggerMaterialButton(shipMaterial, "inspect");
            }
        }
        followPrimaryCamera3D(ship, 0.0, 2.5, 9.0, 0.2);
        main_camera.rotationX = -0.15;
        main_listener.x = main_camera.x;
        main_listener.y = main_camera.y;
        main_listener.z = main_camera.z;
        engine_audio.x = ship_transform.x;
        engine_audio.y = ship_transform.y;
        engine_audio.z = ship_transform.z;
        beacon_light.intensity = 1.3 + elapsed * 0.1;
        sun_light.directionX = -0.2 + elapsed * 0.02;
        if (ship_transform.z < -6.5) {
            playAudio(engineAudio);
        } else {
            stopAudio(engineAudio);
        }
        engine_audio.pitch = 0.9 + elapsed * 0.05;
    }

    void onDraw(zenith::Canvas& canvas, float alpha) {
        zenith::game::Transform3DView ship_transform = transform3D(ship);
        zenith::game::Body3DView ship_body = body3D(ship);
        zenith::game::Character3DView ship_character = character3D(ship);
        zenith::game::Camera3DView main_camera = camera3D(camera);
        zenith::game::Mesh3DView ship_mesh = mesh3D(ship);
        zenith::game::AudioSource3DView engine_audio = audioSource3D(engineAudio);
        zenith::game::AudioListener3DView main_listener = audioListener3D(listener);
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        zenith::game::DirectionalLight3DView sun_light = directionalLight3D(sunLight);
        zenith::game::SphereCollider3DView ship_hull = sphereCollider3D(ship);
        zenith::game::BoxCollider3DView audio_trigger = boxCollider3D(engineAudio);
        zenith::game::RaycastHit3DResult pick = raycast3D(main_camera.x, main_camera.y, main_camera.z, 0.0, -0.15, -1.0, 20.0);
        zenith::game::MaterialPropertyView display_property = materialProperty(shipMaterial, "displayName");
        zenith::game::MaterialPropertyView surface_property = materialProperty(shipMaterial, "surfaceMode");
        zenith::game::MaterialPropertyView albedo_property = materialProperty(shipMaterial, "albedo");
        zenith::game::MaterialPropertyView inspect_property = materialProperty(shipMaterial, "inspect");
        zenith::game::MaterialPropertyView tint_property = materialProperty(shipMaterial, "tint");
        zenith::game::MaterialPropertyView first_property = materialPropertyAt(shipMaterial, 0);
        zenith::game::MaterialPropertyView variant_tint = materialProperty(shipMaterialVariant, "tint");
        zenith::game::MaterialPropertyOptionView first_surface_option = materialPropertyOption(shipMaterial, "surfaceMode", 0);
        zenith::game::MaterialPropertyOptionView variant_surface_option = materialPropertyOption(shipMaterialVariant, "surfaceMode", 3);
        std::string header = "Zenith 3D Scene Runtime";
        std::string elapsed_text = "Elapsed: ";
        elapsed_text = zenith::concat(elapsed_text, elapsed);
        std::string position_text = "Ship Pos: ";
        position_text = zenith::concat(position_text, ship_transform.x);
        position_text = zenith::concat(position_text, ", ");
        position_text = zenith::concat(position_text, ship_transform.y);
        position_text = zenith::concat(position_text, ", ");
        position_text = zenith::concat(position_text, ship_transform.z);
        std::string velocity_text = "Ship Vel: ";
        velocity_text = zenith::concat(velocity_text, ship_body.vx);
        velocity_text = zenith::concat(velocity_text, ", ");
        velocity_text = zenith::concat(velocity_text, ship_body.vy);
        velocity_text = zenith::concat(velocity_text, ", ");
        velocity_text = zenith::concat(velocity_text, ship_body.vz);
        std::string mesh_text = "Mesh: ";
        mesh_text = zenith::concat(mesh_text, ship_mesh.meshPath);
        std::string shader_text = "Shader: ";
        shader_text = zenith::concat(shader_text, ship_mesh.shaderPath);
        std::string material_text = "Material: ";
        material_text = zenith::concat(material_text, meshMaterialPath(ship));
        std::string property_text = "Name / Surface: ";
        property_text = zenith::concat(property_text, display_property.stringValue);
        property_text = zenith::concat(property_text, " / ");
        property_text = zenith::concat(property_text, surface_property.stringValue);
        std::string schema_text = "Kind / Label: ";
        schema_text = zenith::concat(schema_text, surface_property.kind);
        schema_text = zenith::concat(schema_text, " / ");
        schema_text = zenith::concat(schema_text, surface_property.label);
        std::string asset_text = "Image / Tint: ";
        asset_text = zenith::concat(asset_text, albedo_property.stringValue);
        asset_text = zenith::concat(asset_text, " / ");
        asset_text = zenith::concat(asset_text, tint_property.stringValue);
        std::string action_text = "Action / Count: ";
        action_text = zenith::concat(action_text, inspect_property.stringValue);
        action_text = zenith::concat(action_text, " / ");
        action_text = zenith::concat(action_text, inspect_property.triggerCount);
        std::string inspect_text = "Inspect / Status: ";
        inspect_text = zenith::concat(inspect_text, inspectCount);
        inspect_text = zenith::concat(inspect_text, " / ");
        inspect_text = zenith::concat(inspect_text, inspectStatus);
        std::string binding_text = "Name / Callback: ";
        binding_text = zenith::concat(binding_text, entityName(ship));
        binding_text = zenith::concat(binding_text, " / ");
        binding_text = zenith::concat(binding_text, surface_property.callback);
        std::string option_text = "First / Options: ";
        option_text = zenith::concat(option_text, first_property.name);
        option_text = zenith::concat(option_text, " / ");
        option_text = zenith::concat(option_text, surface_property.options);
        std::string variant_text = "Variant / Option: ";
        variant_text = zenith::concat(variant_text, variant_tint.stringValue);
        variant_text = zenith::concat(variant_text, " / ");
        variant_text = zenith::concat(variant_text, variant_surface_option.value);
        std::string radio_text = "Radio Label / Count: ";
        radio_text = zenith::concat(radio_text, first_surface_option.label);
        radio_text = zenith::concat(radio_text, " / ");
        radio_text = zenith::concat(radio_text, materialPropertyOptionCount(shipMaterial, "surfaceMode"));
        std::string runtime_text = "Surface / Tint: ";
        runtime_text = zenith::concat(runtime_text, surfaceModeStatus);
        runtime_text = zenith::concat(runtime_text, " / ");
        runtime_text = zenith::concat(runtime_text, pointLight3D(beaconLight).color);
        std::string sync_text = "Boost / Roughness: ";
        sync_text = zenith::concat(sync_text, boostTrailStatus);
        sync_text = zenith::concat(sync_text, " / ");
        sync_text = zenith::concat(sync_text, roughnessStatus);
        std::string character_text = "Move / Turn / Grounded: ";
        character_text = zenith::concat(character_text, ship_character.moveSpeed);
        character_text = zenith::concat(character_text, " / ");
        character_text = zenith::concat(character_text, ship_character.turnSpeed);
        character_text = zenith::concat(character_text, " / ");
        character_text = zenith::concat(character_text, ship_character.isGrounded);
        std::string audio_text = "Audio / Range: ";
        audio_text = zenith::concat(audio_text, engine_audio.clipPath);
        audio_text = zenith::concat(audio_text, " / ");
        audio_text = zenith::concat(audio_text, engine_audio.minDistance);
        audio_text = zenith::concat(audio_text, "-");
        audio_text = zenith::concat(audio_text, engine_audio.maxDistance);
        std::string listener_text = "Listener Gain / Primary: ";
        listener_text = zenith::concat(listener_text, main_listener.gain);
        listener_text = zenith::concat(listener_text, " / ");
        listener_text = zenith::concat(listener_text, main_listener.primary);
        std::string light_text = "Point / Sun: ";
        light_text = zenith::concat(light_text, beacon_light.range);
        light_text = zenith::concat(light_text, " / ");
        light_text = zenith::concat(light_text, sun_light.castShadows);
        std::string camera_text = "Camera Z/FOV: ";
        camera_text = zenith::concat(camera_text, main_camera.z);
        camera_text = zenith::concat(camera_text, " / ");
        camera_text = zenith::concat(camera_text, main_camera.fov);
        std::string tag_text = "Tag: ";
        tag_text = zenith::concat(tag_text, entityTag(ship));
        std::string collider_text = "Hull / Trigger: ";
        collider_text = zenith::concat(collider_text, ship_hull.radius);
        collider_text = zenith::concat(collider_text, " / ");
        collider_text = zenith::concat(collider_text, audio_trigger.depth);
        std::string overlap_text = "Overlap / Contains: ";
        overlap_text = zenith::concat(overlap_text, overlaps3D(ship, engineAudio));
        overlap_text = zenith::concat(overlap_text, " / ");
        overlap_text = zenith::concat(overlap_text, containsPoint3D(ship, ship_transform.x, ship_transform.y, ship_transform.z));
        std::string ray_text = "Ray Hit / Distance: ";
        ray_text = zenith::concat(ray_text, pick.hit);
        ray_text = zenith::concat(ray_text, " / ");
        ray_text = zenith::concat(ray_text, pick.distance);
        canvas.drawText(header, 2.0, 1.0, "yellow");
        canvas.drawText(elapsed_text, 2.0, 3.0, "white");
        canvas.drawText(position_text, 2.0, 5.0, "cyan");
        canvas.drawText(velocity_text, 2.0, 7.0, "green");
        canvas.drawText(mesh_text, 2.0, 9.0, "magenta");
        canvas.drawText(shader_text, 2.0, 11.0, "white");
        canvas.drawText(material_text, 2.0, 13.0, "yellow");
        canvas.drawText(property_text, 2.0, 15.0, "cyan");
        canvas.drawText(schema_text, 2.0, 17.0, "white");
        canvas.drawText(asset_text, 2.0, 19.0, "green");
        canvas.drawText(action_text, 2.0, 21.0, "magenta");
        canvas.drawText(inspect_text, 2.0, 23.0, "white");
        canvas.drawText(binding_text, 2.0, 25.0, "yellow");
        canvas.drawText(option_text, 2.0, 27.0, "yellow");
        canvas.drawText(variant_text, 2.0, 29.0, "white");
        canvas.drawText(radio_text, 2.0, 31.0, "cyan");
        canvas.drawText(runtime_text, 2.0, 33.0, "white");
        canvas.drawText(sync_text, 2.0, 35.0, "green");
        canvas.drawText(character_text, 2.0, 37.0, "white");
        canvas.drawText(audio_text, 2.0, 39.0, "green");
        canvas.drawText(listener_text, 2.0, 41.0, "yellow");
        canvas.drawText(light_text, 2.0, 43.0, "cyan");
        canvas.drawText(camera_text, 2.0, 45.0, "yellow");
        canvas.drawText(tag_text, 2.0, 47.0, "cyan");
        canvas.drawText(collider_text, 2.0, 49.0, "magenta");
        canvas.drawText(overlap_text, 2.0, 51.0, "white");
        canvas.drawText(ray_text, 2.0, 53.0, "green");
        canvas.drawText("3D scene state stays code-first and backend-stable.", 2.0, 56.0, "white");
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "inspectShip") { this->inspectShip(); return; }
        if (name == "applyDisplayName") { this->applyDisplayName(val); return; }
        if (name == "applySurfaceMode") { this->applySurfaceMode(val); return; }
        if (name == "applyTint") { this->applyTint(val); return; }
        if (name == "applyBoostTrail") { this->applyBoostTrail(val == "true"); return; }
        if (name == "applyRoughness") { try { this->applyRoughness(std::stof(val)); } catch(...) {} return; }
        if (name == "onFrame") { try { this->onFrame(std::stof(val)); } catch(...) {} return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    CodeFirstScene3D scene = CodeFirstScene3D();
    zenith::runGameLoop(scene);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

