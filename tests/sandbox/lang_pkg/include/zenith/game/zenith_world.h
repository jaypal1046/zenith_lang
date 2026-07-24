#ifndef ZENITH_WORLD_H
#define ZENITH_WORLD_H

#include "zenith_physics.h"
#include "zenith_resource.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace zenith {
namespace game {

struct EntityId {
    uint32_t index = 0;
    uint32_t generation = 0;

    static EntityId invalid() {
        return {};
    }

    explicit operator bool() const {
        return index != 0;
    }
};

inline bool operator==(const EntityId& lhs, const EntityId& rhs) {
    return lhs.index == rhs.index && lhs.generation == rhs.generation;
}

inline bool operator!=(const EntityId& lhs, const EntityId& rhs) {
    return !(lhs == rhs);
}

struct NameComponent {
    std::string value;
};

struct TagComponent {
    std::string value;
};

struct Transform2D {
    physics::Vec2 position;
    float rotation = 0.0f;
    physics::Vec2 scale = physics::Vec2(1.0f, 1.0f);
};

struct Transform3D {
    physics::Vec3 position;
    physics::Vec3 rotation;
    physics::Vec3 scale = physics::Vec3(1.0f, 1.0f, 1.0f);
};

struct RelationshipComponent {
    EntityId parent = EntityId::invalid();
    std::vector<EntityId> children;
};

struct SpriteRenderer2D {
    std::shared_ptr<resource::TextureAsset> texture;
    physics::Vec2 size = physics::Vec2(1.0f, 1.0f);
    physics::Vec2 anchor = physics::Vec2(0.5f, 0.5f);
    std::string tintColor = "white";
    int sortOrder = 0;
    bool visible = true;
};

struct Character2DComponent {
    float moveSpeed = 10.0f;
    float jumpForce = 12.0f;
    bool isGrounded = false;
    bool facingRight = true;
};

struct AudioSource2DComponent {
    std::shared_ptr<resource::AudioAsset> clip;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnAwake = false;
    bool isPlaying = false;
};

struct MeshRenderer3D {
    std::shared_ptr<resource::MeshAsset> mesh;
    std::shared_ptr<resource::ShaderAsset> shader;
    std::shared_ptr<resource::MaterialAsset> material;
    bool visible = true;
    bool castShadows = true;
};

struct Character3DComponent {
    float moveSpeed = 6.0f;
    float turnSpeed = 4.0f;
    bool isGrounded = false;
    bool useGravity = true;
};

struct AudioSource3DComponent {
    std::shared_ptr<resource::AudioAsset> clip;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnAwake = false;
    bool isPlaying = false;
    float minDistance = 1.0f;
    float maxDistance = 20.0f;
};

struct PointLight3DComponent {
    float intensity = 1.0f;
    float range = 10.0f;
    std::string color = "white";
    bool enabled = true;
};

struct DirectionalLight3DComponent {
    physics::Vec3 direction = physics::Vec3(0.0f, -1.0f, 0.0f);
    float intensity = 1.0f;
    std::string color = "white";
    bool castShadows = true;
    bool enabled = true;
};

struct Camera2DComponent {
    float zoom = 1.0f;
    bool primary = false;
    physics::Vec2 viewportOrigin;
    physics::Vec2 viewportSize = physics::Vec2(1.0f, 1.0f);
};

struct AudioListener2DComponent {
    float gain = 1.0f;
    bool primary = false;
    bool enabled = true;
};

struct Camera3DComponent {
    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    bool primary = false;
    physics::Vec2 viewportOrigin;
    physics::Vec2 viewportSize = physics::Vec2(1.0f, 1.0f);
};

struct AudioListener3DComponent {
    float gain = 1.0f;
    bool primary = false;
    bool enabled = true;
};

template <typename T>
class ComponentPool {
public:
    T& assign(EntityId entity, const T& value = T()) {
        size_t slot = entityToSlot(entity);
        ensureSize(slot + 1);
        data[slot] = value;
        active[slot] = 1;
        return data[slot];
    }

