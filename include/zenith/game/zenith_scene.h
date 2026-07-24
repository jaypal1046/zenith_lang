#ifndef ZENITH_SCENE_H
#define ZENITH_SCENE_H

#include "zenith_physics.h"
#include "zenith_world.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace zenith {

class Canvas;

namespace game {

template <typename T>
class FieldRef {
public:
    FieldRef() = default;
    FieldRef(const FieldRef&) = default;
    FieldRef(FieldRef&&) = default;

    FieldRef(std::function<T()> getterFn, std::function<void(const T&)> setterFn)
        : getter(std::move(getterFn)), setter(std::move(setterFn)) {}

    T get() const {
        return getter ? getter() : T{};
    }

    void set(const T& value) const {
        if (setter) {
            setter(value);
        }
    }

    operator T() const {
        return get();
    }

    FieldRef& operator=(const T& value) {
        set(value);
        return *this;
    }

    FieldRef& operator=(const FieldRef& other) {
        if (!isBound()) {
            getter = other.getter;
            setter = other.setter;
        } else {
            set(other.get());
        }
        return *this;
    }

    FieldRef& operator=(FieldRef&& other) {
        if (!isBound()) {
            getter = std::move(other.getter);
            setter = std::move(other.setter);
        } else {
            set(other.get());
        }
        return *this;
    }

    FieldRef& operator+=(const T& value) {
        set(get() + value);
        return *this;
    }

    FieldRef& operator-=(const T& value) {
        set(get() - value);
        return *this;
    }

    FieldRef& operator*=(const T& value) {
        set(get() * value);
        return *this;
    }

    FieldRef& operator/=(const T& value) {
        set(get() / value);
        return *this;
    }

private:
    bool isBound() const {
        return static_cast<bool>(getter) || static_cast<bool>(setter);
    }

    std::function<T()> getter;
    std::function<void(const T&)> setter;
};

template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const FieldRef<T>& ref) {
    stream << ref.get();
    return stream;
}

struct Transform2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> rotation;
    FieldRef<float> scaleX;
    FieldRef<float> scaleY;
};

struct Body2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> vx;
    FieldRef<float> vy;
    FieldRef<float> mass;
    FieldRef<float> gravityScale;
    FieldRef<float> friction;
    FieldRef<float> restitution;
};

struct BoxCollider2DView {
    FieldRef<float> offsetX;
    FieldRef<float> offsetY;
    FieldRef<float> width;
    FieldRef<float> height;
    FieldRef<bool> isTrigger;
    FieldRef<bool> enabled;
};

struct CircleCollider2DView {
    FieldRef<float> offsetX;
    FieldRef<float> offsetY;
    FieldRef<float> radius;
    FieldRef<bool> isTrigger;
    FieldRef<bool> enabled;
};

struct CapsuleCollider2DView {
    FieldRef<float> offsetX;
    FieldRef<float> offsetY;
    FieldRef<float> height;
    FieldRef<float> radius;
    FieldRef<bool> isTrigger;
    FieldRef<bool> enabled;
};

struct Camera2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> zoom;
    FieldRef<bool> primary;
    FieldRef<float> viewportX;
    FieldRef<float> viewportY;
    FieldRef<float> viewportWidth;
    FieldRef<float> viewportHeight;
};

struct AudioListener2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> gain;
    FieldRef<bool> primary;
    FieldRef<bool> enabled;
};

struct TextureHandleView {
    std::string path;
    int id = 0;
    int refCount = 0;
    bool loaded = false;
    int width = 0;
    int height = 0;
    int channels = 0;
    int gpuId = 0;
};

struct AudioHandleView {
    std::string path;
    int id = 0;
    int refCount = 0;
    bool loaded = false;
    float duration = 0.0f;
    bool spatial = false;
};

struct MeshHandleView {
    std::string path;
    int id = 0;
    int refCount = 0;
    bool loaded = false;
    int vertexCount = 0;
    int triangleCount = 0;
    int vbo = 0;
    int ebo = 0;
};

struct ShaderHandleView {
    std::string path;
    int id = 0;
    int refCount = 0;
    bool loaded = false;
    int programId = 0;
};

struct MaterialHandleView {
    std::string path;
    int id = 0;
    int refCount = 0;
    bool loaded = false;
    std::string shaderPath;
    int propertyCount = 0;
};

struct Sprite2DView {
    FieldRef<float> width;
    FieldRef<float> height;
    FieldRef<float> anchorX;
    FieldRef<float> anchorY;
    FieldRef<std::string> color;
    FieldRef<std::string> texturePath;
    FieldRef<TextureHandleView> texture;
    FieldRef<int> sortOrder;
    FieldRef<bool> visible;
};

struct Tilemap2DView {
    FieldRef<int> columns;
    FieldRef<int> rows;
    FieldRef<float> tileWidth;
    FieldRef<float> tileHeight;
    FieldRef<float> anchorX;
    FieldRef<float> anchorY;
    FieldRef<int> sortOrder;
    FieldRef<bool> visible;
};

struct Character2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> vx;
    FieldRef<float> vy;
    FieldRef<float> moveSpeed;
    FieldRef<float> jumpForce;
    FieldRef<bool> isGrounded;
    FieldRef<bool> facingRight;
    FieldRef<std::string> texturePath;
    FieldRef<TextureHandleView> texture;
    FieldRef<std::string> color;
};

struct AudioSource2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<std::string> clipPath;
    FieldRef<AudioHandleView> clip;
    FieldRef<float> volume;
    FieldRef<float> pitch;
    FieldRef<bool> loop;
    FieldRef<bool> playOnAwake;
    FieldRef<bool> isPlaying;
};

struct Transform3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> rotationX;
    FieldRef<float> rotationY;
    FieldRef<float> rotationZ;
    FieldRef<float> scaleX;
    FieldRef<float> scaleY;
    FieldRef<float> scaleZ;
};

struct Body3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> vx;
    FieldRef<float> vy;
    FieldRef<float> vz;
    FieldRef<float> mass;
    FieldRef<float> gravityScale;
    FieldRef<float> friction;
    FieldRef<float> restitution;
    FieldRef<bool> useGravity;
    FieldRef<bool> isGrounded;
};

struct BoxCollider3DView {
    FieldRef<float> offsetX;
    FieldRef<float> offsetY;
    FieldRef<float> offsetZ;
    FieldRef<float> width;
    FieldRef<float> height;
    FieldRef<float> depth;
    FieldRef<bool> isTrigger;
    FieldRef<bool> enabled;
};

struct SphereCollider3DView {
    FieldRef<float> offsetX;
    FieldRef<float> offsetY;
    FieldRef<float> offsetZ;
    FieldRef<float> radius;
    FieldRef<bool> isTrigger;
    FieldRef<bool> enabled;
};

struct Camera3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> rotationX;
    FieldRef<float> rotationY;
    FieldRef<float> rotationZ;
    FieldRef<float> fov;
    FieldRef<float> nearClip;
    FieldRef<float> farClip;
    FieldRef<bool> primary;
    FieldRef<float> viewportX;
    FieldRef<float> viewportY;
    FieldRef<float> viewportWidth;
    FieldRef<float> viewportHeight;
};

struct AudioListener3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> gain;
    FieldRef<bool> primary;
    FieldRef<bool> enabled;
};

struct Mesh3DView {
    FieldRef<std::string> meshPath;
    FieldRef<std::string> shaderPath;
    FieldRef<std::string> materialPath;
    FieldRef<MeshHandleView> mesh;
    FieldRef<ShaderHandleView> shader;
    FieldRef<MaterialHandleView> material;
    FieldRef<bool> visible;
    FieldRef<bool> castShadows;
};

struct MaterialPropertyView {
    FieldRef<bool> exists;
    FieldRef<std::string> name;
    FieldRef<std::string> label;
    FieldRef<std::string> kind;
    FieldRef<std::string> options;
    FieldRef<std::string> callback;
    FieldRef<std::string> stringValue;
    FieldRef<float> numberValue;
    FieldRef<bool> boolValue;
    FieldRef<int> triggerCount;
};

struct MaterialPropertyOptionView {
    FieldRef<bool> exists;
    FieldRef<std::string> label;
    FieldRef<std::string> value;
};

struct Character3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> vx;
    FieldRef<float> vy;
    FieldRef<float> vz;
    FieldRef<float> moveSpeed;
    FieldRef<float> turnSpeed;
    FieldRef<float> jumpSpeed;
    FieldRef<float> groundAcceleration;
    FieldRef<float> airAcceleration;
    FieldRef<float> groundFriction;
    FieldRef<float> airControl;
    FieldRef<float> groundSnapDistance;
    FieldRef<float> maxSlopeAngle;
    FieldRef<float> moveInputX;
    FieldRef<float> moveInputY;
    FieldRef<float> moveInputZ;
    FieldRef<bool> isGrounded;
    FieldRef<bool> useGravity;
    FieldRef<std::string> meshPath;
    FieldRef<std::string> shaderPath;
    FieldRef<std::string> materialPath;
    FieldRef<MeshHandleView> mesh;
    FieldRef<ShaderHandleView> shader;
    FieldRef<MaterialHandleView> material;
};

struct AudioSource3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<std::string> clipPath;
    FieldRef<AudioHandleView> clip;
    FieldRef<float> volume;
    FieldRef<float> pitch;
    FieldRef<bool> loop;
    FieldRef<bool> playOnAwake;
    FieldRef<bool> isPlaying;
    FieldRef<float> minDistance;
    FieldRef<float> maxDistance;
};

struct PointLight3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> intensity;
    FieldRef<float> range;
    FieldRef<std::string> color;
    FieldRef<bool> enabled;
};

struct DirectionalLight3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<float> directionX;
    FieldRef<float> directionY;
    FieldRef<float> directionZ;
    FieldRef<float> intensity;
    FieldRef<std::string> color;
    FieldRef<bool> castShadows;
    FieldRef<bool> enabled;
};

class Scene {
public:
    using PrefabBuilder = std::function<void(Scene&, EntityId)>;
    using StreamBuilder = std::function<void(Scene&, const std::string&)>;

    World world;
    physics::PhysicsWorld physics;
    std::string name = "Scene";
    std::string clearColor = "black";
    float fixedDeltaTime = 1.0f / 60.0f;
    float maxFrameDelta = 0.25f;
    int maxFixedStepsPerFrame = 8;
    bool autoSimulatePhysics = true;
    bool autoRenderWorld2D = true;
    bool drawEntityNames = true;
    bool debugDrawGrid2D = false;
    bool debugDrawColliders2D = false;
    bool debugDrawTransforms2D = false;
    bool debugDrawCameraBounds2D = false;
    bool debugDrawRuntimeStats = false;
    float debugGridCellWidth = 1.0f;
    float debugGridCellHeight = 1.0f;
    std::string debugOverlayColor = "cyan";
    bool debugOverlayEnabled = true;
    bool minimalInspectorEnabled = false;
    bool paused = false;

    virtual ~Scene() = default;

    void load() {
        if (loaded) {
            return;
        }
        loaded = true;
        onLoad();
    }

