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

class Scene3DProbe : public zenith::game::Scene {
private:
public:
    zenith::game::EntityId ship;
    zenith::game::EntityId camera;
    zenith::game::EntityId marker;
    zenith::game::EntityId engineAudio;
    zenith::game::EntityId listener;
    zenith::game::EntityId beaconLight;
    zenith::game::EntityId sunLight;
    std::string shipMaterial = "";
    int inspectCount = 0;
    std::string inspectTarget = "";
    std::string displayNameEvent = "";
    std::string surfaceModeEvent = "";
    std::string previewEvent = "";
    std::string tintEvent = "";
    bool glowEvent = false;
    float roughnessEvent = 0.0;

    Scene3DProbe()  {}
    Scene3DProbe(zenith::game::EntityId ship, zenith::game::EntityId camera, zenith::game::EntityId marker, zenith::game::EntityId engineAudio, zenith::game::EntityId listener, zenith::game::EntityId beaconLight, zenith::game::EntityId sunLight, std::string shipMaterial, int inspectCount, std::string inspectTarget, std::string displayNameEvent, std::string surfaceModeEvent, std::string previewEvent, std::string tintEvent, bool glowEvent, float roughnessEvent) : ship(ship), camera(camera), marker(marker), engineAudio(engineAudio), listener(listener), beaconLight(beaconLight), sunLight(sunLight), shipMaterial(shipMaterial), inspectCount(inspectCount), inspectTarget(inspectTarget), displayNameEvent(displayNameEvent), surfaceModeEvent(surfaceModeEvent), previewEvent(previewEvent), tintEvent(tintEvent), glowEvent(glowEvent), roughnessEvent(roughnessEvent) {}

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
        name = "Scene3DProbe";
        clearColor = "teal";
        fixedDeltaTime = 0.05;
        autoRenderWorld2D = false;
        drawEntityNames = false;
        zenith::game::MeshHandleView ship_mesh_asset = loadMesh("assets/ship.glb");
        zenith::game::ShaderHandleView ship_shader_asset = loadShader("assets/ship.shader");
        zenith::game::MaterialHandleView ship_material_asset = loadMaterial("assets/ship.mat", "assets/ship.shader");
        zenith::game::AudioHandleView engine_clip_asset = loadAudio("assets/engine.wav", true);
        ship = spawnCharacter3DHandle("Ship", ship_mesh_asset, ship_shader_asset, ship_material_asset, 3.0, 1.5, -7.0);
        camera = spawnCamera3D("Camera3D", 0.0, 2.5, 6.0, 70.0, true);
        engineAudio = spawnAudioSource3DHandle("EngineAudio", engine_clip_asset, 3.0, 1.5, -7.0, true);
        listener = spawnAudioListener3D("Listener3D", 0.0, 2.5, 6.0, true);
        beaconLight = spawnPointLight3D("BeaconLight", 3.0, 2.0, -6.0, "#44ccff", 1.5, 16.0);
        sunLight = spawnDirectionalLight3D("SunLight", 0.0, 6.0, 0.0, -0.1, -1.0, 0.2, "#ffeeaa", 1.1, true);
        marker = createEntity("Marker");
        setEntityTag(ship, "hero3d");
        setParent(marker, ship);
        setParent(engineAudio, ship);
        setParent(listener, camera);
        setParent(beaconLight, ship);
        zenith::game::Transform3DView ship_transform = transform3D(ship);
        ship_transform.rotationY = 0.3;
        ship_transform.scaleZ = 1.5;
        zenith::game::Transform3DView marker_transform = transform3D(marker);
        marker_transform.x = 3.4;
        marker_transform.y = 1.5;
        marker_transform.z = -7.3;
        zenith::game::Body3DView ship_body = body3D(ship);
        ship_body.mass = 5.0;
        ship_body.vx = 6.0;
        ship_body.vy = 0.0;
        ship_body.vz = -2.0;
        attachSphereCollider3D(ship, 1.4, false);
        attachBoxCollider3D(marker, 2.2, 1.6, 2.5, true);
        zenith::game::SphereCollider3DView ship_hull = sphereCollider3D(ship);
        ship_hull.offsetZ = 0.1;
        zenith::game::BoxCollider3DView marker_trigger = boxCollider3D(marker);
        marker_trigger.offsetX = 0.2;
        marker_trigger.depth = 2.5;
        zenith::game::Character3DView ship_character = character3D(ship);
        ship_character.moveSpeed = 9.0;
        ship_character.turnSpeed = 2.5;
        ship_character.isGrounded = true;
        ship_character.useGravity = false;
        zenith::game::AudioSource3DView engine_audio = audioSource3D(engineAudio);
        engine_audio.volume = 0.7;
        engine_audio.loop = true;
        engine_audio.minDistance = 2.0;
        engine_audio.maxDistance = 30.0;
        zenith::game::AudioListener3DView main_listener = audioListener3D(listener);
        main_listener.gain = 0.9;
        zenith::game::PointLight3DView beacon_light = pointLight3D(beaconLight);
        beacon_light.range = 18.0;
        beacon_light.enabled = true;
        zenith::game::DirectionalLight3DView sun_light = directionalLight3D(sunLight);
        sun_light.directionX = -0.15;
        sun_light.castShadows = true;
        zenith::game::Camera3DView main_camera = camera3D(camera);
        main_camera.nearClip = 0.2;
        main_camera.viewportHeight = 0.75;
        shipMaterial = ship_material_asset.path;
        defineMaterialText(shipMaterial, "displayName", "Display Name", "Ship Commander");
        defineMaterialRadio(shipMaterial, "surfaceMode", "Surface Mode", "opaque,transparent,wireframe", "opaque");
        defineMaterialImage(shipMaterial, "preview", "Preview", "assets/ship_preview.png");
        defineMaterialButton(shipMaterial, "inspect", "Inspect", "inspectShip");
        defineMaterialColor(shipMaterial, "tint", "Tint", "#44ccff");
        defineMaterialToggle(shipMaterial, "glow", "Glow", true);
        defineMaterialNumber(shipMaterial, "roughness", "Roughness", 0.4);
        zenith::game::MaterialPropertyView display_property = materialProperty(shipMaterial, "displayName");
        display_property.stringValue = "Flagship";
        display_property.callback = "applyDisplayName";
        zenith::game::MaterialPropertyView surface_property = materialProperty(shipMaterial, "surfaceMode");
        surface_property.stringValue = "transparent";
        surface_property.options = "opaque,transparent,wireframe";
        surface_property.callback = "applySurfaceMode";
        zenith::game::MaterialPropertyView preview_property = materialProperty(shipMaterial, "preview");
        preview_property.callback = "applyPreview";
        zenith::game::MaterialPropertyView tint_property = materialProperty(shipMaterial, "tint");
        tint_property.stringValue = "#88ffee";
        tint_property.callback = "applyTint";
        zenith::game::MaterialPropertyView glow_property = materialProperty(shipMaterial, "glow");
        glow_property.callback = "applyGlow";
        zenith::game::MaterialPropertyView roughness_property = materialProperty(shipMaterial, "roughness");
        roughness_property.callback = "applyRoughness";
        zenith::game::Mesh3DView ship_mesh = mesh3D(ship);
        ship_mesh.mesh = ship_mesh_asset;
        ship_mesh.shader = ship_shader_asset;
        ship_mesh.material = ship_material_asset;
        ship_mesh.castShadows = false;
    }

    void inspectShip() {
        inspectCount = inspectCount + 1;
        inspectTarget = entityName(ship);
    }

    void applyDisplayName(std::string value) {
        displayNameEvent = value;
    }

    void applySurfaceMode(std::string value) {
        surfaceModeEvent = value;
    }

    void applyPreview(std::string value) {
        previewEvent = value;
    }

    void applyTint(std::string value) {
        tintEvent = value;
    }

    void applyGlow(bool value) {
        glowEvent = value;
    }

    void applyRoughness(float value) {
        roughnessEvent = value;
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "inspectShip") { this->inspectShip(); return; }
        if (name == "applyDisplayName") { this->applyDisplayName(val); return; }
        if (name == "applySurfaceMode") { this->applySurfaceMode(val); return; }
        if (name == "applyPreview") { this->applyPreview(val); return; }
        if (name == "applyTint") { this->applyTint(val); return; }
        if (name == "applyGlow") { this->applyGlow(val == "true"); return; }
        if (name == "applyRoughness") { try { this->applyRoughness(std::stof(val)); } catch(...) {} return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    Scene3DProbe scene = Scene3DProbe();
    scene.load();
    println(scene.name);
    println(scene.clearColor);
    println(scene.entityName(scene.findEntityByTag("hero3d")));
    println(scene.mesh3D(scene.ship).meshPath);
    println(scene.mesh3D(scene.ship).shaderPath);
    println(scene.meshMaterialPath(scene.ship));
    println(scene.materialTextProperty(scene.shipMaterial, "displayName"));
    println(scene.materialRadioProperty(scene.shipMaterial, "surfaceMode"));
    println(scene.materialImageProperty(scene.shipMaterial, "preview"));
    println(scene.materialButtonProperty(scene.shipMaterial, "inspect"));
    println(scene.materialColorProperty(scene.shipMaterial, "tint"));
    println(scene.character3D(scene.ship).meshPath);
    println(scene.audioSource3D(scene.engineAudio).clipPath);
    zenith::game::MeshHandleView ship_mesh_handle = scene.mesh3D(scene.ship).mesh;
    zenith::game::ShaderHandleView ship_shader_handle = scene.mesh3D(scene.ship).shader;
    zenith::game::MaterialHandleView ship_material_handle = scene.mesh3D(scene.ship).material;
    zenith::game::AudioHandleView engine_clip_handle = scene.audioSource3D(scene.engineAudio).clip;
    std::string mesh_handle_id = "mesh_handle_id=";
    mesh_handle_id = zenith::concat(mesh_handle_id, ship_mesh_handle.id);
    println(mesh_handle_id);
    std::string mesh_handle_loaded = "mesh_handle_loaded=";
    mesh_handle_loaded = zenith::concat(mesh_handle_loaded, ship_mesh_handle.loaded);
    println(mesh_handle_loaded);
    std::string shader_handle_id = "shader_handle_id=";
    shader_handle_id = zenith::concat(shader_handle_id, ship_shader_handle.id);
    println(shader_handle_id);
    std::string material_handle_props = "material_handle_props=";
    material_handle_props = zenith::concat(material_handle_props, ship_material_handle.propertyCount);
    println(material_handle_props);
    std::string material_handle_shader = "material_handle_shader=";
    material_handle_shader = zenith::concat(material_handle_shader, ship_material_handle.shaderPath);
    println(material_handle_shader);
    std::string audio_handle_spatial = "audio_handle_spatial=";
    audio_handle_spatial = zenith::concat(audio_handle_spatial, engine_clip_handle.spatial);
    println(audio_handle_spatial);
    std::string listener_primary = "listener_primary=";
    listener_primary = zenith::concat(listener_primary, scene.audioListener3D(scene.listener).primary);
    println(listener_primary);
    std::string point_color = "point_color=";
    point_color = zenith::concat(point_color, scene.pointLight3D(scene.beaconLight).color);
    println(point_color);
    std::string children = "children=";
    children = zenith::concat(children, scene.childCount(scene.ship));
    println(children);
    std::string start_z = "start_z=";
    start_z = zenith::concat(start_z, scene.entityPositionZ(scene.ship));
    println(start_z);
    std::string primary = "camera_primary=";
    primary = zenith::concat(primary, scene.camera3D(scene.camera).primary);
    println(primary);
    scene.updateFrame(0.05);
    scene.followPrimaryCamera3D(scene.ship, 1.0, 0.5, 8.0, 1.0);
    std::string moved_x = "moved_x=";
    moved_x = zenith::concat(moved_x, scene.transform3D(scene.ship).x);
    println(moved_x);
    std::string moved_z = "moved_z=";
    moved_z = zenith::concat(moved_z, scene.transform3D(scene.ship).z);
    println(moved_z);
    std::string velocity_z = "vz=";
    velocity_z = zenith::concat(velocity_z, scene.body3D(scene.ship).vz);
    println(velocity_z);
    std::string gravity = "use_gravity=";
    gravity = zenith::concat(gravity, scene.character3D(scene.ship).useGravity);
    println(gravity);
    std::string fov = "fov=";
    fov = zenith::concat(fov, scene.camera3D(scene.camera).fov);
    println(fov);
    std::string near_clip = "near=";
    near_clip = zenith::concat(near_clip, scene.camera3D(scene.camera).nearClip);
    println(near_clip);
    std::string viewport = "viewport_h=";
    viewport = zenith::concat(viewport, scene.camera3D(scene.camera).viewportHeight);
    println(viewport);
    std::string camera_follow_x = "camera_follow_x=";
    camera_follow_x = zenith::concat(camera_follow_x, scene.camera3D(scene.camera).x);
    println(camera_follow_x);
    std::string camera_follow_y = "camera_follow_y=";
    camera_follow_y = zenith::concat(camera_follow_y, scene.camera3D(scene.camera).y);
    println(camera_follow_y);
    std::string camera_follow_z = "camera_follow_z=";
    camera_follow_z = zenith::concat(camera_follow_z, scene.camera3D(scene.camera).z);
    println(camera_follow_z);
    std::string material_count = "material_props=";
    material_count = zenith::concat(material_count, scene.materialPropertyCount(scene.shipMaterial));
    println(material_count);
    std::string has_preview = "has_preview=";
    has_preview = zenith::concat(has_preview, scene.materialHasProperty(scene.shipMaterial, "preview"));
    println(has_preview);
    zenith::game::MaterialPropertyView surface_property = scene.materialProperty(scene.shipMaterial, "surfaceMode");
    zenith::game::MaterialPropertyView first_property_view = scene.materialPropertyAt(scene.shipMaterial, 0);
    zenith::game::MaterialPropertyView display_property = scene.materialProperty(scene.shipMaterial, "displayName");
    zenith::game::MaterialPropertyView preview_property = scene.materialProperty(scene.shipMaterial, "preview");
    zenith::game::MaterialPropertyView inspect_property = scene.materialProperty(scene.shipMaterial, "inspect");
    zenith::game::MaterialPropertyView roughness_property = scene.materialProperty(scene.shipMaterial, "roughness");
    zenith::game::MaterialPropertyView glow_property = scene.materialProperty(scene.shipMaterial, "glow");
    zenith::game::MaterialPropertyView tint_property = scene.materialProperty(scene.shipMaterial, "tint");
    zenith::game::MaterialPropertyOptionView first_surface_option = scene.materialPropertyOption(scene.shipMaterial, "surfaceMode", 0);
    first_surface_option.label = "Opaque Mode";
    std::string first_property = "property_0=";
    first_property = zenith::concat(first_property, first_property_view.name);
    println(first_property);
    std::string surface_kind = "surface_kind=";
    surface_kind = zenith::concat(surface_kind, surface_property.kind);
    println(surface_kind);
    std::string surface_label = "surface_label=";
    surface_label = zenith::concat(surface_label, surface_property.label);
    println(surface_label);
    std::string surface_options = "surface_options=";
    surface_options = zenith::concat(surface_options, surface_property.options);
    println(surface_options);
    std::string surface_exists = "surface_exists=";
    surface_exists = zenith::concat(surface_exists, surface_property.exists);
    println(surface_exists);
    std::string surface_value = "surface_value=";
    surface_value = zenith::concat(surface_value, surface_property.stringValue);
    println(surface_value);
    std::string surface_callback = "surface_callback=";
    surface_callback = zenith::concat(surface_callback, surface_property.callback);
    println(surface_callback);
    std::string surface_option_count = "surface_option_count=";
    surface_option_count = zenith::concat(surface_option_count, scene.materialPropertyOptionCount(scene.shipMaterial, "surfaceMode"));
    println(surface_option_count);
    std::string surface_option_exists = "surface_option_exists=";
    surface_option_exists = zenith::concat(surface_option_exists, first_surface_option.exists);
    println(surface_option_exists);
    std::string surface_option_label = "surface_option_label=";
    surface_option_label = zenith::concat(surface_option_label, first_surface_option.label);
    println(surface_option_label);
    std::string surface_option_value = "surface_option_value=";
    surface_option_value = zenith::concat(surface_option_value, first_surface_option.value);
    println(surface_option_value);
    std::string display_notified = "display_notified=";
    display_notified = zenith::concat(display_notified, scene.notifyMaterialProperty(scene.shipMaterial, "displayName"));
    println(display_notified);
    std::string display_event = "display_event=";
    display_event = zenith::concat(display_event, scene.displayNameEvent);
    println(display_event);
    std::string preview_notified = "preview_notified=";
    preview_notified = zenith::concat(preview_notified, scene.notifyMaterialProperty(scene.shipMaterial, "preview"));
    println(preview_notified);
    std::string preview_event = "preview_event=";
    preview_event = zenith::concat(preview_event, scene.previewEvent);
    println(preview_event);
    std::string surface_notified = "surface_notified=";
    surface_notified = zenith::concat(surface_notified, scene.notifyMaterialProperty(scene.shipMaterial, "surfaceMode"));
    println(surface_notified);
    std::string surface_event = "surface_event=";
    surface_event = zenith::concat(surface_event, scene.surfaceModeEvent);
    println(surface_event);
    std::string surface_notify_count = "surface_notify_count=";
    surface_notify_count = zenith::concat(surface_notify_count, surface_property.triggerCount);
    println(surface_notify_count);
    std::string tint_notified = "tint_notified=";
    tint_notified = zenith::concat(tint_notified, scene.notifyMaterialProperty(scene.shipMaterial, "tint"));
    println(tint_notified);
    std::string tint_event = "tint_event=";
    tint_event = zenith::concat(tint_event, scene.tintEvent);
    println(tint_event);
    std::string glow_notified = "glow_notified=";
    glow_notified = zenith::concat(glow_notified, scene.notifyMaterialProperty(scene.shipMaterial, "glow"));
    println(glow_notified);
    std::string glow_event = "glow_event=";
    glow_event = zenith::concat(glow_event, scene.glowEvent);
    println(glow_event);
    std::string roughness_notified = "roughness_notified=";
    roughness_notified = zenith::concat(roughness_notified, scene.notifyMaterialProperty(scene.shipMaterial, "roughness"));
    println(roughness_notified);
    std::string roughness_event = "roughness_event=";
    roughness_event = zenith::concat(roughness_event, scene.roughnessEvent);
    println(roughness_event);
    std::string button_triggered = "button_triggered=";
    button_triggered = zenith::concat(button_triggered, scene.triggerMaterialButton(scene.shipMaterial, "inspect"));
    println(button_triggered);
    std::string button_triggers = "button_triggers=";
    button_triggers = zenith::concat(button_triggers, scene.materialButtonTriggerCount(scene.shipMaterial, "inspect"));
    println(button_triggers);
    std::string inspect_view_triggers = "inspect_view_triggers=";
    inspect_view_triggers = zenith::concat(inspect_view_triggers, inspect_property.triggerCount);
    println(inspect_view_triggers);
    std::string inspect_count = "inspect_count=";
    inspect_count = zenith::concat(inspect_count, scene.inspectCount);
    println(inspect_count);
    std::string inspect_target = "inspect_target=";
    inspect_target = zenith::concat(inspect_target, scene.inspectTarget);
    println(inspect_target);
    std::string variant_material = scene.cloneMaterial(scene.shipMaterial, "assets/ship_variant.mat");
    zenith::game::MaterialPropertyView variant_tint = scene.materialProperty(variant_material, "tint");
    variant_tint.stringValue = "#ffaa33";
    scene.addMaterialPropertyOption(variant_material, "surfaceMode", "Ghost", "ghost");
    zenith::game::MaterialPropertyOptionView variant_surface_option = scene.materialPropertyOption(variant_material, "surfaceMode", 3);
    std::string variant_exists = "variant_exists=";
    variant_exists = zenith::concat(variant_exists, scene.materialExists(variant_material));
    println(variant_exists);
    std::string variant_shader = "variant_shader=";
    variant_shader = zenith::concat(variant_shader, scene.materialShaderPath(variant_material));
    println(variant_shader);
    std::string variant_color = "variant_color=";
    variant_color = zenith::concat(variant_color, variant_tint.stringValue);
    println(variant_color);
    std::string variant_option_count = "variant_option_count=";
    variant_option_count = zenith::concat(variant_option_count, scene.materialPropertyOptionCount(variant_material, "surfaceMode"));
    println(variant_option_count);
    std::string variant_option_label = "variant_option_label=";
    variant_option_label = zenith::concat(variant_option_label, variant_surface_option.label);
    println(variant_option_label);
    std::string removed_variant_option = "removed_variant_option=";
    removed_variant_option = zenith::concat(removed_variant_option, scene.removeMaterialPropertyOption(variant_material, "surfaceMode", 3));
    println(removed_variant_option);
    std::string variant_option_count_after_remove = "variant_option_count_after_remove=";
    variant_option_count_after_remove = zenith::concat(variant_option_count_after_remove, scene.materialPropertyOptionCount(variant_material, "surfaceMode"));
    println(variant_option_count_after_remove);
    std::string cleared_variant_options = "cleared_variant_options=";
    cleared_variant_options = zenith::concat(cleared_variant_options, scene.clearMaterialPropertyOptions(variant_material, "surfaceMode"));
    println(cleared_variant_options);
    std::string variant_option_count_after_clear = "variant_option_count_after_clear=";
    variant_option_count_after_clear = zenith::concat(variant_option_count_after_clear, scene.materialPropertyOptionCount(variant_material, "surfaceMode"));
    println(variant_option_count_after_clear);
    std::string scratch_material = scene.createMaterial("assets/ship_scratch.mat", "");
    std::string copied_props = "copied_props=";
    copied_props = zenith::concat(copied_props, scene.copyMaterialProperties(scene.shipMaterial, scratch_material));
    println(copied_props);
    std::string scratch_has_inspect = "scratch_has_inspect=";
    scratch_has_inspect = zenith::concat(scratch_has_inspect, scene.materialHasProperty(scratch_material, "inspect"));
    println(scratch_has_inspect);
    std::string removed_inspect = "removed_inspect=";
    removed_inspect = zenith::concat(removed_inspect, scene.removeMaterialProperty(scratch_material, "inspect"));
    println(removed_inspect);
    std::string scratch_has_inspect_after = "scratch_has_inspect_after=";
    scratch_has_inspect_after = zenith::concat(scratch_has_inspect_after, scene.materialHasProperty(scratch_material, "inspect"));
    println(scratch_has_inspect_after);
    std::string cleared_props = "cleared_props=";
    cleared_props = zenith::concat(cleared_props, scene.clearMaterialProperties(scratch_material));
    println(cleared_props);
    std::string scratch_props_after_clear = "scratch_props_after_clear=";
    scratch_props_after_clear = zenith::concat(scratch_props_after_clear, scene.materialPropertyCount(scratch_material));
    println(scratch_props_after_clear);
    std::string roughness = "roughness=";
    roughness = zenith::concat(roughness, roughness_property.numberValue);
    println(roughness);
    std::string audio_min = "audio_min=";
    audio_min = zenith::concat(audio_min, scene.audioSource3D(scene.engineAudio).minDistance);
    println(audio_min);
    std::string audio_max = "audio_max=";
    audio_max = zenith::concat(audio_max, scene.audioSource3D(scene.engineAudio).maxDistance);
    println(audio_max);
    std::string listener_gain = "listener_gain=";
    listener_gain = zenith::concat(listener_gain, scene.audioListener3D(scene.listener).gain);
    println(listener_gain);
    std::string point_range = "point_range=";
    point_range = zenith::concat(point_range, scene.pointLight3D(scene.beaconLight).range);
    println(point_range);
    std::string ship_radius = "ship_radius=";
    ship_radius = zenith::concat(ship_radius, scene.sphereCollider3D(scene.ship).radius);
    println(ship_radius);
    std::string marker_depth = "marker_depth=";
    marker_depth = zenith::concat(marker_depth, scene.boxCollider3D(scene.marker).depth);
    println(marker_depth);
    std::string sun_dir_y = "sun_dir_y=";
    sun_dir_y = zenith::concat(sun_dir_y, scene.directionalLight3D(scene.sunLight).directionY);
    println(sun_dir_y);
    std::string overlap = "overlap3d=";
    overlap = zenith::concat(overlap, scene.overlaps3D(scene.ship, scene.marker));
    println(overlap);
    std::string glow = "glow=";
    glow = zenith::concat(glow, glow_property.boolValue);
    println(glow);
    std::string audio_playing = "audio_playing=";
    audio_playing = zenith::concat(audio_playing, scene.audioSource3D(scene.engineAudio).isPlaying);
    println(audio_playing);
    std::string move_speed = "move_speed=";
    move_speed = zenith::concat(move_speed, scene.character3D(scene.ship).moveSpeed);
    println(move_speed);
    std::string turn_speed = "turn_speed=";
    turn_speed = zenith::concat(turn_speed, scene.character3D(scene.ship).turnSpeed);
    println(turn_speed);
    std::string grounded = "grounded=";
    grounded = zenith::concat(grounded, scene.character3D(scene.ship).isGrounded);
    println(grounded);
    std::string shadows = "cast_shadows=";
    shadows = zenith::concat(shadows, scene.mesh3D(scene.ship).castShadows);
    println(shadows);
    scene.stopAudio(scene.engineAudio);
    std::string audio_stopped = "audio_stopped=";
    audio_stopped = zenith::concat(audio_stopped, scene.audioSource3D(scene.engineAudio).isPlaying);
    println(audio_stopped);
    std::string listener_enabled = "listener_enabled=";
    listener_enabled = zenith::concat(listener_enabled, scene.audioListener3D(scene.listener).enabled);
    println(listener_enabled);
    std::string contains = "contains_center_3d=";
    contains = zenith::concat(contains, scene.containsPoint3D(scene.ship, scene.transform3D(scene.ship).x, scene.transform3D(scene.ship).y, scene.transform3D(scene.ship).z));
    println(contains);
    zenith::game::RaycastHit3DResult pick = scene.raycast3D(3.3, 1.5, 2.0, 0.0, 0.0, -1.0, 20.0);
    std::string ray_hit = "ray_hit=";
    ray_hit = zenith::concat(ray_hit, pick.hit);
    println(ray_hit);
    std::string ray_name = "ray_name=";
    ray_name = zenith::concat(ray_name, scene.entityName(pick.entity));
    println(ray_name);
    std::string ray_distance = "ray_distance=";
    ray_distance = zenith::concat(ray_distance, pick.distance);
    println(ray_distance);
    std::string ray_point_z = "ray_point_z=";
    ray_point_z = zenith::concat(ray_point_z, pick.pointZ);
    println(ray_point_z);
    std::string ray_normal_z = "ray_normal_z=";
    ray_normal_z = zenith::concat(ray_normal_z, pick.normalZ);
    println(ray_normal_z);
    std::string sun_shadows = "sun_shadows=";
    sun_shadows = zenith::concat(sun_shadows, scene.directionalLight3D(scene.sunLight).castShadows);
    println(sun_shadows);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