    T& assign(EntityId entity, T&& value) {
        size_t slot = entityToSlot(entity);
        ensureSize(slot + 1);
        data[slot] = std::move(value);
        active[slot] = 1;
        return data[slot];
    }

    bool has(EntityId entity) const {
        size_t slot = entityToSlot(entity);
        return hasSlot(slot);
    }

    bool hasSlot(size_t slot) const {
        return slot < active.size() && active[slot] != 0;
    }

    T* get(EntityId entity) {
        size_t slot = entityToSlot(entity);
        if (!hasSlot(slot)) {
            return nullptr;
        }
        return &data[slot];
    }

    const T* get(EntityId entity) const {
        size_t slot = entityToSlot(entity);
        if (!hasSlot(slot)) {
            return nullptr;
        }
        return &data[slot];
    }

    void remove(EntityId entity) {
        size_t slot = entityToSlot(entity);
        if (slot < active.size()) {
            active[slot] = 0;
        }
    }

private:
    static size_t entityToSlot(EntityId entity) {
        return static_cast<size_t>(entity.index - 1);
    }

    void ensureSize(size_t size) {
        if (data.size() < size) {
            data.resize(size);
            active.resize(size, 0);
        }
    }

    std::vector<T> data;
    std::vector<uint8_t> active;
};

class World {
public:
    EntityId createEntity() {
        if (!freeList.empty()) {
            uint32_t index = freeList.back();
            freeList.pop_back();
            size_t slot = static_cast<size_t>(index - 1);
            alive[slot] = 1;
            ++liveCount;
            return EntityId{index, generations[slot]};
        }

        generations.push_back(1);
        alive.push_back(1);
        ++liveCount;
        return EntityId{
            static_cast<uint32_t>(generations.size()),
            generations.back()
        };
    }

    EntityId createEntity(const std::string& name) {
        EntityId entity = createEntity();
        addName(entity, name);
        return entity;
    }

    bool destroyEntity(EntityId entity) {
        if (!isAlive(entity)) {
            return false;
        }

        detachFromParent(entity);
        orphanChildren(entity);

        names.remove(entity);
        tags.remove(entity);
        transforms2D.remove(entity);
        transforms3D.remove(entity);
        rigidBodies2D.remove(entity);
        rigidBodies3D.remove(entity);
        boxColliders2D.remove(entity);
        circleColliders2D.remove(entity);
        spriteRenderers2D.remove(entity);
        characters2D.remove(entity);
        audioSources2D.remove(entity);
        meshRenderers3D.remove(entity);
        characters3D.remove(entity);
        audioSources3D.remove(entity);
        pointLights3D.remove(entity);
        directionalLights3D.remove(entity);
        cameras2D.remove(entity);
        audioListeners2D.remove(entity);
        cameras3D.remove(entity);
        audioListeners3D.remove(entity);
        relationships.remove(entity);

        size_t slot = entityToSlot(entity);
        alive[slot] = 0;
        ++generations[slot];
        freeList.push_back(entity.index);
        --liveCount;
        return true;
    }

    bool isAlive(EntityId entity) const {
        if (entity.index == 0) {
            return false;
        }

        size_t slot = entityToSlot(entity);
        return slot < alive.size() &&
               alive[slot] != 0 &&
               generations[slot] == entity.generation;
    }

    size_t entityCount() const {
        return liveCount;
    }

    std::vector<EntityId> aliveEntities() const {
        std::vector<EntityId> entities;
        entities.reserve(liveCount);
        for (size_t i = 0; i < alive.size(); ++i) {
            if (alive[i] == 0) {
                continue;
            }
            entities.push_back(EntityId{
                static_cast<uint32_t>(i + 1),
                generations[i]
            });
        }
        return entities;
    }

    NameComponent& addName(EntityId entity, std::string value) {
        requireAlive(entity);
        return names.assign(entity, NameComponent{std::move(value)});
    }