    void updateFrame(float frameDt) {
        load();

        if (frameDt < 0.0f) {
            frameDt = 0.0f;
        }
        if (frameDt > maxFrameDelta) {
            frameDt = maxFrameDelta;
        }

        ++frameCount;
        lastFrameDelta = frameDt;
        lastFixedStepsThisFrame = 0;

        onFrame(frameDt);

        if (paused || fixedDeltaTime <= 0.0f) {
            return;
        }

        accumulator += frameDt;

        while (accumulator + 0.000001f >= fixedDeltaTime && lastFixedStepsThisFrame < maxFixedStepsPerFrame) {
            simulateFixedStep(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
            ++lastFixedStepsThisFrame;
            ++fixedStepCount;
        }

        if (lastFixedStepsThisFrame == maxFixedStepsPerFrame && accumulator >= fixedDeltaTime) {
            accumulator = std::fmod(accumulator, fixedDeltaTime);
            ++droppedStepFrames;
        }
    }

    void simulateFixedStep(float dt) {
        load();
        onFixedUpdate(dt);

        if (autoSimulatePhysics) {
            rebuildPhysicsBindings();
            applyCharacterControllerInputs3D(dt);
            physics.step(dt);
            resolveCharacterControllers3D();
            syncFromPhysics();
        }

        onPostPhysics(dt);
    }

    void render(Canvas& canvas) {
        load();
        onDraw(canvas, interpolationAlpha());
    }

    virtual void triggerCallback(std::string name, std::string val = "") {}
    virtual void triggerEntityCallback(std::string name, EntityId entity) {
        (void)name;
        (void)entity;
    }

    void setPaused(bool value) {
        paused = value;
    }

    bool isLoaded() const {
        return loaded;
    }

    float interpolationAlpha() const {
        if (fixedDeltaTime <= 0.0f) {
            return 0.0f;
        }

        float alpha = accumulator / fixedDeltaTime;
        if (alpha < 0.0f) {
            return 0.0f;
        }
        if (alpha > 1.0f) {
            return 1.0f;
        }
        return alpha;
    }

    uint64_t totalFrames() const {
        return frameCount;
    }

    uint64_t totalFixedSteps() const {
        return fixedStepCount;
    }

    uint64_t framesWithDroppedSteps() const {
        return droppedStepFrames;
    }

    int lastSubstepCount() const {
        return lastFixedStepsThisFrame;
    }

    float accumulatedTime() const {
        return accumulator;
    }

    float frameDelta() const {
        return lastFrameDelta;
    }

    void inspectEntity(EntityId entity) {
        if (world.isAlive(entity)) {
            inspectorEntityTarget = entity;
            inspectorMaterialTarget.clear();
            return;
        }
        inspectorEntityTarget = EntityId::invalid();
    }

    EntityId inspectedEntity() const {
        return world.isAlive(inspectorEntityTarget) ? inspectorEntityTarget : EntityId::invalid();
    }

    void inspectMaterial(std::string materialPath) {
        inspectorMaterialTarget = materialPath;
    }

    std::string inspectedMaterialPath() const {
        return inspectorMaterialTarget;
    }

    void clearInspectorTarget() {
        inspectorEntityTarget = EntityId::invalid();
        inspectorMaterialTarget.clear();
    }

    bool selectNextInspectorEntity() {
        const std::vector<EntityId> entities = world.aliveEntities();
        if (entities.empty()) {
            inspectorEntityTarget = EntityId::invalid();
            return false;
        }

        if (!world.isAlive(inspectorEntityTarget)) {
            inspectEntity(entities.front());
            return true;
        }

        auto it = std::find(entities.begin(), entities.end(), inspectorEntityTarget);
        if (it == entities.end() || std::next(it) == entities.end()) {
            inspectEntity(entities.front());
            return true;
        }

        inspectEntity(*std::next(it));
        return true;
    }

    bool selectPreviousInspectorEntity() {
        const std::vector<EntityId> entities = world.aliveEntities();
        if (entities.empty()) {
            inspectorEntityTarget = EntityId::invalid();
            return false;
        }

        if (!world.isAlive(inspectorEntityTarget)) {
            inspectEntity(entities.back());
            return true;
        }

        auto it = std::find(entities.begin(), entities.end(), inspectorEntityTarget);
        if (it == entities.end() || it == entities.begin()) {
            inspectEntity(entities.back());
            return true;
        }

        inspectEntity(*std::prev(it));
        return true;
    }

    void registerPrefab(std::string prefabName, PrefabBuilder builder) {
        prefabs[std::move(prefabName)] = std::move(builder);
    }

    void registerPrefabCallback(std::string prefabName, std::string callbackName) {
        registerPrefab(std::move(prefabName), [callback = std::move(callbackName)](Scene& scene, EntityId entity) {
            scene.triggerEntityCallback(callback, entity);
        });
    }

    bool hasPrefab(const std::string& prefabName) const {
        return prefabs.find(prefabName) != prefabs.end();
    }

    EntityId instantiate(const std::string& prefabName, const std::string& entityName = "") {
        auto it = prefabs.find(prefabName);
        if (it == prefabs.end()) {
            throw std::invalid_argument("Zenith Scene prefab not found: " + prefabName);
        }

        EntityId entity = entityName.empty() ? world.createEntity(prefabName) : world.createEntity(entityName);
        it->second(*this, entity);
        return entity;
    }

    EntityId instantiatePrefab(const std::string& prefabName, const std::string& entityName = "") {
        return instantiate(prefabName, entityName);
    }

    EntityId instantiateArchetype(EntityId source, const std::string& entityName = "") {
        return world.cloneEntityHierarchy(source, entityName);
    }

    bool setParent(EntityId child, EntityId parent) {
        if (!world.setParent(child, parent)) {
            return false;
        }
        const std::string owner = sceneStreamOwner(parent);
        if (!owner.empty()) {
            adoptSceneStreamBranchIfUnowned(child, owner);
        }
        return true;
    }

    bool clearParent(EntityId child) {
        return world.clearParent(child);
    }

    EntityId parentOf(EntityId child) const {
        return world.parentOf(child);
    }

    int childCount(EntityId parent) const {
        return static_cast<int>(world.childrenOf(parent).size());
    }

    EntityId childAt(EntityId parent, int index) const {
        const std::vector<EntityId> children = world.childrenOf(parent);
        if (index < 0 || static_cast<size_t>(index) >= children.size()) {
            return EntityId::invalid();
        }
        return children[static_cast<size_t>(index)];
    }

    void registerSceneStream(std::string streamName, StreamBuilder builder) {
        sceneStreams[std::move(streamName)] = std::move(builder);
    }

    void registerSceneStreamCallback(std::string streamName, std::string callbackName) {
        registerSceneStream(std::move(streamName), [callback = std::move(callbackName)](Scene& scene, const std::string& instanceName) {
            scene.triggerCallback(callback, instanceName);
        });
    }

    bool hasSceneStream(const std::string& streamName) const {
        return sceneStreams.find(streamName) != sceneStreams.end();
    }

    bool loadSceneStream(const std::string& streamName, const std::string& instanceName = "") {
        auto it = sceneStreams.find(streamName);
        if (it == sceneStreams.end()) {
            return false;
        }

        const std::string key = instanceName.empty() ? streamName : instanceName;
        if (loadedStreams.find(key) != loadedStreams.end()) {
            return false;
        }

        std::vector<EntityId> before = world.aliveEntities();
        it->second(*this, key);

        StreamInstance instance;
        instance.source = streamName;
        instance.entities = filterSceneStreamOwnedEntities(key, collectNewEntities(before));
        markSceneStreamOwnedEntities(key, instance.entities);
        loadedStreams[key] = std::move(instance);
        return true;
    }

    bool unloadSceneStream(const std::string& instanceName) {
        auto it = loadedStreams.find(instanceName);
        if (it == loadedStreams.end()) {
            return false;
        }

        std::vector<EntityId> entities = collectSceneStreamTrackedEntities(instanceName, it->second);
        std::sort(entities.begin(), entities.end(), [this](EntityId lhs, EntityId rhs) {
            return hierarchyDepth(lhs) > hierarchyDepth(rhs);
        });

        for (EntityId entity : entities) {
            clearSceneStreamOwner(entity);
            if (world.isAlive(entity)) {
                world.destroyEntity(entity);
            }
        }

        for (EntityId entity : it->second.entities) {
            clearSceneStreamOwner(entity);
        }

        loadedStreams.erase(it);
        return true;
    }

    bool isSceneStreamLoaded(const std::string& instanceName) const {
        return loadedStreams.find(instanceName) != loadedStreams.end();
    }

    int sceneStreamEntityCount(const std::string& instanceName) const {
        auto it = loadedStreams.find(instanceName);
        if (it == loadedStreams.end()) {
            return 0;
        }
        return static_cast<int>(collectSceneStreamTrackedEntities(instanceName, it->second).size());
    }

    void setEntityLayer(EntityId entity, int layerBits) {
        if (!world.isAlive(entity)) {
            return;
        }

        LayerMaskComponent* layerMask = world.getLayerMask(entity);
        if (layerMask == nullptr) {
            layerMask = &world.addLayerMask(entity);
        }
        layerMask->layers = static_cast<uint32_t>(layerBits == 0 ? 1 : layerBits);
    }

    int entityLayer(EntityId entity) const {
        return static_cast<int>(world.entityLayerBits(entity));
    }

    void setEntityMask(EntityId entity, int maskBits) {
        if (!world.isAlive(entity)) {
            return;
        }

        LayerMaskComponent* layerMask = world.getLayerMask(entity);
        if (layerMask == nullptr) {
            layerMask = &world.addLayerMask(entity);
        }
        layerMask->mask = static_cast<uint32_t>(maskBits);
    }

    int entityMask(EntityId entity) const {
        return static_cast<int>(world.entityMaskBits(entity));
    }

    void setEntityLayerMask(EntityId entity, int layerBits, int maskBits) {
        if (!world.isAlive(entity)) {
            return;
        }

        LayerMaskComponent& layerMask = world.addLayerMask(entity);
        layerMask.layers = static_cast<uint32_t>(layerBits == 0 ? 1 : layerBits);
        layerMask.mask = static_cast<uint32_t>(maskBits);
    }

    bool canEntitiesInteract(EntityId first, EntityId second) const {
        return world.canInteract(first, second);
    }

    Transform2DView transform2D(EntityId entity) {
        Transform2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
                if (physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    body->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
                if (physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    body->position.y = value;
                }
            });

        view.rotation = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->rotation;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->rotation = value;
                }
            });

        view.scaleX = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->scale.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->scale.x = value;
                }
            });

        view.scaleY = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->scale.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->scale.y = value;
                }
            });

        return view;
    }

    Body2DView body2D(EntityId entity) {
        Body2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.x;
                }
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->position.x = value;
                }
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.y;
                }
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->position.y = value;
                }
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.vx = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->velocity.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->velocity.x = value;
                }
            });

        view.vy = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->velocity.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->velocity.y = value;
                }
            });

        view.mass = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->mass;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->mass = value <= 0.0f ? 1.0f : value;
                }
            });

        view.gravityScale = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->gravityScale;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->gravityScale = value;
                }
            });

        view.friction = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->friction;
                }
                return 0.2f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->friction = value;
                }
            });

        view.restitution = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->restitution;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->restitution = value;
                }
            });

        return view;
    }

    BoxCollider2DView boxCollider2D(EntityId entity) {
        BoxCollider2DView view;

        view.offsetX = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->offset.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->offset.x = value;
                }
            });

        view.offsetY = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->offset.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->offset.y = value;
                }
            });

        view.width = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->size.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->size.x = value <= 0.0f ? 0.01f : value;
                }
            });

        view.height = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->size.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->size.y = value <= 0.0f ? 0.01f : value;
                }
            });

        view.isTrigger = FieldRef<bool>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->isTrigger;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->isTrigger = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const physics::BoxCollider2D* collider = world.getBoxCollider2D(entity)) {
                    return collider->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::BoxCollider2D* collider = ensureBoxCollider2DViewComponent(entity)) {
                    collider->enabled = value;
                }
            });

        return view;
    }

    CircleCollider2DView circleCollider2D(EntityId entity) {
        CircleCollider2DView view;

        view.offsetX = FieldRef<float>(
            [this, entity]() {
                if (const physics::CircleCollider2D* collider = world.getCircleCollider2D(entity)) {
                    return collider->offset.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::CircleCollider2D* collider = ensureCircleCollider2DViewComponent(entity)) {
                    collider->offset.x = value;
                }
            });

        view.offsetY = FieldRef<float>(
            [this, entity]() {
                if (const physics::CircleCollider2D* collider = world.getCircleCollider2D(entity)) {
                    return collider->offset.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::CircleCollider2D* collider = ensureCircleCollider2DViewComponent(entity)) {
                    collider->offset.y = value;
                }
            });

        view.radius = FieldRef<float>(
            [this, entity]() {
                if (const physics::CircleCollider2D* collider = world.getCircleCollider2D(entity)) {
                    return collider->radius;
                }
                return 0.5f;
            },
            [this, entity](const float& value) {
                if (physics::CircleCollider2D* collider = ensureCircleCollider2DViewComponent(entity)) {
                    collider->radius = value <= 0.0f ? 0.01f : value;
                }
            });

        view.isTrigger = FieldRef<bool>(
            [this, entity]() {
                if (const physics::CircleCollider2D* collider = world.getCircleCollider2D(entity)) {
                    return collider->isTrigger;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::CircleCollider2D* collider = ensureCircleCollider2DViewComponent(entity)) {
                    collider->isTrigger = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const physics::CircleCollider2D* collider = world.getCircleCollider2D(entity)) {
                    return collider->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::CircleCollider2D* collider = ensureCircleCollider2DViewComponent(entity)) {
                    collider->enabled = value;
                }
            });

        return view;
    }

    CapsuleCollider2DView capsuleCollider2D(EntityId entity) {
        CapsuleCollider2DView view;

        view.offsetX = FieldRef<float>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->offset.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    collider->offset.x = value;
                }
            });

        view.offsetY = FieldRef<float>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->offset.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    collider->offset.y = value;
                }
            });

        view.height = FieldRef<float>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->height;
                }
                return 2.0f;
            },
            [this, entity](const float& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    const float radius = collider->radius <= 0.0f ? 0.01f : collider->radius;
                    collider->height = std::max(value, radius * 2.0f);
                }
            });

        view.radius = FieldRef<float>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->radius;
                }
                return 0.5f;
            },
            [this, entity](const float& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    collider->radius = value <= 0.0f ? 0.01f : value;
                    collider->height = std::max(collider->height, collider->radius * 2.0f);
                }
            });

        view.isTrigger = FieldRef<bool>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->isTrigger;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    collider->isTrigger = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity)) {
                    return collider->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::CapsuleCollider2D* collider = ensureCapsuleCollider2DViewComponent(entity)) {
                    collider->enabled = value;
                }
            });

        return view;
    }

    Camera2DView camera2D(EntityId entity) {
        Camera2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.zoom = FieldRef<float>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->zoom;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Camera2DComponent* camera = ensureCamera2DViewComponent(entity)) {
                    camera->zoom = value;
                }
            });

        view.primary = FieldRef<bool>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->primary;
                }
                return false;
            },
            [this, entity](const bool& value) {
                Camera2DComponent* camera = ensureCamera2DViewComponent(entity);
                if (camera == nullptr) {
                    return;
                }
                if (value) {
                    std::optional<EntityId> current = world.primaryCamera2D();
                    if (current.has_value() && current.value() != entity) {
                        if (Camera2DComponent* currentCamera = world.getCamera2D(current.value())) {
                            currentCamera->primary = false;
                        }
                    }
                }
                camera->primary = value;
            });

        view.viewportX = FieldRef<float>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->viewportOrigin.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Camera2DComponent* camera = ensureCamera2DViewComponent(entity)) {
                    camera->viewportOrigin.x = value;
                }
            });

        view.viewportY = FieldRef<float>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->viewportOrigin.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Camera2DComponent* camera = ensureCamera2DViewComponent(entity)) {
                    camera->viewportOrigin.y = value;
                }
            });

        view.viewportWidth = FieldRef<float>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->viewportSize.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Camera2DComponent* camera = ensureCamera2DViewComponent(entity)) {
                    camera->viewportSize.x = value;
                }
            });

        view.viewportHeight = FieldRef<float>(
            [this, entity]() {
                if (const Camera2DComponent* camera = world.getCamera2D(entity)) {
                    return camera->viewportSize.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Camera2DComponent* camera = ensureCamera2DViewComponent(entity)) {
                    camera->viewportSize.y = value;
                }
            });

        return view;
    }

    AudioListener2DView audioListener2D(EntityId entity) {
        AudioListener2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.gain = FieldRef<float>(
            [this, entity]() {
                if (const AudioListener2DComponent* listener = world.getAudioListener2D(entity)) {
                    return listener->gain;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioListener2DComponent* listener = ensureAudioListener2DViewComponent(entity)) {
                    listener->gain = value;
                }
            });

        view.primary = FieldRef<bool>(
            [this, entity]() {
                if (const AudioListener2DComponent* listener = world.getAudioListener2D(entity)) {
                    return listener->primary;
                }
                return false;
            },
            [this, entity](const bool& value) {
                AudioListener2DComponent* listener = ensureAudioListener2DViewComponent(entity);
                if (listener == nullptr) {
                    return;
                }
                if (value) {
                    std::optional<EntityId> current = world.primaryAudioListener2D();
                    if (current.has_value() && current.value() != entity) {
                        if (AudioListener2DComponent* currentListener = world.getAudioListener2D(current.value())) {
                            currentListener->primary = false;
                        }
                    }
                }
                listener->primary = value;
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const AudioListener2DComponent* listener = world.getAudioListener2D(entity)) {
                    return listener->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (AudioListener2DComponent* listener = ensureAudioListener2DViewComponent(entity)) {
                    listener->enabled = value;
                }
            });

        return view;
    }

    Sprite2DView sprite2D(EntityId entity) {
        Sprite2DView view;

        view.width = FieldRef<float>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->size.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->size.x = value;
                }
            });

        view.height = FieldRef<float>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->size.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->size.y = value;
                }
            });

        view.anchorX = FieldRef<float>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->anchor.x;
                }
                return 0.5f;
            },
            [this, entity](const float& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->anchor.x = value;
                }
            });

        view.anchorY = FieldRef<float>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->anchor.y;
                }
                return 0.5f;
            },
            [this, entity](const float& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->anchor.y = value;
                }
            });

        view.color = FieldRef<std::string>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->tintColor;
                }
                return std::string("white");
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->tintColor = value;
                }
            });

        view.texturePath = FieldRef<std::string>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return assetPathFromBinding(sprite->textureHandle, sprite->texture);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    assignSpriteTextureBinding(*sprite, value);
                }
            });

        view.texture = FieldRef<TextureHandleView>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return makeTextureHandleView(sprite->textureHandle, sprite->texture);
                }
                return TextureHandleView{};
            },
            [this, entity](const TextureHandleView& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    assignSpriteTextureBinding(*sprite, value);
                }
            });

        view.sortOrder = FieldRef<int>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->sortOrder;
                }
                return 0;
            },
            [this, entity](const int& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->sortOrder = value;
                }
            });

        view.visible = FieldRef<bool>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->visible;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->visible = value;
                }
            });

        return view;
    }

    Tilemap2DView tilemap2D(EntityId entity) {
        Tilemap2DView view;

        view.columns = FieldRef<int>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->columns;
                }
                return 1;
            },
            [this, entity](const int& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->resize(value, tilemap->rows, 0);
                }
            });

        view.rows = FieldRef<int>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->rows;
                }
                return 1;
            },
            [this, entity](const int& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->resize(tilemap->columns, value, 0);
                }
            });

        view.tileWidth = FieldRef<float>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->tileSize.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->tileSize.x = value <= 0.0f ? 1.0f : value;
                }
            });

        view.tileHeight = FieldRef<float>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->tileSize.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->tileSize.y = value <= 0.0f ? 1.0f : value;
                }
            });

        view.anchorX = FieldRef<float>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->anchor.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->anchor.x = value;
                }
            });

        view.anchorY = FieldRef<float>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->anchor.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->anchor.y = value;
                }
            });

        view.sortOrder = FieldRef<int>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->sortOrder;
                }
                return 0;
            },
            [this, entity](const int& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->sortOrder = value;
                }
            });

        view.visible = FieldRef<bool>(
            [this, entity]() {
                if (const Tilemap2DComponent* tilemap = world.getTilemap2D(entity)) {
                    return tilemap->visible;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (Tilemap2DComponent* tilemap = ensureTilemap2DViewComponent(entity)) {
                    tilemap->visible = value;
                }
            });

        return view;
    }

    Character2DView character2D(EntityId entity) {
        Character2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.x;
                }
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->position.x = value;
                }
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.y;
                }
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->position.y = value;
                }
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.vx = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->velocity.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->velocity.x = value;
                }
            });

        view.vy = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->velocity.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody2D* body = ensureBody2DViewComponent(entity)) {
                    body->velocity.y = value;
                }
            });

        view.moveSpeed = FieldRef<float>(
            [this, entity]() {
                if (const Character2DComponent* character = world.getCharacter2D(entity)) {
                    return character->moveSpeed;
                }
                return 10.0f;
            },
            [this, entity](const float& value) {
                if (Character2DComponent* character = ensureCharacter2DViewComponent(entity)) {
                    character->moveSpeed = value;
                }
            });

        view.jumpForce = FieldRef<float>(
            [this, entity]() {
                if (const Character2DComponent* character = world.getCharacter2D(entity)) {
                    return character->jumpForce;
                }
                return 12.0f;
            },
            [this, entity](const float& value) {
                if (Character2DComponent* character = ensureCharacter2DViewComponent(entity)) {
                    character->jumpForce = value;
                }
            });

        view.isGrounded = FieldRef<bool>(
            [this, entity]() {
                if (const Character2DComponent* character = world.getCharacter2D(entity)) {
                    return character->isGrounded;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (Character2DComponent* character = ensureCharacter2DViewComponent(entity)) {
                    character->isGrounded = value;
                }
            });

        view.facingRight = FieldRef<bool>(
            [this, entity]() {
                if (const Character2DComponent* character = world.getCharacter2D(entity)) {
                    return character->facingRight;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (Character2DComponent* character = ensureCharacter2DViewComponent(entity)) {
                    character->facingRight = value;
                }
            });

        view.texturePath = FieldRef<std::string>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return assetPathFromBinding(sprite->textureHandle, sprite->texture);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    assignSpriteTextureBinding(*sprite, value);
                }
            });

        view.texture = FieldRef<TextureHandleView>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return makeTextureHandleView(sprite->textureHandle, sprite->texture);
                }
                return TextureHandleView{};
            },
            [this, entity](const TextureHandleView& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    assignSpriteTextureBinding(*sprite, value);
                }
            });

        view.color = FieldRef<std::string>(
            [this, entity]() {
                if (const SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                    return sprite->tintColor;
                }
                return std::string("white");
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->tintColor = value;
                }
            });

        return view;
    }

    AudioSource2DView audioSource2D(EntityId entity) {
        AudioSource2DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.x;
                }
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.x = value;
                }
                if (physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    body->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform2D* transform = world.getTransform2D(entity)) {
                    return transform->position.y;
                }
                if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    return body->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform2D* transform = ensureTransform2DViewComponent(entity)) {
                    transform->position.y = value;
                }
                if (physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                    body->position.y = value;
                }
            });

        view.clipPath = FieldRef<std::string>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return assetPathFromBinding(audio->clipHandle, audio->clip);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    assignAudioSource2DBinding(*audio, value);
                }
            });

        view.clip = FieldRef<AudioHandleView>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return makeAudioHandleView(audio->clipHandle, audio->clip);
                }
                return AudioHandleView{};
            },
            [this, entity](const AudioHandleView& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    assignAudioSource2DBinding(*audio, value);
                }
            });

        view.volume = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return audio->volume;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->volume = value;
                }
            });

        view.pitch = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return audio->pitch;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->pitch = value;
                }
            });

        view.loop = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return audio->loop;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->loop = value;
                }
            });

        view.playOnAwake = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return audio->playOnAwake;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->playOnAwake = value;
                }
            });

        view.isPlaying = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
                    return audio->isPlaying;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->isPlaying = value;
                }
            });

        return view;
    }

    void attachBoxCollider2D(EntityId entity, float width, float height, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::BoxCollider2D* collider = world.getBoxCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addBoxCollider2D(entity);
        }
        collider->size.x = width <= 0.0f ? 0.01f : width;
        collider->size.y = height <= 0.0f ? 0.01f : height;
        collider->isTrigger = isTrigger;
    }

    void attachCircleCollider2D(EntityId entity, float radius, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::CircleCollider2D* collider = world.getCircleCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addCircleCollider2D(entity);
        }
        collider->radius = radius <= 0.0f ? 0.01f : radius;
        collider->isTrigger = isTrigger;
    }

    void attachCapsuleCollider2D(EntityId entity, float height, float radius, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addCapsuleCollider2D(entity);
        }
        collider->radius = radius <= 0.0f ? 0.01f : radius;
        collider->height = std::max(height, collider->radius * 2.0f);
        collider->isTrigger = isTrigger;
    }

    bool overlaps2D(EntityId first, EntityId second) const {
        return world.overlaps2D(first, second);
    }

    bool destroyEntityHierarchy(EntityId entity) {
        return world.destroyEntityHierarchy(entity);
    }

    bool containsPoint2D(EntityId entity, float x, float y) const {
        return world.containsPoint2D(entity, physics::Vec2(x, y));
    }

    RaycastHit2DResult raycast2D(
        float originX,
        float originY,
        float directionX,
        float directionY,
        float maxDistance
    ) const {
        return world.raycast2D(originX, originY, directionX, directionY, maxDistance);
    }

    RaycastHit2DResult raycast2DMask(
        float originX,
        float originY,
        float directionX,
        float directionY,
        float maxDistance,
        int layerMask
    ) const {
        return world.raycast2D(originX, originY, directionX, directionY, maxDistance, static_cast<uint32_t>(layerMask));
    }

    void attachBoxCollider3D(EntityId entity, float width, float height, float depth, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::BoxCollider3D* collider = world.getBoxCollider3D(entity);
        if (collider == nullptr) {
            collider = &world.addBoxCollider3D(entity);
        }
        collider->size.x = width <= 0.0f ? 0.01f : width;
        collider->size.y = height <= 0.0f ? 0.01f : height;
        collider->size.z = depth <= 0.0f ? 0.01f : depth;
        collider->isTrigger = isTrigger;
    }

    void attachSphereCollider3D(EntityId entity, float radius, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::SphereCollider3D* collider = world.getSphereCollider3D(entity);
        if (collider == nullptr) {
            collider = &world.addSphereCollider3D(entity);
        }
        collider->radius = radius <= 0.0f ? 0.01f : radius;
        collider->isTrigger = isTrigger;
    }

    void attachCapsuleCollider3D(EntityId entity, float height, float radius, bool isTrigger) {
        if (!world.isAlive(entity)) {
            return;
        }
        physics::CapsuleCollider3D* collider = world.getCapsuleCollider3D(entity);
        if (collider == nullptr) {
            collider = &world.addCapsuleCollider3D(entity);
        }
        collider->radius = radius <= 0.0f ? 0.01f : radius;
        collider->height = std::max(height, collider->radius * 2.0f);
        collider->isTrigger = isTrigger;
    }

    void setCharacterMove3D(EntityId entity, float x, float y, float z) {
        if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
            character->moveInput = physics::Vec3(x, y, z);
        }
    }

    void jumpCharacter3D(EntityId entity) {
        if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
            character->jumpQueued = true;
        }
    }

    EntityId instantiatePrefab2D(EntityId source, float posX, float posY, std::string entityName = "") {
        return world.instantiatePrefab2D(source, posX, posY, entityName);
    }

    EntityId instantiatePrefab3D(EntityId source, float posX, float posY, float posZ, std::string entityName = "") {
        return world.instantiatePrefab3D(source, posX, posY, posZ, entityName);
    }

    bool overlaps3D(EntityId first, EntityId second) const {
        return world.overlaps3D(first, second);
    }

    bool containsPoint3D(EntityId entity, float x, float y, float z) const {
        return world.containsPoint3D(entity, physics::Vec3(x, y, z));
    }

    RaycastHit3DResult raycast3D(
        float originX,
        float originY,
        float originZ,
        float directionX,
        float directionY,
        float directionZ,
        float maxDistance
    ) const {
        return world.raycast3D(
            originX,
            originY,
            originZ,
            directionX,
            directionY,
            directionZ,
            maxDistance);
    }

    RaycastHit3DResult raycast3DMask(
        float originX,
        float originY,
        float originZ,
        float directionX,
        float directionY,
        float directionZ,
        float maxDistance,
        int layerMask
    ) const {
        return world.raycast3D(
            originX,
            originY,
            originZ,
            directionX,
            directionY,
            directionZ,
            maxDistance,
            static_cast<uint32_t>(layerMask));
    }

    TextureHandleView loadTexture(std::string texturePath) {
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        resource::AssetHandle handle = resources.loadTextureHandle(texturePath);
        return makeTextureHandleView(handle, resources.loadTexture(handle));
    }

    AudioHandleView loadAudio(std::string clipPath, bool spatial) {
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        resource::AssetHandle handle = resources.loadAudioHandle(clipPath, spatial);
        return makeAudioHandleView(handle, resources.loadAudio(handle, spatial));
    }

    MeshHandleView loadMesh(std::string meshPath) {
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        resource::AssetHandle handle = resources.loadMeshHandle(meshPath);
        return makeMeshHandleView(handle, resources.loadMesh(handle));
    }

    ShaderHandleView loadShader(std::string shaderPath) {
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        resource::AssetHandle handle = resources.loadShaderHandle(shaderPath);
        return makeShaderHandleView(handle, resources.loadShader(handle));
    }

    MaterialHandleView loadMaterial(std::string materialPath, std::string shaderPath) {
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        resource::AssetHandle handle = resources.loadMaterialHandle(materialPath, shaderPath);
        return makeMaterialHandleView(handle, resources.loadMaterial(handle, shaderPath));
    }

    std::string importAsset(std::string sourcePath, std::string importedPath, std::string groupName, std::string bundleName) {
        return resource::ResourceManager::getInstance().importAsset(sourcePath, importedPath, groupName, bundleName);
    }

    bool createAssetBundle(std::string bundleName) {
        return resource::ResourceManager::getInstance().createBundle(bundleName);
    }

    bool addAssetToBundle(std::string bundleName, std::string assetPath) {
        return resource::ResourceManager::getInstance().addAssetToBundle(bundleName, assetPath);
    }

    int assetBundleAssetCount(std::string bundleName) const {
        return resource::ResourceManager::getInstance().bundleAssetCount(bundleName);
    }

    std::string assetBundleAsset(std::string bundleName, int index) const {
        return resource::ResourceManager::getInstance().bundleAsset(bundleName, index);
    }

    bool setAssetMemoryBudget(std::string assetType, int bytes) {
        return resource::ResourceManager::getInstance().setMemoryBudgetByName(assetType, bytes <= 0 ? 0 : static_cast<size_t>(bytes));
    }

    int assetMemoryBudget(std::string assetType) const {
        return static_cast<int>(resource::ResourceManager::getInstance().memoryBudgetByName(assetType));
    }

    int assetMemoryUsage(std::string assetType) const {
        return static_cast<int>(resource::ResourceManager::getInstance().memoryUsageByName(assetType));
    }

    void setAssetHotReload(bool enabled) {
        resource::ResourceManager::getInstance().setHotReloadEnabled(enabled);
    }

    bool assetHotReloadEnabled() const {
        return resource::ResourceManager::getInstance().hotReloadEnabled();
    }

    int pollAssetChanges() {
        return resource::ResourceManager::getInstance().pollForDirtyAssets();
    }

    bool reloadAsset(std::string assetPath) {
        return resource::ResourceManager::getInstance().reloadAsset(assetPath);
    }

    int reloadDirtyAssets() {
        return resource::ResourceManager::getInstance().reloadDirtyAssets();
    }

    bool markAssetDirty(std::string assetPath) {
        return resource::ResourceManager::getInstance().markAssetDirty(assetPath);
    }

    std::string assetGroup(std::string assetPath) const {
        return resource::ResourceManager::getInstance().assetGroup(assetPath);
    }

    std::string importedAssetPath(std::string assetPath) const {
        return resource::ResourceManager::getInstance().importedAssetPath(assetPath);
    }

    std::string assetDatabaseJson() const {
        return resource::ResourceManager::getInstance().metadataJson();
    }

    bool bakeAssetMetadata(std::string outputPath) const {
        return resource::ResourceManager::getInstance().bakeMetadata(outputPath);
    }

    EntityId spawnSprite(std::string name, float x, float y, float w, float h, std::string color) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        SpriteRenderer2D& sprite = world.addSpriteRenderer2D(entity);
        sprite.size = physics::Vec2(w, h);
        sprite.tintColor = color;
        return entity;
    }

    EntityId spawnTexturedSprite(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {
        EntityId entity = spawnSprite(name, x, y, w, h, color);
        if (!texturePath.empty()) {
            if (SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
                assignSpriteTextureBinding(*sprite, texturePath);
            }
        }
        return entity;
    }

    EntityId spawnTexturedSpriteHandle(std::string name, TextureHandleView texture, float x, float y, float w, float h, std::string color) {
        EntityId entity = spawnTexturedSprite(name, "", x, y, w, h, color);
        if (SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            assignSpriteTextureBinding(*sprite, texture);
        }
        return entity;
    }

    EntityId spawnCamera2D(std::string name, float x, float y, float zoom, bool primary) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        Camera2DComponent& camera = world.addCamera2D(entity);
        camera.zoom = zoom <= 0.0f ? 1.0f : zoom;
        camera.primary = primary;
        camera.viewportOrigin = physics::Vec2(0.0f, 0.0f);
        camera.viewportSize = physics::Vec2(1.0f, 1.0f);

        if (primary) {
            for (EntityId other : world.aliveEntities()) {
                if (other == entity) {
                    continue;
                }
                if (Camera2DComponent* otherCamera = world.getCamera2D(other)) {
                    otherCamera->primary = false;
                }
            }
        }

        return entity;
    }

    EntityId spawnTilemap2D(std::string name, float x, float y, int columns, int rows, float tileWidth, float tileHeight, std::string defaultColor) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        Tilemap2DComponent& tilemap = world.addTilemap2D(entity);
        tilemap.columns = std::max(1, columns);
        tilemap.rows = std::max(1, rows);
        tilemap.tileSize.x = tileWidth <= 0.0f ? 1.0f : tileWidth;
        tilemap.tileSize.y = tileHeight <= 0.0f ? 1.0f : tileHeight;
        tilemap.anchor = physics::Vec2(0.0f, 0.0f);
        tilemap.cells.assign(static_cast<size_t>(tilemap.columns * tilemap.rows), 0);
        tilemap.paletteColors = std::vector<std::string>{"", defaultColor.empty() ? "white" : defaultColor};
        return entity;
    }

    void resizeTilemap2D(EntityId entity, int columns, int rows, int fillTileId) {
        Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        if (tilemap == nullptr) {
            return;
        }
        tilemap->resize(columns, rows, fillTileId);
    }

    void setTilemapCell(EntityId entity, int column, int row, int tileId) {
        Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        if (tilemap == nullptr) {
            return;
        }
        tilemap->setCellAt(column, row, tileId);
    }

    int tilemapCell(EntityId entity, int column, int row) const {
        const Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        return tilemap == nullptr ? 0 : tilemap->cellAt(column, row);
    }

    void fillTilemap(EntityId entity, int tileId) {
        Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        if (tilemap == nullptr) {
            return;
        }
        tilemap->fill(tileId);
    }

    void clearTilemap(EntityId entity) {
        fillTilemap(entity, 0);
    }

    void setTilemapPaletteColor(EntityId entity, int tileId, std::string color) {
        Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        if (tilemap == nullptr) {
            return;
        }
        tilemap->setPaletteColor(tileId, color);
    }

    std::string tilemapPaletteColor(EntityId entity, int tileId) const {
        const Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        return tilemap == nullptr ? std::string() : tilemap->paletteColor(tileId);
    }

    EntityId spawnCharacter2D(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {
        EntityId entity = spawnTexturedSprite(name, texturePath, x, y, w, h, color);
        physics::RigidBody2D& body = world.addRigidBody2D(entity);
        if (const Transform2D* transform = world.getTransform2D(entity)) {
            body.position = transform->position;
        }

        world.addCharacter2D(entity);
        return entity;
    }

    EntityId spawnCharacter2DHandle(std::string name, TextureHandleView texture, float x, float y, float w, float h, std::string color) {
        EntityId entity = spawnCharacter2D(name, "", x, y, w, h, color);
        if (SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            assignSpriteTextureBinding(*sprite, texture);
        }
        return entity;
    }

    EntityId spawnAudioSource2D(std::string name, std::string clipPath, float x, float y, bool playOnAwake) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        AudioSource2DComponent& audio = world.addAudioSource2D(entity);
        if (!clipPath.empty()) {
            assignAudioSource2DBinding(audio, clipPath);
        }
        audio.playOnAwake = playOnAwake;
        audio.isPlaying = playOnAwake;
        return entity;
    }

    EntityId spawnAudioSource2DHandle(std::string name, AudioHandleView clip, float x, float y, bool playOnAwake) {
        EntityId entity = spawnAudioSource2D(name, "", x, y, playOnAwake);
        if (AudioSource2DComponent* audio = world.getAudioSource2D(entity)) {
            assignAudioSource2DBinding(*audio, clip);
        }
        return entity;
    }

    EntityId spawnAudioListener2D(std::string name, float x, float y, bool primary) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        if (primary) {
            std::optional<EntityId> current = world.primaryAudioListener2D();
            if (current.has_value()) {
                if (AudioListener2DComponent* existing = world.getAudioListener2D(current.value())) {
                    existing->primary = false;
                }
            }
        }

        AudioListener2DComponent& listener = world.addAudioListener2D(entity);
        listener.primary = primary;
        return entity;
    }

    EntityId spawnMesh(std::string name, std::string meshPath, std::string shaderPath, float x, float y, float z) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);
        MeshRenderer3D& mesh = world.addMeshRenderer3D(entity);
        if (!meshPath.empty()) {
            assignMeshBinding(mesh, meshPath);
        }
        if (!shaderPath.empty()) {
            assignShaderBinding(mesh, shaderPath);
        }
        return entity;
    }

    EntityId spawnMeshHandle(std::string name, MeshHandleView mesh, ShaderHandleView shader, float x, float y, float z) {
        EntityId entity = spawnMesh(name, "", "", x, y, z);
        if (MeshRenderer3D* meshRenderer = world.getMeshRenderer3D(entity)) {
            assignMeshBinding(*meshRenderer, mesh);
            assignShaderBinding(*meshRenderer, shader);
        }
        return entity;
    }

    EntityId spawnCharacter3D(std::string name, std::string meshPath, std::string shaderPath, std::string materialPath, float x, float y, float z) {
        EntityId entity = spawnMesh(name, meshPath, shaderPath, x, y, z);
        physics::RigidBody3D& body = world.addRigidBody3D(entity);
        body.position = physics::Vec3(x, y, z);

        Character3DComponent& character = world.addCharacter3D(entity);
        body.useGravity = character.useGravity;

        if (!materialPath.empty()) {
            if (MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                assignMaterialBinding(*mesh, materialPath, shaderPath);
            }
        }

        return entity;
    }

    EntityId spawnCharacter3DHandle(std::string name, MeshHandleView mesh, ShaderHandleView shader, MaterialHandleView material, float x, float y, float z) {
        EntityId entity = spawnCharacter3D(name, "", "", "", x, y, z);
        if (MeshRenderer3D* meshRenderer = world.getMeshRenderer3D(entity)) {
            assignMeshBinding(*meshRenderer, mesh);
            assignShaderBinding(*meshRenderer, shader);
            assignMaterialBinding(*meshRenderer, material);
        }
        return entity;
    }

    std::string createMaterial(std::string materialPath, std::string shaderPath) {
        if (materialPath.empty()) {
            return "";
        }
        return resource::ResourceManager::getInstance().loadMaterial(materialPath, shaderPath)->path;
    }

    bool materialExists(std::string materialPath) const {
        return !materialPath.empty() && resource::ResourceManager::getInstance().hasMaterial(materialPath);
    }

    void setMaterialShaderPath(std::string materialPath, std::string shaderPath) {
        if (materialPath.empty()) {
            return;
        }
        resource::ResourceManager::getInstance().loadMaterial(materialPath, shaderPath)->shaderPath = shaderPath;
    }

    std::string materialShaderPath(std::string materialPath) {
        if (materialPath.empty()) {
            return "";
        }
        return resource::ResourceManager::getInstance().loadMaterial(materialPath)->shaderPath;
    }

    std::string cloneMaterial(std::string sourceMaterialPath, std::string targetMaterialPath) {
        if (targetMaterialPath.empty()) {
            return "";
        }
        std::shared_ptr<resource::MaterialAsset> source = findMaterialAsset(sourceMaterialPath);
        if (!source) {
            return "";
        }
        std::shared_ptr<resource::MaterialAsset> target = resource::ResourceManager::getInstance().loadMaterial(targetMaterialPath, source->shaderPath);
        if (!target) {
            return "";
        }
        target->shaderPath = source->shaderPath;
        target->copyPropertiesFrom(*source);
        return target->path;
    }

    int copyMaterialProperties(std::string sourceMaterialPath, std::string targetMaterialPath) {
        if (targetMaterialPath.empty()) {
            return 0;
        }
        std::shared_ptr<resource::MaterialAsset> source = findMaterialAsset(sourceMaterialPath);
        std::shared_ptr<resource::MaterialAsset> target = ensureMaterialAsset(targetMaterialPath);
        if (!source || !target) {
            return 0;
        }
        return target->copyPropertiesFrom(*source);
    }

    bool removeMaterialProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->removeProperty(propertyName) : false;
    }

    int clearMaterialProperties(std::string materialPath) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->clearProperties() : 0;
    }

    bool defineMaterialText(std::string materialPath, std::string propertyName, std::string label, std::string defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineText(propertyName, label, defaultValue);
        return true;
    }

    bool defineMaterialNumber(std::string materialPath, std::string propertyName, std::string label, float defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineNumber(propertyName, label, defaultValue);
        return true;
    }

    bool defineMaterialToggle(std::string materialPath, std::string propertyName, std::string label, bool defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineToggle(propertyName, label, defaultValue);
        return true;
    }

    bool defineMaterialRadio(std::string materialPath, std::string propertyName, std::string label, std::string optionsCsv, std::string defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineRadio(propertyName, label, optionsCsv, defaultValue);
        return true;
    }

    bool defineMaterialImage(std::string materialPath, std::string propertyName, std::string label, std::string defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineImage(propertyName, label, defaultValue);
        return true;
    }

    bool defineMaterialButton(std::string materialPath, std::string propertyName, std::string label, std::string actionValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineButton(propertyName, label, actionValue);
        return true;
    }

    bool defineMaterialColor(std::string materialPath, std::string propertyName, std::string label, std::string defaultValue) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        if (!material || propertyName.empty()) {
            return false;
        }
        material->defineColor(propertyName, label, defaultValue);
        return true;
    }

    bool setMaterialTextProperty(std::string materialPath, std::string propertyName, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setText(propertyName, value) : false;
    }

    std::string materialTextProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->text(propertyName) : "";
    }

    bool setMaterialNumberProperty(std::string materialPath, std::string propertyName, float value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setNumber(propertyName, value) : false;
    }

    float materialNumberProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->number(propertyName) : 0.0f;
    }

    bool setMaterialToggleProperty(std::string materialPath, std::string propertyName, bool value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setToggle(propertyName, value) : false;
    }

    bool materialToggleProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->toggle(propertyName) : false;
    }

    bool setMaterialRadioProperty(std::string materialPath, std::string propertyName, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setRadio(propertyName, value) : false;
    }

    std::string materialRadioProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->radio(propertyName) : "";
    }

    bool setMaterialImageProperty(std::string materialPath, std::string propertyName, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setImage(propertyName, value) : false;
    }

    std::string materialImageProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->image(propertyName) : "";
    }

    bool setMaterialButtonProperty(std::string materialPath, std::string propertyName, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setButton(propertyName, value) : false;
    }

    std::string materialButtonProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->button(propertyName) : "";
    }

    bool setMaterialColorProperty(std::string materialPath, std::string propertyName, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setColor(propertyName, value) : false;
    }

    bool createMaterialVariant(std::string sourceMaterialPath, std::string variantPath) {
        return !cloneMaterial(sourceMaterialPath, variantPath).empty();
    }

    std::string materialColorProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->color(propertyName) : "";
    }

    bool materialHasProperty(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->hasProperty(propertyName) : false;
    }

    int materialPropertyCount(std::string materialPath) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->propertyCount() : 0;
    }

    std::string materialPropertyNameAt(std::string materialPath, int index) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyNameAt(index) : "";
    }

    std::string materialPropertyKind(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyKindName(propertyName) : "";
    }

    std::string materialPropertyLabel(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyLabel(propertyName) : "";
    }

    std::string materialPropertyOptions(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyOptionsCsv(propertyName) : "";
    }

    int materialPropertyOptionCount(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyOptionCount(propertyName) : 0;
    }

    bool triggerMaterialButton(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        if (!material) {
            return false;
        }

        std::string action;
        if (!material->triggerButton(propertyName, &action)) {
            return false;
        }

        if (!action.empty()) {
            triggerCallback(action);
        }
        return true;
    }

    int materialButtonTriggerCount(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyTriggerCount(propertyName) : 0;
    }

    bool setMaterialPropertyCallback(std::string materialPath, std::string propertyName, std::string callbackName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->setPropertyCallback(propertyName, callbackName) : false;
    }

    std::string materialPropertyCallback(std::string materialPath, std::string propertyName) const {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        return material ? material->propertyCallback(propertyName) : "";
    }

    bool notifyMaterialProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
        if (!material) {
            return false;
        }

        std::string callbackName;
        std::string callbackValue;
        if (!material->notifyProperty(propertyName, &callbackName, &callbackValue)) {
            return false;
        }

        if (!callbackName.empty()) {
            triggerCallback(callbackName, callbackValue);
        }
        return true;
    }

    bool addMaterialPropertyOption(std::string materialPath, std::string propertyName, std::string label, std::string value) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->addPropertyOption(propertyName, label, value) : false;
    }

    bool removeMaterialPropertyOption(std::string materialPath, std::string propertyName, int index) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->removePropertyOption(propertyName, index) : false;
    }

    int clearMaterialPropertyOptions(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->clearPropertyOptions(propertyName) : 0;
    }

    MaterialPropertyView materialProperty(std::string materialPath, std::string propertyName) {
        MaterialPropertyView view;
        std::shared_ptr<std::string> propertyKey = std::make_shared<std::string>(propertyName);

        view.exists = FieldRef<bool>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->hasProperty(*propertyKey) : false;
            },
            [](const bool&) {}
        );

        view.name = FieldRef<std::string>(
            [propertyKey]() {
                return *propertyKey;
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material && material->renameProperty(*propertyKey, value)) {
                    *propertyKey = value;
                }
            }
        );

        view.label = FieldRef<std::string>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyLabel(*propertyKey) : "";
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyLabel(*propertyKey, value);
                }
            }
        );

        view.kind = FieldRef<std::string>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyKindName(*propertyKey) : "";
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyKindName(*propertyKey, value);
                }
            }
        );

        view.options = FieldRef<std::string>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyOptionsCsv(*propertyKey) : "";
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyOptionsCsv(*propertyKey, value);
                }
            }
        );

        view.callback = FieldRef<std::string>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyCallback(*propertyKey) : "";
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyCallback(*propertyKey, value);
                }
            }
        );

        view.stringValue = FieldRef<std::string>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->text(*propertyKey) : "";
            },
            [this, materialPath, propertyKey](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setText(*propertyKey, value);
                }
            }
        );

        view.numberValue = FieldRef<float>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->number(*propertyKey) : 0.0f;
            },
            [this, materialPath, propertyKey](const float& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setNumber(*propertyKey, value);
                }
            }
        );

        view.boolValue = FieldRef<bool>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->toggle(*propertyKey) : false;
            },
            [this, materialPath, propertyKey](const bool& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setToggle(*propertyKey, value);
                }
            }
        );

        view.triggerCount = FieldRef<int>(
            [this, materialPath, propertyKey]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyTriggerCount(*propertyKey) : 0;
            },
            [this, materialPath, propertyKey](const int& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyTriggerCount(*propertyKey, value);
                }
            }
        );

        return view;
    }

    MaterialPropertyView materialPropertyAt(std::string materialPath, int index) {
        return materialProperty(materialPath, materialPropertyNameAt(materialPath, index));
    }

    MaterialPropertyOptionView materialPropertyOption(std::string materialPath, std::string propertyName, int index) {
        MaterialPropertyOptionView view;
        std::shared_ptr<std::string> propertyKey = std::make_shared<std::string>(propertyName);
        std::shared_ptr<int> optionIndex = std::make_shared<int>(index);

        view.exists = FieldRef<bool>(
            [this, materialPath, propertyKey, optionIndex]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyOptionCount(*propertyKey) > *optionIndex && *optionIndex >= 0 : false;
            },
            [](const bool&) {}
        );

        view.label = FieldRef<std::string>(
            [this, materialPath, propertyKey, optionIndex]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyOptionLabel(*propertyKey, *optionIndex) : "";
            },
            [this, materialPath, propertyKey, optionIndex](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyOptionLabel(*propertyKey, *optionIndex, value);
                }
            }
        );

        view.value = FieldRef<std::string>(
            [this, materialPath, propertyKey, optionIndex]() {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                return material ? material->propertyOptionValue(*propertyKey, *optionIndex) : "";
            },
            [this, materialPath, propertyKey, optionIndex](const std::string& value) {
                std::shared_ptr<resource::MaterialAsset> material = findMaterialAsset(materialPath);
                if (material) {
                    material->setPropertyOptionValue(*propertyKey, *optionIndex, value);
                }
            }
        );

        return view;
    }

    bool setMeshMaterial(EntityId entity, std::string materialPath) {
        if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
            assignMaterialBinding(*mesh, materialPath);
            return true;
        }
        return false;
    }

    std::string meshMaterialPath(EntityId entity) {
        if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
            return assetPathFromBinding(mesh->materialHandle, mesh->material);
        }
        return "";
    }

    MaterialHandleView meshMaterialHandle(EntityId entity) {
        if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
            return makeMaterialHandleView(mesh->materialHandle, mesh->material);
        }
        return MaterialHandleView{};
    }

    EntityId spawnCamera3D(std::string name, float x, float y, float z, float fov, bool primary) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);
        if (primary) {
            std::optional<EntityId> current = world.primaryCamera3D();
            if (current.has_value()) {
                if (Camera3DComponent* existing = world.getCamera3D(current.value())) {
                    existing->primary = false;
                }
            }
        }
        Camera3DComponent& camera = world.addCamera3D(entity);
        camera.fov = fov;
        camera.primary = primary;
        return entity;
    }

    void setEntityPosition3D(EntityId entity, float x, float y, float z) {
        if (!world.isAlive(entity)) {
            return;
        }
        Transform3D* transform = ensureTransform3DViewComponent(entity);
        if (transform != nullptr) {
            transform->position = physics::Vec3(x, y, z);
        }
        if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
            body->position = physics::Vec3(x, y, z);
        }
    }

    void moveEntity3D(EntityId entity, float dx, float dy, float dz) {
        Transform3DView view = transform3D(entity);
        setEntityPosition3D(entity, view.x + dx, view.y + dy, view.z + dz);
    }

    float entityPositionZ(EntityId entity) {
        if (const Transform3D* transform = world.getTransform3D(entity)) {
            return transform->position.z;
        }
        if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
            return body->position.z;
        }
        return 0.0f;
    }

    Transform3DView transform3D(EntityId entity) {
        Transform3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.z = value;
                }
            });

        view.rotationX = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.x;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->rotation.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.x = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->rotation.x = value;
                }
            });

        view.rotationY = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.y;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->rotation.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.y = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->rotation.y = value;
                }
            });

        view.rotationZ = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.z;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->rotation.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.z = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->rotation.z = value;
                }
            });

        view.scaleX = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->scale.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->scale.x = value;
                }
            });

        view.scaleY = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->scale.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->scale.y = value;
                }
            });

        view.scaleZ = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->scale.z;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->scale.z = value;
                }
            });

        return view;
    }

    Body3DView body3D(EntityId entity) {
        Body3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.x;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.x = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.y;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.y = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.z;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.z = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.vx = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.x = value;
                }
            });

        view.vy = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.y = value;
                }
            });

        view.vz = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.z = value;
                }
            });

        view.mass = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->mass;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->mass = value <= 0.0f ? 1.0f : value;
                }
            });

        view.gravityScale = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->gravityScale;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->gravityScale = value;
                }
            });

        view.friction = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->friction;
                }
                return 0.2f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->friction = value;
                }
            });

        view.restitution = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->restitution;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->restitution = value;
                }
            });

        view.useGravity = FieldRef<bool>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->useGravity;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->useGravity = value;
                }
            });

        view.isGrounded = FieldRef<bool>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->isGrounded;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->isGrounded = value;
                }
            });

        return view;
    }

    BoxCollider3DView boxCollider3D(EntityId entity) {
        BoxCollider3DView view;

        view.offsetX = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->offset.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->offset.x = value;
                }
            });

        view.offsetY = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->offset.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->offset.y = value;
                }
            });

        view.offsetZ = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->offset.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->offset.z = value;
                }
            });

        view.width = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->size.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->size.x = value <= 0.0f ? 0.01f : value;
                }
            });

        view.height = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->size.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->size.y = value <= 0.0f ? 0.01f : value;
                }
            });

        view.depth = FieldRef<float>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->size.z;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->size.z = value <= 0.0f ? 0.01f : value;
                }
            });

        view.isTrigger = FieldRef<bool>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->isTrigger;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->isTrigger = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const physics::BoxCollider3D* collider = world.getBoxCollider3D(entity)) {
                    return collider->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::BoxCollider3D* collider = ensureBoxCollider3DViewComponent(entity)) {
                    collider->enabled = value;
                }
            });

        return view;
    }

    SphereCollider3DView sphereCollider3D(EntityId entity) {
        SphereCollider3DView view;

        view.offsetX = FieldRef<float>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->offset.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->offset.x = value;
                }
            });

        view.offsetY = FieldRef<float>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->offset.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->offset.y = value;
                }
            });

        view.offsetZ = FieldRef<float>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->offset.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->offset.z = value;
                }
            });

        view.radius = FieldRef<float>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->radius;
                }
                return 0.5f;
            },
            [this, entity](const float& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->radius = value <= 0.0f ? 0.01f : value;
                }
            });

        view.isTrigger = FieldRef<bool>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->isTrigger;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->isTrigger = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const physics::SphereCollider3D* collider = world.getSphereCollider3D(entity)) {
                    return collider->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::SphereCollider3D* collider = ensureSphereCollider3DViewComponent(entity)) {
                    collider->enabled = value;
                }
            });

        return view;
    }

    Camera3DView camera3D(EntityId entity) {
        Camera3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.rotationX = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.x = value;
                }
            });

        view.rotationY = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.y = value;
                }
            });

        view.rotationZ = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->rotation.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->rotation.z = value;
                }
            });

        view.fov = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->fov;
                }
                return 60.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->fov = value;
                }
            });

        view.nearClip = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->nearClip;
                }
                return 0.1f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->nearClip = value;
                }
            });

        view.farClip = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->farClip;
                }
                return 1000.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->farClip = value;
                }
            });

        view.primary = FieldRef<bool>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->primary;
                }
                return false;
            },
            [this, entity](const bool& value) {
                Camera3DComponent* camera = ensureCamera3DViewComponent(entity);
                if (camera == nullptr) {
                    return;
                }
                if (value) {
                    std::optional<EntityId> current = world.primaryCamera3D();
                    if (current.has_value() && current.value() != entity) {
                        if (Camera3DComponent* currentCamera = world.getCamera3D(current.value())) {
                            currentCamera->primary = false;
                        }
                    }
                }
                camera->primary = value;
            });

        view.viewportX = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->viewportOrigin.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->viewportOrigin.x = value;
                }
            });

        view.viewportY = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->viewportOrigin.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->viewportOrigin.y = value;
                }
            });

        view.viewportWidth = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->viewportSize.x;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->viewportSize.x = value;
                }
            });

        view.viewportHeight = FieldRef<float>(
            [this, entity]() {
                if (const Camera3DComponent* camera = world.getCamera3D(entity)) {
                    return camera->viewportSize.y;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (Camera3DComponent* camera = ensureCamera3DViewComponent(entity)) {
                    camera->viewportSize.y = value;
                }
            });

        return view;
    }

    AudioListener3DView audioListener3D(EntityId entity) {
        AudioListener3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.gain = FieldRef<float>(
            [this, entity]() {
                if (const AudioListener3DComponent* listener = world.getAudioListener3D(entity)) {
                    return listener->gain;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioListener3DComponent* listener = ensureAudioListener3DViewComponent(entity)) {
                    listener->gain = value;
                }
            });

        view.primary = FieldRef<bool>(
            [this, entity]() {
                if (const AudioListener3DComponent* listener = world.getAudioListener3D(entity)) {
                    return listener->primary;
                }
                return false;
            },
            [this, entity](const bool& value) {
                AudioListener3DComponent* listener = ensureAudioListener3DViewComponent(entity);
                if (listener == nullptr) {
                    return;
                }
                if (value) {
                    std::optional<EntityId> current = world.primaryAudioListener3D();
                    if (current.has_value() && current.value() != entity) {
                        if (AudioListener3DComponent* currentListener = world.getAudioListener3D(current.value())) {
                            currentListener->primary = false;
                        }
                    }
                }
                listener->primary = value;
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const AudioListener3DComponent* listener = world.getAudioListener3D(entity)) {
                    return listener->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (AudioListener3DComponent* listener = ensureAudioListener3DViewComponent(entity)) {
                    listener->enabled = value;
                }
            });

        return view;
    }

    PointLight3DView pointLight3D(EntityId entity) {
        PointLight3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.intensity = FieldRef<float>(
            [this, entity]() {
                if (const PointLight3DComponent* light = world.getPointLight3D(entity)) {
                    return light->intensity;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (PointLight3DComponent* light = ensurePointLight3DViewComponent(entity)) {
                    light->intensity = value;
                }
            });

        view.range = FieldRef<float>(
            [this, entity]() {
                if (const PointLight3DComponent* light = world.getPointLight3D(entity)) {
                    return light->range;
                }
                return 10.0f;
            },
            [this, entity](const float& value) {
                if (PointLight3DComponent* light = ensurePointLight3DViewComponent(entity)) {
                    light->range = value;
                }
            });

        view.color = FieldRef<std::string>(
            [this, entity]() {
                if (const PointLight3DComponent* light = world.getPointLight3D(entity)) {
                    return light->color;
                }
                return std::string("white");
            },
            [this, entity](const std::string& value) {
                if (PointLight3DComponent* light = ensurePointLight3DViewComponent(entity)) {
                    light->color = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const PointLight3DComponent* light = world.getPointLight3D(entity)) {
                    return light->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (PointLight3DComponent* light = ensurePointLight3DViewComponent(entity)) {
                    light->enabled = value;
                }
            });

        return view;
    }

    DirectionalLight3DView directionalLight3D(EntityId entity) {
        DirectionalLight3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.directionX = FieldRef<float>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->direction.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->direction.x = value;
                }
            });

        view.directionY = FieldRef<float>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->direction.y;
                }
                return -1.0f;
            },
            [this, entity](const float& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->direction.y = value;
                }
            });

        view.directionZ = FieldRef<float>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->direction.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->direction.z = value;
                }
            });

        view.intensity = FieldRef<float>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->intensity;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->intensity = value;
                }
            });

        view.color = FieldRef<std::string>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->color;
                }
                return std::string("white");
            },
            [this, entity](const std::string& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->color = value;
                }
            });

        view.castShadows = FieldRef<bool>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->castShadows;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->castShadows = value;
                }
            });

        view.enabled = FieldRef<bool>(
            [this, entity]() {
                if (const DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity)) {
                    return light->enabled;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (DirectionalLight3DComponent* light = ensureDirectionalLight3DViewComponent(entity)) {
                    light->enabled = value;
                }
            });

        return view;
    }

    Mesh3DView mesh3D(EntityId entity) {
        Mesh3DView view;

        view.meshPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->meshHandle, mesh->mesh);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMeshBinding(*mesh, value);
                }
            });

        view.shaderPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->shaderHandle, mesh->shader);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignShaderBinding(*mesh, value);
                }
            });

        view.materialPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->materialHandle, mesh->material);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMaterialBinding(*mesh, value);
                }
            });

        view.mesh = FieldRef<MeshHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeMeshHandleView(mesh->meshHandle, mesh->mesh);
                }
                return MeshHandleView{};
            },
            [this, entity](const MeshHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMeshBinding(*mesh, value);
                }
            });

        view.shader = FieldRef<ShaderHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeShaderHandleView(mesh->shaderHandle, mesh->shader);
                }
                return ShaderHandleView{};
            },
            [this, entity](const ShaderHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignShaderBinding(*mesh, value);
                }
            });

        view.material = FieldRef<MaterialHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeMaterialHandleView(mesh->materialHandle, mesh->material);
                }
                return MaterialHandleView{};
            },
            [this, entity](const MaterialHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMaterialBinding(*mesh, value);
                }
            });

        view.visible = FieldRef<bool>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return mesh->visible;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->visible = value;
                }
            });

        view.castShadows = FieldRef<bool>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return mesh->castShadows;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->castShadows = value;
                }
            });

        return view;
    }

    Character3DView character3D(EntityId entity) {
        Character3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.x;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.x = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.y;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.y = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.z;
                }
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->position.z = value;
                }
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
            });

        view.vx = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.x = value;
                }
            });

        view.vy = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.y = value;
                }
            });

        view.vz = FieldRef<float>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->velocity.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->velocity.z = value;
                }
            });

        view.moveSpeed = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->moveSpeed;
                }
                return 6.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->moveSpeed = value;
                }
            });

        view.turnSpeed = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->turnSpeed;
                }
                return 4.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->turnSpeed = value;
                }
            });

        view.jumpSpeed = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->jumpSpeed;
                }
                return 7.5f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->jumpSpeed = std::max(0.0f, value);
                }
            });

        view.groundAcceleration = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->groundAcceleration;
                }
                return 36.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->groundAcceleration = std::max(0.0f, value);
                }
            });

        view.airAcceleration = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->airAcceleration;
                }
                return 14.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->airAcceleration = std::max(0.0f, value);
                }
            });

        view.groundFriction = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->groundFriction;
                }
                return 20.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->groundFriction = std::max(0.0f, value);
                }
            });

        view.airControl = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->airControl;
                }
                return 0.35f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->airControl = std::clamp(value, 0.0f, 1.0f);
                }
            });

        view.groundSnapDistance = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->groundSnapDistance;
                }
                return 0.2f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->groundSnapDistance = std::max(0.0f, value);
                }
            });

        view.maxSlopeAngle = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->maxSlopeAngle;
                }
                return 55.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->maxSlopeAngle = std::clamp(value, 0.0f, 89.0f);
                }
            });

        view.moveInputX = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->moveInput.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->moveInput.x = value;
                }
            });

        view.moveInputY = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->moveInput.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->moveInput.y = value;
                }
            });

        view.moveInputZ = FieldRef<float>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->moveInput.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->moveInput.z = value;
                }
            });

        view.isGrounded = FieldRef<bool>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->isGrounded;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->isGrounded;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->isGrounded = value;
                }
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->isGrounded = value;
                }
            });

        view.useGravity = FieldRef<bool>(
            [this, entity]() {
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->useGravity;
                }
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->useGravity;
                }
                return true;
            },
            [this, entity](const bool& value) {
                if (physics::RigidBody3D* body = ensureBody3DViewComponent(entity)) {
                    body->useGravity = value;
                }
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->useGravity = value;
                }
            });

        view.meshPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->meshHandle, mesh->mesh);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMeshBinding(*mesh, value);
                }
            });

        view.shaderPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->shaderHandle, mesh->shader);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignShaderBinding(*mesh, value);
                }
            });

        view.materialPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return assetPathFromBinding(mesh->materialHandle, mesh->material);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMaterialBinding(*mesh, value);
                }
            });

        view.mesh = FieldRef<MeshHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeMeshHandleView(mesh->meshHandle, mesh->mesh);
                }
                return MeshHandleView{};
            },
            [this, entity](const MeshHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMeshBinding(*mesh, value);
                }
            });

        view.shader = FieldRef<ShaderHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeShaderHandleView(mesh->shaderHandle, mesh->shader);
                }
                return ShaderHandleView{};
            },
            [this, entity](const ShaderHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignShaderBinding(*mesh, value);
                }
            });

        view.material = FieldRef<MaterialHandleView>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    return makeMaterialHandleView(mesh->materialHandle, mesh->material);
                }
                return MaterialHandleView{};
            },
            [this, entity](const MaterialHandleView& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    assignMaterialBinding(*mesh, value);
                }
            });

        return view;
    }

    AudioSource3DView audioSource3D(EntityId entity) {
        AudioSource3DView view;

        view.x = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.x;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.x;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.x = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.x = value;
                }
            });

        view.y = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.y;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.y;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.y = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.y = value;
                }
            });

        view.z = FieldRef<float>(
            [this, entity]() {
                if (const Transform3D* transform = world.getTransform3D(entity)) {
                    return transform->position.z;
                }
                if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    return body->position.z;
                }
                return 0.0f;
            },
            [this, entity](const float& value) {
                if (Transform3D* transform = ensureTransform3DViewComponent(entity)) {
                    transform->position.z = value;
                }
                if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                    body->position.z = value;
                }
            });

        view.clipPath = FieldRef<std::string>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return assetPathFromBinding(audio->clipHandle, audio->clip);
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    assignAudioSource3DBinding(*audio, value);
                }
            });

        view.clip = FieldRef<AudioHandleView>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return makeAudioHandleView(audio->clipHandle, audio->clip);
                }
                return AudioHandleView{};
            },
            [this, entity](const AudioHandleView& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    assignAudioSource3DBinding(*audio, value);
                }
            });

        view.volume = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->volume;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->volume = value;
                }
            });

        view.pitch = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->pitch;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->pitch = value;
                }
            });

        view.loop = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->loop;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->loop = value;
                }
            });

        view.playOnAwake = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->playOnAwake;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->playOnAwake = value;
                }
            });

        view.isPlaying = FieldRef<bool>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->isPlaying;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->isPlaying = value;
                }
            });

        view.minDistance = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->minDistance;
                }
                return 1.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->minDistance = value;
                }
            });

        view.maxDistance = FieldRef<float>(
            [this, entity]() {
                if (const AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
                    return audio->maxDistance;
                }
                return 20.0f;
            },
            [this, entity](const float& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->maxDistance = value;
                }
            });

        return view;
    }

    EntityId spawnAudioSource3D(std::string name, std::string clipPath, float x, float y, float z, bool playOnAwake) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);

        AudioSource3DComponent& audio = world.addAudioSource3D(entity);
        if (!clipPath.empty()) {
            assignAudioSource3DBinding(audio, clipPath);
        }
        audio.playOnAwake = playOnAwake;
        audio.isPlaying = playOnAwake;
        return entity;
    }

    EntityId spawnAudioSource3DHandle(std::string name, AudioHandleView clip, float x, float y, float z, bool playOnAwake) {
        EntityId entity = spawnAudioSource3D(name, "", x, y, z, playOnAwake);
        if (AudioSource3DComponent* audio = world.getAudioSource3D(entity)) {
            assignAudioSource3DBinding(*audio, clip);
        }
        return entity;
    }

    EntityId spawnPointLight3D(std::string name, float x, float y, float z, std::string color, float intensity, float range) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);

        PointLight3DComponent& light = world.addPointLight3D(entity);
        light.color = color.empty() ? "white" : color;
        light.intensity = intensity;
        light.range = range;
        return entity;
    }

    EntityId spawnDirectionalLight3D(std::string name, float x, float y, float z, float dirX, float dirY, float dirZ, std::string color, float intensity, bool castShadows) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);

        DirectionalLight3DComponent& light = world.addDirectionalLight3D(entity);
        light.direction = physics::Vec3(dirX, dirY, dirZ);
        light.color = color.empty() ? "white" : color;
        light.intensity = intensity;
        light.castShadows = castShadows;
        return entity;
    }

    EntityId spawnAudioListener3D(std::string name, float x, float y, float z, bool primary) {
        EntityId entity = world.createEntity(name);
        Transform3D& transform = world.addTransform3D(entity);
        transform.position = physics::Vec3(x, y, z);

        if (primary) {
            std::optional<EntityId> current = world.primaryAudioListener3D();
            if (current.has_value()) {
                if (AudioListener3DComponent* existing = world.getAudioListener3D(current.value())) {
                    existing->primary = false;
                }
            }
        }

        AudioListener3DComponent& listener = world.addAudioListener3D(entity);
        listener.primary = primary;
        return entity;
    }

    bool playAudio(EntityId entity) {
        if (AudioSource2DComponent* audio2D = world.getAudioSource2D(entity)) {
            audio2D->isPlaying = true;
            return true;
        }
        if (AudioSource3DComponent* audio3D = world.getAudioSource3D(entity)) {
            audio3D->isPlaying = true;
            return true;
        }
        return false;
    }

    bool stopAudio(EntityId entity) {
        if (AudioSource2DComponent* audio2D = world.getAudioSource2D(entity)) {
            audio2D->isPlaying = false;
            return true;
        }
        if (AudioSource3DComponent* audio3D = world.getAudioSource3D(entity)) {
            audio3D->isPlaying = false;
            return true;
        }
        return false;
    }

