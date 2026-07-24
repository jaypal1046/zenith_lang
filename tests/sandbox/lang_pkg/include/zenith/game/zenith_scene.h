#ifndef ZENITH_SCENE_H
#define ZENITH_SCENE_H

#include "zenith_physics.h"
#include "zenith_world.h"
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <unordered_map>

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

struct Sprite2DView {
    FieldRef<float> width;
    FieldRef<float> height;
    FieldRef<float> anchorX;
    FieldRef<float> anchorY;
    FieldRef<std::string> color;
    FieldRef<std::string> texturePath;
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
    FieldRef<std::string> color;
};

struct AudioSource2DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<std::string> clipPath;
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
    FieldRef<bool> visible;
    FieldRef<bool> castShadows;
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
    FieldRef<bool> isGrounded;
    FieldRef<bool> useGravity;
    FieldRef<std::string> meshPath;
    FieldRef<std::string> shaderPath;
    FieldRef<std::string> materialPath;
};

struct AudioSource3DView {
    FieldRef<float> x;
    FieldRef<float> y;
    FieldRef<float> z;
    FieldRef<std::string> clipPath;
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
            physics.step(dt);
            syncFromPhysics();
        }

        onPostPhysics(dt);
    }

    void render(Canvas& canvas) {
        load();
        onDraw(canvas, interpolationAlpha());
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

    void registerPrefab(std::string prefabName, PrefabBuilder builder) {
        prefabs[std::move(prefabName)] = std::move(builder);
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
                    if (sprite->texture) {
                        return sprite->texture->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->texture = resource::ResourceManager::getInstance().loadTexture(value);
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
                    if (sprite->texture) {
                        return sprite->texture->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (SpriteRenderer2D* sprite = ensureSprite2DViewComponent(entity)) {
                    sprite->texture = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadTexture(value);
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
                    if (audio->clip) {
                        return audio->clip->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (AudioSource2DComponent* audio = ensureAudioSource2DViewComponent(entity)) {
                    audio->clip = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadAudio(value, false);
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

    bool overlaps2D(EntityId first, EntityId second) const {
        return world.overlaps2D(first, second);
    }

    bool containsPoint2D(EntityId entity, float x, float y) const {
        return world.containsPoint2D(entity, physics::Vec2(x, y));
    }

    EntityId spawnCharacter2D(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        SpriteRenderer2D& sprite = world.addSpriteRenderer2D(entity);
        sprite.size = physics::Vec2(w, h);
        sprite.tintColor = color;
        if (!texturePath.empty()) {
            sprite.texture = resource::ResourceManager::getInstance().loadTexture(texturePath);
        }

        physics::RigidBody2D& body = world.addRigidBody2D(entity);
        body.position = transform.position;

        world.addCharacter2D(entity);
        return entity;
    }

    EntityId spawnAudioSource2D(std::string name, std::string clipPath, float x, float y, bool playOnAwake) {
        EntityId entity = world.createEntity(name);
        Transform2D& transform = world.addTransform2D(entity);
        transform.position = physics::Vec2(x, y);

        AudioSource2DComponent& audio = world.addAudioSource2D(entity);
        if (!clipPath.empty()) {
            audio.clip = resource::ResourceManager::getInstance().loadAudio(clipPath, false);
        }
        audio.playOnAwake = playOnAwake;
        audio.isPlaying = playOnAwake;
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
            mesh.mesh = resource::ResourceManager::getInstance().loadMesh(meshPath);
        }
        if (!shaderPath.empty()) {
            mesh.shader = resource::ResourceManager::getInstance().loadShader(shaderPath);
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
                mesh->material = resource::ResourceManager::getInstance().loadMaterial(materialPath);
            }
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

    std::string materialColorProperty(std::string materialPath, std::string propertyName) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->color(propertyName) : "";
    }

    int materialPropertyCount(std::string materialPath) {
        std::shared_ptr<resource::MaterialAsset> material = ensureMaterialAsset(materialPath);
        return material ? material->propertyCount() : 0;
    }

    bool setMeshMaterial(EntityId entity, std::string materialPath) {
        if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
            mesh->material = materialPath.empty()
                ? nullptr
                : resource::ResourceManager::getInstance().loadMaterial(materialPath);
            return true;
        }
        return false;
    }

    std::string meshMaterialPath(EntityId entity) {
        if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
            if (mesh->material) {
                return mesh->material->path;
            }
        }
        return "";
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
                    if (mesh->mesh) {
                        return mesh->mesh->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->mesh = resource::ResourceManager::getInstance().loadMesh(value);
                }
            });

        view.shaderPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    if (mesh->shader) {
                        return mesh->shader->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->shader = resource::ResourceManager::getInstance().loadShader(value);
                }
            });

        view.materialPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    if (mesh->material) {
                        return mesh->material->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->material = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadMaterial(value);
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

        view.isGrounded = FieldRef<bool>(
            [this, entity]() {
                if (const Character3DComponent* character = world.getCharacter3D(entity)) {
                    return character->isGrounded;
                }
                return false;
            },
            [this, entity](const bool& value) {
                if (Character3DComponent* character = ensureCharacter3DViewComponent(entity)) {
                    character->isGrounded = value;
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
                    if (mesh->mesh) {
                        return mesh->mesh->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->mesh = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadMesh(value);
                }
            });

        view.shaderPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    if (mesh->shader) {
                        return mesh->shader->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->shader = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadShader(value);
                }
            });

        view.materialPath = FieldRef<std::string>(
            [this, entity]() {
                if (const MeshRenderer3D* mesh = world.getMeshRenderer3D(entity)) {
                    if (mesh->material) {
                        return mesh->material->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (MeshRenderer3D* mesh = ensureMesh3DViewComponent(entity)) {
                    mesh->material = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadMaterial(value);
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
                    if (audio->clip) {
                        return audio->clip->path;
                    }
                }
                return std::string();
            },
            [this, entity](const std::string& value) {
                if (AudioSource3DComponent* audio = ensureAudioSource3DViewComponent(entity)) {
                    audio->clip = value.empty() ? nullptr : resource::ResourceManager::getInstance().loadAudio(value, true);
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
            audio.clip = resource::ResourceManager::getInstance().loadAudio(clipPath, true);
        }
        audio.playOnAwake = playOnAwake;
        audio.isPlaying = playOnAwake;
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

        for (EntityId entity : world.rigidBodyEntities2D()) {
            if (physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
                physics.registerBody(body);
            }
        }

        for (EntityId entity : world.rigidBodyEntities3D()) {
            if (physics::RigidBody3D* body = world.getRigidBody3D(entity)) {
                physics.registerBody(body);
            }
        }
    }

    void syncFromPhysics() {
        world.syncTransformsFromPhysics2D();
        world.syncTransformsFromPhysics3D();
    }

private:
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

    bool loaded = false;
    float accumulator = 0.0f;
    float lastFrameDelta = 0.0f;
    uint64_t frameCount = 0;
    uint64_t fixedStepCount = 0;
    uint64_t droppedStepFrames = 0;
    int lastFixedStepsThisFrame = 0;
    std::unordered_map<std::string, PrefabBuilder> prefabs;
};

} // namespace game
} // namespace zenith

#endif // ZENITH_SCENE_H