    TagComponent& addTag(EntityId entity, std::string value) {
        requireAlive(entity);
        return tags.assign(entity, TagComponent{std::move(value)});
    }

    Transform2D& addTransform2D(EntityId entity, const Transform2D& value = Transform2D()) {
        requireAlive(entity);
        return transforms2D.assign(entity, value);
    }

    Transform3D& addTransform3D(EntityId entity, const Transform3D& value = Transform3D()) {
        requireAlive(entity);
        return transforms3D.assign(entity, value);
    }

    physics::RigidBody2D& addRigidBody2D(EntityId entity, const physics::RigidBody2D& value = physics::RigidBody2D()) {
        requireAlive(entity);
        return rigidBodies2D.assign(entity, value);
    }

    physics::RigidBody3D& addRigidBody3D(EntityId entity, const physics::RigidBody3D& value = physics::RigidBody3D()) {
        requireAlive(entity);
        return rigidBodies3D.assign(entity, value);
    }

    physics::BoxCollider2D& addBoxCollider2D(EntityId entity, const physics::BoxCollider2D& value = physics::BoxCollider2D()) {
        requireAlive(entity);
        return boxColliders2D.assign(entity, value);
    }

    physics::CircleCollider2D& addCircleCollider2D(EntityId entity, const physics::CircleCollider2D& value = physics::CircleCollider2D()) {
        requireAlive(entity);
        return circleColliders2D.assign(entity, value);
    }

    SpriteRenderer2D& addSpriteRenderer2D(EntityId entity, const SpriteRenderer2D& value = SpriteRenderer2D()) {
        requireAlive(entity);
        return spriteRenderers2D.assign(entity, value);
    }

    Character2DComponent& addCharacter2D(EntityId entity, const Character2DComponent& value = Character2DComponent()) {
        requireAlive(entity);
        return characters2D.assign(entity, value);
    }

    AudioSource2DComponent& addAudioSource2D(EntityId entity, const AudioSource2DComponent& value = AudioSource2DComponent()) {
        requireAlive(entity);
        return audioSources2D.assign(entity, value);
    }

    MeshRenderer3D& addMeshRenderer3D(EntityId entity, const MeshRenderer3D& value = MeshRenderer3D()) {
        requireAlive(entity);
        return meshRenderers3D.assign(entity, value);
    }

    Character3DComponent& addCharacter3D(EntityId entity, const Character3DComponent& value = Character3DComponent()) {
        requireAlive(entity);
        return characters3D.assign(entity, value);
    }

    AudioSource3DComponent& addAudioSource3D(EntityId entity, const AudioSource3DComponent& value = AudioSource3DComponent()) {
        requireAlive(entity);
        return audioSources3D.assign(entity, value);
    }

    PointLight3DComponent& addPointLight3D(EntityId entity, const PointLight3DComponent& value = PointLight3DComponent()) {
        requireAlive(entity);
        return pointLights3D.assign(entity, value);
    }

    DirectionalLight3DComponent& addDirectionalLight3D(EntityId entity, const DirectionalLight3DComponent& value = DirectionalLight3DComponent()) {
        requireAlive(entity);
        return directionalLights3D.assign(entity, value);
    }

    Camera2DComponent& addCamera2D(EntityId entity, const Camera2DComponent& value = Camera2DComponent()) {
        requireAlive(entity);
        return cameras2D.assign(entity, value);
    }

    AudioListener2DComponent& addAudioListener2D(EntityId entity, const AudioListener2DComponent& value = AudioListener2DComponent()) {
        requireAlive(entity);
        return audioListeners2D.assign(entity, value);
    }

    Camera3DComponent& addCamera3D(EntityId entity, const Camera3DComponent& value = Camera3DComponent()) {
        requireAlive(entity);
        return cameras3D.assign(entity, value);
    }

    AudioListener3DComponent& addAudioListener3D(EntityId entity, const AudioListener3DComponent& value = AudioListener3DComponent()) {
        requireAlive(entity);
        return audioListeners3D.assign(entity, value);
    }