protected:
    virtual void onLoad() {}
    virtual void onFrame(float dt) {}
    virtual void onFixedUpdate(float dt) {}
    virtual void onPostPhysics(float dt) {}
    virtual void onDraw(Canvas& canvas, float alpha) {}

    void rebuildPhysicsBindings() {
        physics.clearBodies();
        world.syncPhysicsFromTransforms2D();
        world.syncPhysicsFromTransforms3D();

        world.forEachRigidBody2D([this](EntityId, physics::RigidBody2D& body) {
            physics.registerBody(&body);
        });

        world.forEachRigidBody3D([this](EntityId, physics::RigidBody3D& body) {
            physics.registerBody(&body);
        });
    }

    void syncFromPhysics() {
        world.syncTransformsFromPhysics2D();
        world.syncTransformsFromPhysics3D();
    }

    struct CharacterControllerShape3D {
        enum class Kind {
            Sphere,
            Box
        };

        Kind kind = Kind::Sphere;
        physics::SphereCollider3D sphere;
        physics::BoxCollider3D box;
        float halfHeight = 0.5f;
    };

    static float moveTowardsScalar(float current, float target, float maxDelta) {
        if (maxDelta <= 0.0f) {
            return current;
        }
        if (current < target) {
            return std::min(current + maxDelta, target);
        }
        return std::max(current - maxDelta, target);
    }

    physics::Vec3 currentEntityPosition3D(EntityId entity) const {
        if (const physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
            return body->position;
        }
        if (const Transform3D* transform = world.getTransform3D(entity)) {
            return transform->position;
        }
        return physics::Vec3();
    }

    CharacterControllerShape3D describeCharacterControllerShape3D(EntityId entity) const {
        CharacterControllerShape3D shape;

        if (const physics::SphereCollider3D* sphere = world.getSphereCollider3D(entity)) {
            if (sphere->enabled && !sphere->isTrigger) {
                shape.kind = CharacterControllerShape3D::Kind::Sphere;
                shape.sphere = *sphere;
                shape.halfHeight = std::max(0.01f, sphere->radius);
                return shape;
            }
        }

        if (const physics::BoxCollider3D* box = world.getBoxCollider3D(entity)) {
            if (box->enabled && !box->isTrigger) {
                shape.kind = CharacterControllerShape3D::Kind::Box;
                shape.box = *box;
                shape.halfHeight = std::max(0.01f, box->size.y * 0.5f);
                return shape;
            }
        }

        shape.kind = CharacterControllerShape3D::Kind::Sphere;
        shape.sphere.radius = 0.5f;
        shape.sphere.enabled = true;
        shape.sphere.isTrigger = false;
        shape.halfHeight = 0.5f;
        return shape;
    }

    static bool computeSphereSpherePenetration(
        const physics::SphereCollider3D& sphereA,
        const physics::Vec3& positionA,
        const physics::SphereCollider3D& sphereB,
        const physics::Vec3& positionB,
        physics::Vec3& outNormal,
        float& outPenetration
    ) {
        if (!sphereA.enabled || !sphereB.enabled) {
            return false;
        }

        const physics::Vec3 centerA = positionA + sphereA.offset;
        const physics::Vec3 centerB = positionB + sphereB.offset;
        const physics::Vec3 delta = centerA - centerB;
        const float radius = sphereA.radius + sphereB.radius;
        const float distanceSquared = delta.lengthSquared();
        if (distanceSquared >= radius * radius) {
            return false;
        }

        const float distance = std::sqrt(std::max(0.0f, distanceSquared));
        if (distance > 0.000001f) {
            outNormal = delta / distance;
            outPenetration = radius - distance;
        } else {
            outNormal = physics::Vec3(0.0f, 1.0f, 0.0f);
            outPenetration = radius;
        }
        return outPenetration > 0.000001f;
    }

    static bool computeBoxBoxPenetration(
        const physics::BoxCollider3D& boxA,
        const physics::Vec3& positionA,
        const physics::BoxCollider3D& boxB,
        const physics::Vec3& positionB,
        physics::Vec3& outNormal,
        float& outPenetration
    ) {
        if (!boxA.enabled || !boxB.enabled) {
            return false;
        }

        const physics::Vec3 centerA = positionA + boxA.offset;
        const physics::Vec3 centerB = positionB + boxB.offset;
        const physics::Vec3 halfA = boxA.size * 0.5f;
        const physics::Vec3 halfB = boxB.size * 0.5f;
        const physics::Vec3 delta = centerA - centerB;

        const float overlapX = (halfA.x + halfB.x) - std::abs(delta.x);
        const float overlapY = (halfA.y + halfB.y) - std::abs(delta.y);
        const float overlapZ = (halfA.z + halfB.z) - std::abs(delta.z);
        if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f) {
            return false;
        }

        outPenetration = overlapX;
        outNormal = physics::Vec3(delta.x < 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f);

        if (overlapY < outPenetration) {
            outPenetration = overlapY;
            outNormal = physics::Vec3(0.0f, delta.y < 0.0f ? -1.0f : 1.0f, 0.0f);
        }

        if (overlapZ < outPenetration) {
            outPenetration = overlapZ;
            outNormal = physics::Vec3(0.0f, 0.0f, delta.z < 0.0f ? -1.0f : 1.0f);
        }

        return outPenetration > 0.000001f;
    }

    static bool computeSphereBoxPenetration(
        const physics::SphereCollider3D& sphere,
        const physics::Vec3& spherePosition,
        const physics::BoxCollider3D& box,
        const physics::Vec3& boxPosition,
        physics::Vec3& outNormal,
        float& outPenetration
    ) {
        if (!sphere.enabled || !box.enabled) {
            return false;
        }

        const physics::Vec3 center = spherePosition + sphere.offset;
        const physics::Vec3 boxCenter = boxPosition + box.offset;
        const physics::Vec3 halfExtent = box.size * 0.5f;
        const physics::Vec3 minBound = boxCenter - halfExtent;
        const physics::Vec3 maxBound = boxCenter + halfExtent;

        const physics::Vec3 closest(
            std::clamp(center.x, minBound.x, maxBound.x),
            std::clamp(center.y, minBound.y, maxBound.y),
            std::clamp(center.z, minBound.z, maxBound.z));

        const physics::Vec3 delta = center - closest;
        const float distanceSquared = delta.lengthSquared();
        if (distanceSquared > sphere.radius * sphere.radius) {
            return false;
        }

        if (distanceSquared > 0.000001f) {
            const float distance = std::sqrt(distanceSquared);
            outNormal = delta / distance;
            outPenetration = sphere.radius - distance;
            return outPenetration > 0.000001f;
        }

        const float distanceToMinX = center.x - minBound.x;
        const float distanceToMaxX = maxBound.x - center.x;
        const float distanceToMinY = center.y - minBound.y;
        const float distanceToMaxY = maxBound.y - center.y;
        const float distanceToMinZ = center.z - minBound.z;
        const float distanceToMaxZ = maxBound.z - center.z;

        outNormal = physics::Vec3(-1.0f, 0.0f, 0.0f);
        outPenetration = sphere.radius + distanceToMinX;

        if (sphere.radius + distanceToMaxX < outPenetration) {
            outNormal = physics::Vec3(1.0f, 0.0f, 0.0f);
            outPenetration = sphere.radius + distanceToMaxX;
        }
        if (sphere.radius + distanceToMinY < outPenetration) {
            outNormal = physics::Vec3(0.0f, -1.0f, 0.0f);
            outPenetration = sphere.radius + distanceToMinY;
        }
        if (sphere.radius + distanceToMaxY < outPenetration) {
            outNormal = physics::Vec3(0.0f, 1.0f, 0.0f);
            outPenetration = sphere.radius + distanceToMaxY;
        }
        if (sphere.radius + distanceToMinZ < outPenetration) {
            outNormal = physics::Vec3(0.0f, 0.0f, -1.0f);
            outPenetration = sphere.radius + distanceToMinZ;
        }
        if (sphere.radius + distanceToMaxZ < outPenetration) {
            outNormal = physics::Vec3(0.0f, 0.0f, 1.0f);
            outPenetration = sphere.radius + distanceToMaxZ;
        }

        return outPenetration > 0.000001f;
    }

    bool computeCharacterPenetration3D(
        const CharacterControllerShape3D& shape,
        const physics::Vec3& position,
        EntityId other,
        physics::Vec3& outNormal,
        float& outPenetration
    ) const {
        if (!world.isAlive(other)) {
            return false;
        }

        const physics::Vec3 otherPosition = currentEntityPosition3D(other);
        bool found = false;

        const auto consider = [&](const physics::Vec3& normal, float penetration) {
            if (penetration <= 0.000001f) {
                return;
            }
            if (!found || penetration > outPenetration) {
                found = true;
                outNormal = normal;
                outPenetration = penetration;
            }
        };

        if (const physics::BoxCollider3D* otherBox = world.getBoxCollider3D(other)) {
            if (otherBox->enabled && !otherBox->isTrigger) {
                physics::Vec3 normal;
                float penetration = 0.0f;
                if (shape.kind == CharacterControllerShape3D::Kind::Sphere) {
                    if (computeSphereBoxPenetration(shape.sphere, position, *otherBox, otherPosition, normal, penetration)) {
                        consider(normal, penetration);
                    }
                } else if (computeBoxBoxPenetration(shape.box, position, *otherBox, otherPosition, normal, penetration)) {
                    consider(normal, penetration);
                }
            }
        }

        if (const physics::SphereCollider3D* otherSphere = world.getSphereCollider3D(other)) {
            if (otherSphere->enabled && !otherSphere->isTrigger) {
                physics::Vec3 normal;
                float penetration = 0.0f;
                if (shape.kind == CharacterControllerShape3D::Kind::Sphere) {
                    if (computeSphereSpherePenetration(shape.sphere, position, *otherSphere, otherPosition, normal, penetration)) {
                        consider(normal, penetration);
                    }
                } else if (computeSphereBoxPenetration(*otherSphere, otherPosition, shape.box, position, normal, penetration)) {
                    consider(normal * -1.0f, penetration);
                }
            }
        }

        return found;
    }

    bool findCharacterPenetration3D(
        EntityId entity,
        const CharacterControllerShape3D& shape,
        const physics::Vec3& position,
        physics::Vec3& outNormal,
        float& outPenetration
    ) const {
        bool found = false;

        const auto considerEntity = [&](EntityId other) {
            if (other == entity || !world.canInteract(entity, other)) {
                return;
            }

            physics::Vec3 normal;
            float penetration = 0.0f;
            if (!computeCharacterPenetration3D(shape, position, other, normal, penetration)) {
                return;
            }

            if (!found || penetration > outPenetration) {
                found = true;
                outNormal = normal;
                outPenetration = penetration;
            }
        };

        for (EntityId other : world.boxColliderEntities3D()) {
            considerEntity(other);
        }
        for (EntityId other : world.sphereColliderEntities3D()) {
            considerEntity(other);
        }

        return found;
    }

    bool snapCharacterToGround3D(
        EntityId entity,
        Character3DComponent& character,
        physics::RigidBody3D& body,
        float minimumGroundDot
    ) const {
        if (!character.useGravity || character.groundSnapDistance <= 0.0f || body.velocity.y > 0.0f) {
            return false;
        }

        const CharacterControllerShape3D shape = describeCharacterControllerShape3D(entity);
        const physics::Vec3 probePosition = body.position - physics::Vec3(0.0f, character.groundSnapDistance, 0.0f);

        physics::Vec3 normal;
        float penetration = 0.0f;
        if (!findCharacterPenetration3D(entity, shape, probePosition, normal, penetration)) {
            return false;
        }

        if (normal.y < minimumGroundDot) {
            return false;
        }

        const float snapDistance = std::clamp(character.groundSnapDistance - penetration, 0.0f, character.groundSnapDistance);
        body.position.y -= snapDistance;
        if (body.velocity.y < 0.0f) {
            body.velocity.y = 0.0f;
        }
        return true;
    }

    void applyCharacterControllerInputs3D(float dt) {
        world.forEachCharacter3D([this, dt](EntityId entity, Character3DComponent& character) {
            physics::RigidBody3D* body = world.getRigidBody3D(entity);
            if (body == nullptr || body->bodyType != physics::BodyType::Dynamic) {
                return;
            }

            body->useGravity = character.useGravity;

            physics::Vec3 input = character.moveInput;
            const float inputMagnitude = input.length();
            if (inputMagnitude > 1.0f) {
                input = input / inputMagnitude;
            }

            if (character.useGravity) {
                input.y = 0.0f;
                const float desiredVelocityX = input.x * character.moveSpeed;
                const float desiredVelocityZ = input.z * character.moveSpeed;
                const float horizontalMagnitude = std::sqrt((input.x * input.x) + (input.z * input.z));
                const float acceleration = character.isGrounded
                    ? std::max(0.0f, character.groundAcceleration)
                    : std::max(0.0f, character.airAcceleration * std::clamp(character.airControl, 0.0f, 1.0f));

                body->velocity.x = moveTowardsScalar(body->velocity.x, desiredVelocityX, acceleration * dt);
                body->velocity.z = moveTowardsScalar(body->velocity.z, desiredVelocityZ, acceleration * dt);

                if (character.isGrounded && horizontalMagnitude <= 0.0001f) {
                    const float frictionStep = std::max(0.0f, character.groundFriction) * dt;
                    body->velocity.x = moveTowardsScalar(body->velocity.x, 0.0f, frictionStep);
                    body->velocity.z = moveTowardsScalar(body->velocity.z, 0.0f, frictionStep);
                }

                if (character.jumpQueued && character.isGrounded) {
                    body->velocity.y = std::max(body->velocity.y, character.jumpSpeed);
                    body->isGrounded = false;
                    character.isGrounded = false;
                }
            } else {
                const float acceleration = std::max(0.0f, character.groundAcceleration);
                const physics::Vec3 desiredVelocity = input * character.moveSpeed;
                body->velocity.x = moveTowardsScalar(body->velocity.x, desiredVelocity.x, acceleration * dt);
                body->velocity.y = moveTowardsScalar(body->velocity.y, desiredVelocity.y, acceleration * dt);
                body->velocity.z = moveTowardsScalar(body->velocity.z, desiredVelocity.z, acceleration * dt);

                if (character.jumpQueued) {
                    body->velocity.y = character.jumpSpeed;
                }
            }

            character.jumpQueued = false;
        });
    }

    void resolveCharacterControllers3D() {
        constexpr float kPi = 3.14159265358979323846f;

        world.forEachCharacter3D([this](EntityId entity, Character3DComponent& character) {
            physics::RigidBody3D* body = world.getRigidBody3D(entity);
            if (body == nullptr || body->bodyType != physics::BodyType::Dynamic) {
                return;
            }

            const CharacterControllerShape3D shape = describeCharacterControllerShape3D(entity);
            const float clampedSlope = std::clamp(character.maxSlopeAngle, 0.0f, 89.0f);
            const float minimumGroundDot = std::cos(clampedSlope * (kPi / 180.0f));
            bool grounded = false;

            for (int iteration = 0; iteration < 4; ++iteration) {
                physics::Vec3 normal;
                float penetration = 0.0f;
                if (!findCharacterPenetration3D(entity, shape, body->position, normal, penetration)) {
                    break;
                }

                body->position += normal * (penetration + 0.0005f);
                const float velocityIntoSurface = physics::Vec3::dot(body->velocity, normal);
                if (velocityIntoSurface < 0.0f) {
                    body->velocity -= normal * velocityIntoSurface;
                }

                if (normal.y >= minimumGroundDot) {
                    grounded = true;
                }
            }

            if (!grounded) {
                grounded = snapCharacterToGround3D(entity, character, *body, minimumGroundDot);
            }

            if (grounded && body->velocity.y < 0.0f) {
                body->velocity.y = 0.0f;
            }

            body->isGrounded = grounded;
            character.isGrounded = grounded;
        });
    }

private:
    static resource::AssetHandle makeAssetHandle(
        const std::string& path,
        int id,
        int refCount,
        bool loaded,
        resource::AssetType type) {
        resource::AssetHandle handle;
        handle.path = path;
        handle.type = type;
        handle.id = id;
        handle.refCount = refCount;
        handle.isLoaded = loaded;
        return handle;
    }

    static resource::AssetHandle makeAssetHandle(const TextureHandleView& view) {
        return makeAssetHandle(view.path, view.id, view.refCount, view.loaded, resource::AssetType::Texture);
    }

    static resource::AssetHandle makeAssetHandle(const AudioHandleView& view) {
        return makeAssetHandle(view.path, view.id, view.refCount, view.loaded, resource::AssetType::Audio);
    }

    static resource::AssetHandle makeAssetHandle(const MeshHandleView& view) {
        return makeAssetHandle(view.path, view.id, view.refCount, view.loaded, resource::AssetType::Mesh);
    }

    static resource::AssetHandle makeAssetHandle(const ShaderHandleView& view) {
        return makeAssetHandle(view.path, view.id, view.refCount, view.loaded, resource::AssetType::Shader);
    }

    static resource::AssetHandle makeAssetHandle(const MaterialHandleView& view) {
        return makeAssetHandle(view.path, view.id, view.refCount, view.loaded, resource::AssetType::Material);
    }

    template <typename TAsset>
    static std::string assetPathFromBinding(
        const resource::AssetHandle& handle,
        const std::shared_ptr<TAsset>& asset) {
        if (!handle.path.empty()) {
            return handle.path;
        }
        return asset ? asset->path : std::string();
    }

    std::string meshShaderPath(const MeshRenderer3D& mesh) const {
        return assetPathFromBinding(mesh.shaderHandle, mesh.shader);
    }

    void assignSpriteTextureBinding(SpriteRenderer2D& sprite, const std::string& texturePath) {
        if (texturePath.empty()) {
            sprite.texture.reset();
            sprite.textureHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        sprite.textureHandle = resources.loadTextureHandle(texturePath);
        sprite.texture = resources.loadTexture(sprite.textureHandle);
    }

    void assignSpriteTextureBinding(SpriteRenderer2D& sprite, const TextureHandleView& texture) {
        const resource::AssetHandle requested = makeAssetHandle(texture);
        if (requested.id == 0 && requested.path.empty()) {
            sprite.texture.reset();
            sprite.textureHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        sprite.textureHandle = resources.loadTextureHandle(requested);
        sprite.texture = resources.loadTexture(sprite.textureHandle);
    }

    void assignAudioSource2DBinding(AudioSource2DComponent& audio, const std::string& clipPath) {
        if (clipPath.empty()) {
            audio.clip.reset();
            audio.clipHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        audio.clipHandle = resources.loadAudioHandle(clipPath, false);
        audio.clip = resources.loadAudio(audio.clipHandle, false);
    }

    void assignAudioSource2DBinding(AudioSource2DComponent& audio, const AudioHandleView& clip) {
        const resource::AssetHandle requested = makeAssetHandle(clip);
        if (requested.id == 0 && requested.path.empty()) {
            audio.clip.reset();
            audio.clipHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        audio.clipHandle = resources.loadAudioHandle(requested, false);
        audio.clip = resources.loadAudio(audio.clipHandle, false);
    }

    void assignAudioSource3DBinding(AudioSource3DComponent& audio, const std::string& clipPath) {
        if (clipPath.empty()) {
            audio.clip.reset();
            audio.clipHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        audio.clipHandle = resources.loadAudioHandle(clipPath, true);
        audio.clip = resources.loadAudio(audio.clipHandle, true);
    }

    void assignAudioSource3DBinding(AudioSource3DComponent& audio, const AudioHandleView& clip) {
        const resource::AssetHandle requested = makeAssetHandle(clip);
        if (requested.id == 0 && requested.path.empty()) {
            audio.clip.reset();
            audio.clipHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        audio.clipHandle = resources.loadAudioHandle(requested, true);
        audio.clip = resources.loadAudio(audio.clipHandle, true);
    }

    void assignMeshBinding(MeshRenderer3D& mesh, const std::string& meshPath) {
        if (meshPath.empty()) {
            mesh.mesh.reset();
            mesh.meshHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        mesh.meshHandle = resources.loadMeshHandle(meshPath);
        mesh.mesh = resources.loadMesh(mesh.meshHandle);
    }

    void assignMeshBinding(MeshRenderer3D& mesh, const MeshHandleView& meshHandle) {
        const resource::AssetHandle requested = makeAssetHandle(meshHandle);
        if (requested.id == 0 && requested.path.empty()) {
            mesh.mesh.reset();
            mesh.meshHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        mesh.meshHandle = resources.loadMeshHandle(requested);
        mesh.mesh = resources.loadMesh(mesh.meshHandle);
    }

    void assignShaderBinding(MeshRenderer3D& mesh, const std::string& shaderPath) {
        if (shaderPath.empty()) {
            mesh.shader.reset();
            mesh.shaderHandle = resource::AssetHandle{};
            if (mesh.material) {
                mesh.material->shaderPath.clear();
            }
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        mesh.shaderHandle = resources.loadShaderHandle(shaderPath);
        mesh.shader = resources.loadShader(mesh.shaderHandle);
        const std::string resolvedShaderPath = assetPathFromBinding(mesh.shaderHandle, mesh.shader);
        if ((mesh.material || mesh.materialHandle.id != 0 || !mesh.materialHandle.path.empty()) && !resolvedShaderPath.empty()) {
            mesh.material = resources.loadMaterial(mesh.materialHandle, resolvedShaderPath);
            if (mesh.material) {
                mesh.material->shaderPath = resolvedShaderPath;
            }
        }
    }

    void assignShaderBinding(MeshRenderer3D& mesh, const ShaderHandleView& shader) {
        const resource::AssetHandle requested = makeAssetHandle(shader);
        if (requested.id == 0 && requested.path.empty()) {
            mesh.shader.reset();
            mesh.shaderHandle = resource::AssetHandle{};
            if (mesh.material) {
                mesh.material->shaderPath.clear();
            }
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        mesh.shaderHandle = resources.loadShaderHandle(requested);
        mesh.shader = resources.loadShader(mesh.shaderHandle);
        const std::string resolvedShaderPath = assetPathFromBinding(mesh.shaderHandle, mesh.shader);
        if ((mesh.material || mesh.materialHandle.id != 0 || !mesh.materialHandle.path.empty()) && !resolvedShaderPath.empty()) {
            mesh.material = resources.loadMaterial(mesh.materialHandle, resolvedShaderPath);
            if (mesh.material) {
                mesh.material->shaderPath = resolvedShaderPath;
            }
        }
    }

    void assignMaterialBinding(
        MeshRenderer3D& mesh,
        const std::string& materialPath,
        const std::string& shaderPath = "") {
        if (materialPath.empty()) {
            mesh.material.reset();
            mesh.materialHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        const std::string resolvedShaderPath = shaderPath.empty() ? meshShaderPath(mesh) : shaderPath;
        mesh.materialHandle = resources.loadMaterialHandle(materialPath, resolvedShaderPath);
        mesh.material = resources.loadMaterial(mesh.materialHandle, resolvedShaderPath);
        if (mesh.material && !resolvedShaderPath.empty()) {
            mesh.material->shaderPath = resolvedShaderPath;
        }
    }

    void assignMaterialBinding(MeshRenderer3D& mesh, const MaterialHandleView& material) {
        const resource::AssetHandle requested = makeAssetHandle(material);
        if (requested.id == 0 && requested.path.empty()) {
            mesh.material.reset();
            mesh.materialHandle = resource::AssetHandle{};
            return;
        }
        resource::ResourceManager& resources = resource::ResourceManager::getInstance();
        const std::string resolvedShaderPath = material.shaderPath.empty() ? meshShaderPath(mesh) : material.shaderPath;
        mesh.materialHandle = resources.loadMaterialHandle(requested, resolvedShaderPath);
        mesh.material = resources.loadMaterial(mesh.materialHandle, resolvedShaderPath);
        if (mesh.material && !resolvedShaderPath.empty()) {
            mesh.material->shaderPath = resolvedShaderPath;
        }
    }

    TextureHandleView makeTextureHandleView(
        const resource::AssetHandle& handle,
        const std::shared_ptr<resource::TextureAsset>& texture) const {
        if (handle.id == 0 && handle.path.empty() && !texture) {
            return TextureHandleView{};
        }
        TextureHandleView view;
        view.path = assetPathFromBinding(handle, texture);
        view.id = handle.id;
        view.refCount = handle.refCount;
        view.loaded = handle.isLoaded || static_cast<bool>(texture);
        if (texture) {
            view.width = texture->width;
            view.height = texture->height;
            view.channels = texture->channels;
            view.gpuId = static_cast<int>(texture->gpuId);
        }
        return view;
    }

    AudioHandleView makeAudioHandleView(
        const resource::AssetHandle& handle,
        const std::shared_ptr<resource::AudioAsset>& audio) const {
        if (handle.id == 0 && handle.path.empty() && !audio) {
            return AudioHandleView{};
        }
        AudioHandleView view;
        view.path = assetPathFromBinding(handle, audio);
        view.id = handle.id;
        view.refCount = handle.refCount;
        view.loaded = handle.isLoaded || static_cast<bool>(audio);
        if (audio) {
            view.duration = audio->duration;
            view.spatial = audio->isSpatial;
        }
        return view;
    }

    MeshHandleView makeMeshHandleView(
        const resource::AssetHandle& handle,
        const std::shared_ptr<resource::MeshAsset>& mesh) const {
        if (handle.id == 0 && handle.path.empty() && !mesh) {
            return MeshHandleView{};
        }
        MeshHandleView view;
        view.path = assetPathFromBinding(handle, mesh);
        view.id = handle.id;
        view.refCount = handle.refCount;
        view.loaded = handle.isLoaded || static_cast<bool>(mesh);
        if (mesh) {
            view.vertexCount = mesh->vertexCount;
            view.triangleCount = mesh->triangleCount;
            view.vbo = static_cast<int>(mesh->vbo);
            view.ebo = static_cast<int>(mesh->ebo);
        }
        return view;
    }

    ShaderHandleView makeShaderHandleView(
        const resource::AssetHandle& handle,
        const std::shared_ptr<resource::ShaderAsset>& shader) const {
        if (handle.id == 0 && handle.path.empty() && !shader) {
            return ShaderHandleView{};
        }
        ShaderHandleView view;
        view.path = assetPathFromBinding(handle, shader);
        view.id = handle.id;
        view.refCount = handle.refCount;
        view.loaded = handle.isLoaded || static_cast<bool>(shader);
        if (shader) {
            view.programId = static_cast<int>(shader->programId);
        }
        return view;
    }

    MaterialHandleView makeMaterialHandleView(
        const resource::AssetHandle& handle,
        const std::shared_ptr<resource::MaterialAsset>& material) const {
        if (handle.id == 0 && handle.path.empty() && !material) {
            return MaterialHandleView{};
        }
        MaterialHandleView view;
        view.path = assetPathFromBinding(handle, material);
        view.id = handle.id;
        view.refCount = handle.refCount;
        view.loaded = handle.isLoaded || static_cast<bool>(material);
        if (material) {
            view.shaderPath = material->shaderPath;
            view.propertyCount = static_cast<int>(material->properties.size());
        }
        return view;
    }

    Transform2D* ensureTransform2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Transform2D* transform = world.getTransform2D(entity);
        if (transform == nullptr) {
            transform = &world.addTransform2D(entity);
        }
        return transform;
    }

    physics::RigidBody2D* ensureBody2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::RigidBody2D* body = world.getRigidBody2D(entity);
        if (body == nullptr) {
            body = &world.addRigidBody2D(entity);
            if (const Transform2D* transform = world.getTransform2D(entity)) {
                body->position = transform->position;
            }
        }
        return body;
    }

    physics::BoxCollider2D* ensureBoxCollider2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::BoxCollider2D* collider = world.getBoxCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addBoxCollider2D(entity);
        }
        return collider;
    }

    physics::CircleCollider2D* ensureCircleCollider2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::CircleCollider2D* collider = world.getCircleCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addCircleCollider2D(entity);
        }
        return collider;
    }

    physics::CapsuleCollider2D* ensureCapsuleCollider2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::CapsuleCollider2D* collider = world.getCapsuleCollider2D(entity);
        if (collider == nullptr) {
            collider = &world.addCapsuleCollider2D(entity);
        }
        collider->radius = collider->radius <= 0.0f ? 0.01f : collider->radius;
        collider->height = std::max(collider->height, collider->radius * 2.0f);
        return collider;
    }

    Camera2DComponent* ensureCamera2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Camera2DComponent* camera = world.getCamera2D(entity);
        if (camera == nullptr) {
            camera = &world.addCamera2D(entity);
        }
        return camera;
    }

    AudioListener2DComponent* ensureAudioListener2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        AudioListener2DComponent* listener = world.getAudioListener2D(entity);
        if (listener == nullptr) {
            listener = &world.addAudioListener2D(entity);
        }
        return listener;
    }

    SpriteRenderer2D* ensureSprite2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity);
        if (sprite == nullptr) {
            sprite = &world.addSpriteRenderer2D(entity);
        }
        return sprite;
    }

    Tilemap2DComponent* ensureTilemap2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Tilemap2DComponent* tilemap = world.getTilemap2D(entity);
        if (tilemap == nullptr) {
            tilemap = &world.addTilemap2D(entity);
        }
        tilemap->columns = std::max(1, tilemap->columns);
        tilemap->rows = std::max(1, tilemap->rows);
        tilemap->tileSize.x = tilemap->tileSize.x <= 0.0f ? 1.0f : tilemap->tileSize.x;
        tilemap->tileSize.y = tilemap->tileSize.y <= 0.0f ? 1.0f : tilemap->tileSize.y;
        if (tilemap->cells.size() != static_cast<size_t>(tilemap->columns * tilemap->rows)) {
            tilemap->cells.assign(static_cast<size_t>(tilemap->columns * tilemap->rows), 0);
        }
        if (tilemap->paletteColors.empty()) {
            tilemap->paletteColors = std::vector<std::string>{"", "white"};
        }
        return tilemap;
    }

    Character2DComponent* ensureCharacter2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Character2DComponent* character = world.getCharacter2D(entity);
        if (character == nullptr) {
            character = &world.addCharacter2D(entity);
        }
        return character;
    }

    AudioSource2DComponent* ensureAudioSource2DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        AudioSource2DComponent* audio = world.getAudioSource2D(entity);
        if (audio == nullptr) {
            audio = &world.addAudioSource2D(entity);
        }
        return audio;
    }

    Transform3D* ensureTransform3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Transform3D* transform = world.getTransform3D(entity);
        if (transform == nullptr) {
            transform = &world.addTransform3D(entity);
        }
        return transform;
    }

    physics::RigidBody3D* ensureBody3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::RigidBody3D* body = world.getRigidBody3D(entity);
        if (body == nullptr) {
            body = &world.addRigidBody3D(entity);
            if (const Transform3D* transform = world.getTransform3D(entity)) {
                body->position = transform->position;
                body->rotation = transform->rotation;
            }
        }
        return body;
    }

    physics::BoxCollider3D* ensureBoxCollider3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::BoxCollider3D* collider = world.getBoxCollider3D(entity);
        if (collider == nullptr) {
            collider = &world.addBoxCollider3D(entity);
        }
        return collider;
    }

    physics::SphereCollider3D* ensureSphereCollider3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        physics::SphereCollider3D* collider = world.getSphereCollider3D(entity);
        if (collider == nullptr) {
            collider = &world.addSphereCollider3D(entity);
        }
        return collider;
    }

    Camera3DComponent* ensureCamera3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Camera3DComponent* camera = world.getCamera3D(entity);
        if (camera == nullptr) {
            camera = &world.addCamera3D(entity);
        }
        return camera;
    }

    AudioListener3DComponent* ensureAudioListener3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        AudioListener3DComponent* listener = world.getAudioListener3D(entity);
        if (listener == nullptr) {
            listener = &world.addAudioListener3D(entity);
        }
        return listener;
    }

    MeshRenderer3D* ensureMesh3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        MeshRenderer3D* mesh = world.getMeshRenderer3D(entity);
        if (mesh == nullptr) {
            mesh = &world.addMeshRenderer3D(entity);
        }
        return mesh;
    }

    Character3DComponent* ensureCharacter3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        Character3DComponent* character = world.getCharacter3D(entity);
        if (character == nullptr) {
            character = &world.addCharacter3D(entity);
        }
        return character;
    }

    AudioSource3DComponent* ensureAudioSource3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        AudioSource3DComponent* audio = world.getAudioSource3D(entity);
        if (audio == nullptr) {
            audio = &world.addAudioSource3D(entity);
        }
        return audio;
    }

    PointLight3DComponent* ensurePointLight3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        PointLight3DComponent* light = world.getPointLight3D(entity);
        if (light == nullptr) {
            light = &world.addPointLight3D(entity);
        }
        return light;
    }

    DirectionalLight3DComponent* ensureDirectionalLight3DViewComponent(EntityId entity) {
        if (!world.isAlive(entity)) {
            return nullptr;
        }
        DirectionalLight3DComponent* light = world.getDirectionalLight3D(entity);
        if (light == nullptr) {
            light = &world.addDirectionalLight3D(entity);
        }
        return light;
    }

    std::shared_ptr<resource::MaterialAsset> ensureMaterialAsset(const std::string& materialPath) const {
        if (materialPath.empty()) {
            return nullptr;
        }
        return resource::ResourceManager::getInstance().loadMaterial(materialPath);
    }

    std::shared_ptr<resource::MaterialAsset> findMaterialAsset(const std::string& materialPath) const {
        if (materialPath.empty() || !resource::ResourceManager::getInstance().hasMaterial(materialPath)) {
            return nullptr;
        }
        return resource::ResourceManager::getInstance().loadMaterial(materialPath);
    }

    struct StreamInstance {
        std::string source;
        std::vector<EntityId> entities;
    };

    static uint64_t sceneStreamEntityKey(EntityId entity) {
        return (static_cast<uint64_t>(entity.generation) << 32) | static_cast<uint64_t>(entity.index);
    }

    static bool containsEntity(const std::vector<EntityId>& entities, EntityId entity) {
        return std::find(entities.begin(), entities.end(), entity) != entities.end();
    }

    std::string sceneStreamOwner(EntityId entity) const {
        if (!world.isAlive(entity)) {
            return "";
        }

        auto it = streamEntityOwners.find(sceneStreamEntityKey(entity));
        return it == streamEntityOwners.end() ? std::string() : it->second;
    }

    void setSceneStreamOwner(EntityId entity, const std::string& owner) {
        if (!entity) {
            return;
        }

        const uint64_t key = sceneStreamEntityKey(entity);
        if (owner.empty()) {
            streamEntityOwners.erase(key);
        } else {
            streamEntityOwners[key] = owner;
        }
    }

    void clearSceneStreamOwner(EntityId entity) {
        setSceneStreamOwner(entity, "");
    }

    std::vector<EntityId> filterSceneStreamOwnedEntities(
        const std::string& owner,
        const std::vector<EntityId>& entities) const {
        std::vector<EntityId> filtered;
        filtered.reserve(entities.size());
        for (EntityId entity : entities) {
            if (!world.isAlive(entity)) {
                continue;
            }

            const std::string existingOwner = sceneStreamOwner(entity);
            if (!existingOwner.empty() && existingOwner != owner) {
                continue;
            }

            if (!containsEntity(filtered, entity)) {
                filtered.push_back(entity);
            }
        }
        return filtered;
    }

    void markSceneStreamOwnedEntities(const std::string& owner, const std::vector<EntityId>& entities) {
        for (EntityId entity : entities) {
            if (world.isAlive(entity)) {
                setSceneStreamOwner(entity, owner);
            }
        }
    }

    void adoptSceneStreamBranchIfUnowned(EntityId entity, const std::string& owner) {
        if (owner.empty() || !world.isAlive(entity)) {
            return;
        }

        const std::string existingOwner = sceneStreamOwner(entity);
        if (!existingOwner.empty() && existingOwner != owner) {
            return;
        }

        if (existingOwner.empty()) {
            setSceneStreamOwner(entity, owner);
            auto stream = loadedStreams.find(owner);
            if (stream != loadedStreams.end() && !containsEntity(stream->second.entities, entity)) {
                stream->second.entities.push_back(entity);
            }
        }

        for (EntityId child : world.childrenOf(entity)) {
            adoptSceneStreamBranchIfUnowned(child, owner);
        }
    }

    std::vector<EntityId> collectNewEntities(const std::vector<EntityId>& before) const {
        std::vector<EntityId> created;
        for (EntityId entity : world.aliveEntities()) {
            if (!containsEntity(before, entity)) {
                created.push_back(entity);
            }
        }
        return created;
    }

    void collectSceneStreamEntitiesRecursive(
        const std::string& owner,
        EntityId entity,
        std::unordered_set<uint64_t>& visited,
        std::vector<EntityId>& entities) const {
        if (!world.isAlive(entity)) {
            return;
        }

        const std::string existingOwner = sceneStreamOwner(entity);
        if (!existingOwner.empty() && existingOwner != owner) {
            return;
        }

        const uint64_t key = sceneStreamEntityKey(entity);
        if (!visited.insert(key).second) {
            return;
        }

        entities.push_back(entity);
        for (EntityId child : world.childrenOf(entity)) {
            collectSceneStreamEntitiesRecursive(owner, child, visited, entities);
        }
    }

    std::vector<EntityId> collectSceneStreamTrackedEntities(
        const std::string& owner,
        const StreamInstance& instance) const {
        std::vector<EntityId> entities;
        std::unordered_set<uint64_t> visited;

        for (EntityId entity : instance.entities) {
            if (!world.isAlive(entity) || sceneStreamOwner(entity) != owner) {
                continue;
            }

            const EntityId parent = world.parentOf(entity);
            if (parent && sceneStreamOwner(parent) == owner) {
                continue;
            }

            collectSceneStreamEntitiesRecursive(owner, entity, visited, entities);
        }

        return entities;
    }

    int hierarchyDepth(EntityId entity) const {
        int depth = 0;
        EntityId cursor = world.parentOf(entity);
        while (cursor) {
            ++depth;
            cursor = world.parentOf(cursor);
        }
        return depth;
    }

    bool loaded = false;
    float accumulator = 0.0f;
    float lastFrameDelta = 0.0f;
    uint64_t frameCount = 0;
    uint64_t fixedStepCount = 0;
    uint64_t droppedStepFrames = 0;
    int lastFixedStepsThisFrame = 0;
    EntityId inspectorEntityTarget = EntityId::invalid();
    std::string inspectorMaterialTarget;
    std::unordered_map<std::string, PrefabBuilder> prefabs;
    std::unordered_map<std::string, StreamBuilder> sceneStreams;
    std::unordered_map<std::string, StreamInstance> loadedStreams;
    std::unordered_map<uint64_t, std::string> streamEntityOwners;
};

} // namespace game
} // namespace zenith

#endif // ZENITH_SCENE_H