    RelationshipComponent& addRelationship(EntityId entity) {
        requireAlive(entity);
        return ensureRelationship(entity);
    }

    bool hasName(EntityId entity) const { return names.has(entity); }
    bool hasTag(EntityId entity) const { return tags.has(entity); }
    bool hasTransform2D(EntityId entity) const { return transforms2D.has(entity); }
    bool hasTransform3D(EntityId entity) const { return transforms3D.has(entity); }
    bool hasRigidBody2D(EntityId entity) const { return rigidBodies2D.has(entity); }
    bool hasRigidBody3D(EntityId entity) const { return rigidBodies3D.has(entity); }
    bool hasBoxCollider2D(EntityId entity) const { return boxColliders2D.has(entity); }
    bool hasCircleCollider2D(EntityId entity) const { return circleColliders2D.has(entity); }
    bool hasSpriteRenderer2D(EntityId entity) const { return spriteRenderers2D.has(entity); }
    bool hasCharacter2D(EntityId entity) const { return characters2D.has(entity); }
    bool hasAudioSource2D(EntityId entity) const { return audioSources2D.has(entity); }
    bool hasMeshRenderer3D(EntityId entity) const { return meshRenderers3D.has(entity); }
    bool hasCharacter3D(EntityId entity) const { return characters3D.has(entity); }
    bool hasAudioSource3D(EntityId entity) const { return audioSources3D.has(entity); }
    bool hasPointLight3D(EntityId entity) const { return pointLights3D.has(entity); }
    bool hasDirectionalLight3D(EntityId entity) const { return directionalLights3D.has(entity); }
    bool hasCamera2D(EntityId entity) const { return cameras2D.has(entity); }
    bool hasAudioListener2D(EntityId entity) const { return audioListeners2D.has(entity); }
    bool hasCamera3D(EntityId entity) const { return cameras3D.has(entity); }
    bool hasAudioListener3D(EntityId entity) const { return audioListeners3D.has(entity); }
    bool hasRelationship(EntityId entity) const { return relationships.has(entity); }

    NameComponent* getName(EntityId entity) { return names.get(entity); }
    TagComponent* getTag(EntityId entity) { return tags.get(entity); }
    Transform2D* getTransform2D(EntityId entity) { return transforms2D.get(entity); }
    Transform3D* getTransform3D(EntityId entity) { return transforms3D.get(entity); }
    physics::RigidBody2D* getRigidBody2D(EntityId entity) { return rigidBodies2D.get(entity); }
    physics::RigidBody3D* getRigidBody3D(EntityId entity) { return rigidBodies3D.get(entity); }
    physics::BoxCollider2D* getBoxCollider2D(EntityId entity) { return boxColliders2D.get(entity); }
    physics::CircleCollider2D* getCircleCollider2D(EntityId entity) { return circleColliders2D.get(entity); }
    SpriteRenderer2D* getSpriteRenderer2D(EntityId entity) { return spriteRenderers2D.get(entity); }
    Character2DComponent* getCharacter2D(EntityId entity) { return characters2D.get(entity); }
    AudioSource2DComponent* getAudioSource2D(EntityId entity) { return audioSources2D.get(entity); }
    MeshRenderer3D* getMeshRenderer3D(EntityId entity) { return meshRenderers3D.get(entity); }
    Character3DComponent* getCharacter3D(EntityId entity) { return characters3D.get(entity); }
    AudioSource3DComponent* getAudioSource3D(EntityId entity) { return audioSources3D.get(entity); }
    PointLight3DComponent* getPointLight3D(EntityId entity) { return pointLights3D.get(entity); }
    DirectionalLight3DComponent* getDirectionalLight3D(EntityId entity) { return directionalLights3D.get(entity); }
    Camera2DComponent* getCamera2D(EntityId entity) { return cameras2D.get(entity); }
    AudioListener2DComponent* getAudioListener2D(EntityId entity) { return audioListeners2D.get(entity); }
    Camera3DComponent* getCamera3D(EntityId entity) { return cameras3D.get(entity); }
    AudioListener3DComponent* getAudioListener3D(EntityId entity) { return audioListeners3D.get(entity); }
    RelationshipComponent* getRelationship(EntityId entity) { return relationships.get(entity); }

    const NameComponent* getName(EntityId entity) const { return names.get(entity); }
    const TagComponent* getTag(EntityId entity) const { return tags.get(entity); }
    const Transform2D* getTransform2D(EntityId entity) const { return transforms2D.get(entity); }
    const Transform3D* getTransform3D(EntityId entity) const { return transforms3D.get(entity); }
    const physics::RigidBody2D* getRigidBody2D(EntityId entity) const { return rigidBodies2D.get(entity); }
    const physics::RigidBody3D* getRigidBody3D(EntityId entity) const { return rigidBodies3D.get(entity); }
    const physics::BoxCollider2D* getBoxCollider2D(EntityId entity) const { return boxColliders2D.get(entity); }
    const physics::CircleCollider2D* getCircleCollider2D(EntityId entity) const { return circleColliders2D.get(entity); }
    const SpriteRenderer2D* getSpriteRenderer2D(EntityId entity) const { return spriteRenderers2D.get(entity); }
    const Character2DComponent* getCharacter2D(EntityId entity) const { return characters2D.get(entity); }
    const AudioSource2DComponent* getAudioSource2D(EntityId entity) const { return audioSources2D.get(entity); }
    const MeshRenderer3D* getMeshRenderer3D(EntityId entity) const { return meshRenderers3D.get(entity); }
    const Character3DComponent* getCharacter3D(EntityId entity) const { return characters3D.get(entity); }
    const AudioSource3DComponent* getAudioSource3D(EntityId entity) const { return audioSources3D.get(entity); }
    const PointLight3DComponent* getPointLight3D(EntityId entity) const { return pointLights3D.get(entity); }
    const DirectionalLight3DComponent* getDirectionalLight3D(EntityId entity) const { return directionalLights3D.get(entity); }
    const Camera2DComponent* getCamera2D(EntityId entity) const { return cameras2D.get(entity); }
    const AudioListener2DComponent* getAudioListener2D(EntityId entity) const { return audioListeners2D.get(entity); }
    const Camera3DComponent* getCamera3D(EntityId entity) const { return cameras3D.get(entity); }
    const AudioListener3DComponent* getAudioListener3D(EntityId entity) const { return audioListeners3D.get(entity); }
    const RelationshipComponent* getRelationship(EntityId entity) const { return relationships.get(entity); }

    void removeName(EntityId entity) { names.remove(entity); }
    void removeTag(EntityId entity) { tags.remove(entity); }
    void removeTransform2D(EntityId entity) { transforms2D.remove(entity); }
    void removeTransform3D(EntityId entity) { transforms3D.remove(entity); }
    void removeRigidBody2D(EntityId entity) { rigidBodies2D.remove(entity); }
    void removeRigidBody3D(EntityId entity) { rigidBodies3D.remove(entity); }
    void removeBoxCollider2D(EntityId entity) { boxColliders2D.remove(entity); }
    void removeCircleCollider2D(EntityId entity) { circleColliders2D.remove(entity); }
    void removeSpriteRenderer2D(EntityId entity) { spriteRenderers2D.remove(entity); }
    void removeCharacter2D(EntityId entity) { characters2D.remove(entity); }
    void removeAudioSource2D(EntityId entity) { audioSources2D.remove(entity); }
    void removeMeshRenderer3D(EntityId entity) { meshRenderers3D.remove(entity); }
    void removeCharacter3D(EntityId entity) { characters3D.remove(entity); }
    void removeAudioSource3D(EntityId entity) { audioSources3D.remove(entity); }
    void removePointLight3D(EntityId entity) { pointLights3D.remove(entity); }
    void removeDirectionalLight3D(EntityId entity) { directionalLights3D.remove(entity); }
    void removeCamera2D(EntityId entity) { cameras2D.remove(entity); }
    void removeAudioListener2D(EntityId entity) { audioListeners2D.remove(entity); }
    void removeCamera3D(EntityId entity) { cameras3D.remove(entity); }
    void removeAudioListener3D(EntityId entity) { audioListeners3D.remove(entity); }

    void removeRelationship(EntityId entity) {
        if (!isAlive(entity)) {
            return;
        }
        detachFromParent(entity);
        orphanChildren(entity);
        relationships.remove(entity);
    }

    bool setParent(EntityId child, EntityId parent) {
        if (!isAlive(child) || !isAlive(parent) || child == parent || wouldCreateCycle(child, parent)) {
            return false;
        }

        ensureRelationship(child);
        ensureRelationship(parent);

        RelationshipComponent* childRelationship = relationships.get(child);
        RelationshipComponent* parentRelationship = relationships.get(parent);
        if (childRelationship == nullptr || parentRelationship == nullptr) {
            return false;
        }

        if (childRelationship->parent == parent) {
            return true;
        }

        detachFromParent(child);
        childRelationship = relationships.get(child);
        parentRelationship = relationships.get(parent);
        if (childRelationship == nullptr || parentRelationship == nullptr) {
            return false;
        }

        childRelationship->parent = parent;
        if (std::find(parentRelationship->children.begin(), parentRelationship->children.end(), child) == parentRelationship->children.end()) {
            parentRelationship->children.push_back(child);
        }

        return true;
    }

    bool clearParent(EntityId child) {
        if (!isAlive(child) || !hasRelationship(child)) {
            return false;
        }
        detachFromParent(child);
        return true;
    }

    EntityId parentOf(EntityId child) const {
        const RelationshipComponent* relationship = relationships.get(child);
        if (relationship == nullptr) {
            return EntityId::invalid();
        }
        return relationship->parent;
    }

    std::vector<EntityId> childrenOf(EntityId parent) const {
        const RelationshipComponent* relationship = relationships.get(parent);
        if (relationship == nullptr) {
            return {};
        }
        return relationship->children;
    }

    std::optional<EntityId> primaryCamera2D() const {
        for (EntityId entity : collectEntities(cameras2D)) {
            const Camera2DComponent* camera = cameras2D.get(entity);
            if (camera != nullptr && camera->primary) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryAudioListener2D() const {
        for (EntityId entity : collectEntities(audioListeners2D)) {
            const AudioListener2DComponent* listener = audioListeners2D.get(entity);
            if (listener != nullptr && listener->primary) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryCamera3D() const {
        for (EntityId entity : collectEntities(cameras3D)) {
            const Camera3DComponent* camera = cameras3D.get(entity);
            if (camera != nullptr && camera->primary) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryAudioListener3D() const {
        for (EntityId entity : collectEntities(audioListeners3D)) {
            const AudioListener3DComponent* listener = audioListeners3D.get(entity);
            if (listener != nullptr && listener->primary) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> findByName(const std::string& name) const {
        for (EntityId entity : aliveEntities()) {
            const NameComponent* component = names.get(entity);
            if (component != nullptr && component->value == name) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> findByTag(const std::string& tag) const {
        for (EntityId entity : aliveEntities()) {
            const TagComponent* component = tags.get(entity);
            if (component != nullptr && component->value == tag) {
                return entity;
            }
        }
        return std::nullopt;
    }

    std::vector<EntityId> spriteEntities2D() const {
        return collectEntities(spriteRenderers2D);
    }

    std::vector<EntityId> meshEntities3D() const {
        return collectEntities(meshRenderers3D);
    }

    std::vector<EntityId> pointLightEntities3D() const {
        return collectEntities(pointLights3D);
    }

    std::vector<EntityId> directionalLightEntities3D() const {
        return collectEntities(directionalLights3D);
    }

    std::vector<EntityId> audioSourceEntities2D() const {
        return collectEntities(audioSources2D);
    }

    std::vector<EntityId> audioSourceEntities3D() const {
        return collectEntities(audioSources3D);
    }

    std::vector<EntityId> rigidBodyEntities2D() const {
        return collectEntities(rigidBodies2D);
    }

    std::vector<EntityId> rigidBodyEntities3D() const {
        return collectEntities(rigidBodies3D);
    }

    std::vector<EntityId> boxColliderEntities2D() const {
        return collectEntities(boxColliders2D);
    }

    std::vector<EntityId> circleColliderEntities2D() const {
        return collectEntities(circleColliders2D);
    }

    bool overlaps2D(EntityId first, EntityId second) const {
        if (!isAlive(first) || !isAlive(second)) {
            return false;
        }

        const physics::Vec2 firstPosition = entityPosition2D(first);
        const physics::Vec2 secondPosition = entityPosition2D(second);
        const physics::BoxCollider2D* firstBox = boxColliders2D.get(first);
        const physics::CircleCollider2D* firstCircle = circleColliders2D.get(first);
        const physics::BoxCollider2D* secondBox = boxColliders2D.get(second);
        const physics::CircleCollider2D* secondCircle = circleColliders2D.get(second);

        if (firstBox != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstBox, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondCircle != nullptr && physics::overlaps(*firstBox, firstPosition, *secondCircle, secondPosition)) {
                return true;
            }
        }

        if (firstCircle != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstCircle, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondCircle != nullptr && physics::overlaps(*firstCircle, firstPosition, *secondCircle, secondPosition)) {
                return true;
            }
        }

        return false;
    }

    bool containsPoint2D(EntityId entity, const physics::Vec2& point) const {
        if (!isAlive(entity)) {
            return false;
        }

        const physics::Vec2 position = entityPosition2D(entity);
        if (const physics::BoxCollider2D* box = boxColliders2D.get(entity)) {
            if (physics::containsPoint(*box, position, point)) {
                return true;
            }
        }
        if (const physics::CircleCollider2D* circle = circleColliders2D.get(entity)) {
            if (physics::containsPoint(*circle, position, point)) {
                return true;
            }
        }
        return false;
    }

    void syncPhysicsFromTransforms2D() {
        for (EntityId entity : collectEntities(rigidBodies2D)) {
            Transform2D* transform = transforms2D.get(entity);
            physics::RigidBody2D* body = rigidBodies2D.get(entity);
            if (transform == nullptr || body == nullptr) {
                continue;
            }
            body->position = transform->position;
        }
    }

    void syncTransformsFromPhysics2D() {
        for (EntityId entity : collectEntities(rigidBodies2D)) {
            Transform2D* transform = transforms2D.get(entity);
            physics::RigidBody2D* body = rigidBodies2D.get(entity);
            if (transform == nullptr || body == nullptr) {
                continue;
            }
            transform->position = body->position;
        }
    }

    void syncPhysicsFromTransforms3D() {
        for (EntityId entity : collectEntities(rigidBodies3D)) {
            Transform3D* transform = transforms3D.get(entity);
            physics::RigidBody3D* body = rigidBodies3D.get(entity);
            if (transform == nullptr || body == nullptr) {
                continue;
            }
            body->position = transform->position;
        }
    }

    void syncTransformsFromPhysics3D() {
        for (EntityId entity : collectEntities(rigidBodies3D)) {
            Transform3D* transform = transforms3D.get(entity);
            physics::RigidBody3D* body = rigidBodies3D.get(entity);
            if (transform == nullptr || body == nullptr) {
                continue;
            }
            transform->position = body->position;
        }
    }

private:
    static size_t entityToSlot(EntityId entity) {
        return static_cast<size_t>(entity.index - 1);
    }

    void requireAlive(EntityId entity) const {
        if (!isAlive(entity)) {
            throw std::out_of_range("Zenith World entity is not alive.");
        }
    }

    RelationshipComponent& ensureRelationship(EntityId entity) {
        if (RelationshipComponent* relationship = relationships.get(entity)) {
            return *relationship;
        }
        return relationships.assign(entity, RelationshipComponent{});
    }

    void detachFromParent(EntityId child) {
        RelationshipComponent* childRelationship = relationships.get(child);
        if (childRelationship == nullptr || !childRelationship->parent) {
            return;
        }

        RelationshipComponent* parentRelationship = relationships.get(childRelationship->parent);
        if (parentRelationship != nullptr) {
            auto it = std::remove(parentRelationship->children.begin(), parentRelationship->children.end(), child);
            parentRelationship->children.erase(it, parentRelationship->children.end());
        }

        childRelationship->parent = EntityId::invalid();
    }

    void orphanChildren(EntityId parent) {
        RelationshipComponent* parentRelationship = relationships.get(parent);
        if (parentRelationship == nullptr) {
            return;
        }

        for (const EntityId child : parentRelationship->children) {
            RelationshipComponent* childRelationship = relationships.get(child);
            if (childRelationship != nullptr && childRelationship->parent == parent) {
                childRelationship->parent = EntityId::invalid();
            }
        }

        parentRelationship->children.clear();
    }

    bool wouldCreateCycle(EntityId child, EntityId proposedParent) const {
        EntityId cursor = proposedParent;
        while (cursor) {
            if (cursor == child) {
                return true;
            }
            const RelationshipComponent* relationship = relationships.get(cursor);
            if (relationship == nullptr || !relationship->parent) {
                break;
            }
            cursor = relationship->parent;
        }
        return false;
    }

    template <typename T>
    std::vector<EntityId> collectEntities(const ComponentPool<T>& pool) const {
        std::vector<EntityId> entities;
        entities.reserve(liveCount);
        for (size_t i = 0; i < alive.size(); ++i) {
            if (alive[i] == 0 || !pool.hasSlot(i)) {
                continue;
            }
            entities.push_back(EntityId{
                static_cast<uint32_t>(i + 1),
                generations[i]
            });
        }
        return entities;
    }

    physics::Vec2 entityPosition2D(EntityId entity) const {
        if (const Transform2D* transform = transforms2D.get(entity)) {
            return transform->position;
        }
        if (const physics::RigidBody2D* body = rigidBodies2D.get(entity)) {
            return body->position;
        }
        return physics::Vec2();
    }

    std::vector<uint32_t> generations;
    std::vector<uint8_t> alive;
    std::vector<uint32_t> freeList;
    size_t liveCount = 0;

    ComponentPool<NameComponent> names;
    ComponentPool<TagComponent> tags;
    ComponentPool<Transform2D> transforms2D;
    ComponentPool<Transform3D> transforms3D;
    ComponentPool<physics::RigidBody2D> rigidBodies2D;
    ComponentPool<physics::RigidBody3D> rigidBodies3D;
    ComponentPool<physics::BoxCollider2D> boxColliders2D;
    ComponentPool<physics::CircleCollider2D> circleColliders2D;
    ComponentPool<SpriteRenderer2D> spriteRenderers2D;
    ComponentPool<Character2DComponent> characters2D;
    ComponentPool<AudioSource2DComponent> audioSources2D;
    ComponentPool<MeshRenderer3D> meshRenderers3D;
    ComponentPool<Character3DComponent> characters3D;
    ComponentPool<AudioSource3DComponent> audioSources3D;
    ComponentPool<PointLight3DComponent> pointLights3D;
    ComponentPool<DirectionalLight3DComponent> directionalLights3D;
    ComponentPool<Camera2DComponent> cameras2D;
    ComponentPool<AudioListener2DComponent> audioListeners2D;
    ComponentPool<Camera3DComponent> cameras3D;
    ComponentPool<AudioListener3DComponent> audioListeners3D;
    ComponentPool<RelationshipComponent> relationships;
};

} // namespace game
} // namespace zenith

#endif // ZENITH_WORLD_H
