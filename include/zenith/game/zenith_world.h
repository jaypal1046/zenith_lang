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

struct LayerMaskComponent {
    uint32_t layers = 1u;
    uint32_t mask = 0xFFFFFFFFu;
};

struct SpriteRenderer2D {
    std::shared_ptr<resource::TextureAsset> texture;
    resource::AssetHandle textureHandle;
    physics::Vec2 size = physics::Vec2(1.0f, 1.0f);
    physics::Vec2 anchor = physics::Vec2(0.5f, 0.5f);
    std::string tintColor = "white";
    int sortOrder = 0;
    bool visible = true;
};

struct Tilemap2DComponent {
    int columns = 1;
    int rows = 1;
    physics::Vec2 tileSize = physics::Vec2(1.0f, 1.0f);
    physics::Vec2 anchor;
    std::vector<int> cells = std::vector<int>(1, 0);
    std::vector<std::string> paletteColors = std::vector<std::string>{"", "white"};
    int sortOrder = 0;
    bool visible = true;

    void resize(int nextColumns, int nextRows, int fillTile = 0) {
        columns = std::max(1, nextColumns);
        rows = std::max(1, nextRows);
        cells.assign(static_cast<size_t>(columns * rows), fillTile);
    }

    bool inBounds(int column, int row) const {
        return column >= 0 && row >= 0 && column < columns && row < rows;
    }

    int cellAt(int column, int row) const {
        if (!inBounds(column, row)) {
            return 0;
        }
        return cells[static_cast<size_t>(row * columns + column)];
    }

    void setCellAt(int column, int row, int tileId) {
        if (!inBounds(column, row)) {
            return;
        }
        cells[static_cast<size_t>(row * columns + column)] = std::max(0, tileId);
    }

    void fill(int tileId) {
        std::fill(cells.begin(), cells.end(), std::max(0, tileId));
    }

    void ensurePaletteIndex(int tileId) {
        if (tileId < 0) {
            return;
        }
        if (paletteColors.size() <= static_cast<size_t>(tileId)) {
            paletteColors.resize(static_cast<size_t>(tileId + 1), "white");
        }
    }

    void setPaletteColor(int tileId, const std::string& color) {
        if (tileId < 0) {
            return;
        }
        ensurePaletteIndex(tileId);
        paletteColors[static_cast<size_t>(tileId)] = color;
    }

    std::string paletteColor(int tileId) const {
        if (tileId < 0 || static_cast<size_t>(tileId) >= paletteColors.size()) {
            return tileId == 0 ? std::string() : std::string("white");
        }
        return paletteColors[static_cast<size_t>(tileId)];
    }
};

struct Character2DComponent {
    float moveSpeed = 10.0f;
    float jumpForce = 12.0f;
    bool isGrounded = false;
    bool facingRight = true;
};

struct AudioSource2DComponent {
    std::shared_ptr<resource::AudioAsset> clip;
    resource::AssetHandle clipHandle;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnAwake = false;
    bool isPlaying = false;
};

struct MeshRenderer3D {
    std::shared_ptr<resource::MeshAsset> mesh;
    resource::AssetHandle meshHandle;
    std::shared_ptr<resource::ShaderAsset> shader;
    resource::AssetHandle shaderHandle;
    std::shared_ptr<resource::MaterialAsset> material;
    resource::AssetHandle materialHandle;
    bool visible = true;
    bool castShadows = true;
};

struct Character3DComponent {
    float moveSpeed = 6.0f;
    float turnSpeed = 4.0f;
    float jumpSpeed = 7.5f;
    float groundAcceleration = 36.0f;
    float airAcceleration = 14.0f;
    float groundFriction = 20.0f;
    float airControl = 0.35f;
    float groundSnapDistance = 0.2f;
    float maxSlopeAngle = 55.0f;
    physics::Vec3 moveInput = physics::Vec3(0.0f, 0.0f, 0.0f);
    bool isGrounded = false;
    bool jumpQueued = false;
    bool useGravity = true;
};

struct AudioSource3DComponent {
    std::shared_ptr<resource::AudioAsset> clip;
    resource::AssetHandle clipHandle;
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

struct RaycastHit2DResult {
    bool hit = false;
    EntityId entity = EntityId::invalid();
    float distance = 0.0f;
    float pointX = 0.0f;
    float pointY = 0.0f;
    float normalX = 0.0f;
    float normalY = 0.0f;
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

struct RaycastHit3DResult {
    bool hit = false;
    EntityId entity = EntityId::invalid();
    float distance = 0.0f;
    float pointX = 0.0f;
    float pointY = 0.0f;
    float pointZ = 0.0f;
    float normalX = 0.0f;
    float normalY = 0.0f;
    float normalZ = 0.0f;
};

template <typename T>
class ComponentPool {
public:
    T& assign(EntityId entity, const T& value = T()) {
        return upsert(entity, value);
    }

    T& assign(EntityId entity, T&& value) {
        return upsert(entity, std::move(value));
    }

    bool has(EntityId entity) const {
        if (!entity) {
            return false;
        }
        size_t slot = entityToSlot(entity);
        if (!hasSlot(slot)) {
            return false;
        }
        return entities[sparse[slot] - 1] == entity;
    }

    bool hasSlot(size_t slot) const {
        return slot < sparse.size() && sparse[slot] != 0;
    }

    T* get(EntityId entity) {
        if (!has(entity)) {
            return nullptr;
        }
        size_t slot = entityToSlot(entity);
        return &data[sparse[slot] - 1];
    }

    const T* get(EntityId entity) const {
        if (!has(entity)) {
            return nullptr;
        }
        size_t slot = entityToSlot(entity);
        return &data[sparse[slot] - 1];
    }

    void remove(EntityId entity) {
        if (!has(entity)) {
            return;
        }
        size_t slot = entityToSlot(entity);
        size_t denseIndex = static_cast<size_t>(sparse[slot] - 1);
        size_t lastIndex = data.size() - 1;

        if (denseIndex != lastIndex) {
            data[denseIndex] = std::move(data[lastIndex]);
            entities[denseIndex] = entities[lastIndex];
            sparse[entityToSlot(entities[denseIndex])] = static_cast<uint32_t>(denseIndex + 1);
        }

        data.pop_back();
        entities.pop_back();
        sparse[slot] = 0;
    }

    size_t size() const {
        return data.size();
    }

    EntityId entityAt(size_t index) const {
        return entities[index];
    }

    T& componentAt(size_t index) {
        return data[index];
    }

    const T& componentAt(size_t index) const {
        return data[index];
    }

    const std::vector<EntityId>& entityList() const {
        return entities;
    }

    std::vector<T>& componentData() {
        return data;
    }

    const std::vector<T>& componentData() const {
        return data;
    }

private:
    static size_t entityToSlot(EntityId entity) {
        return static_cast<size_t>(entity.index - 1);
    }

    void ensureSparseSize(size_t size) {
        if (sparse.size() < size) {
            sparse.resize(size, 0);
        }
    }

    template <typename TValue>
    T& upsert(EntityId entity, TValue&& value) {
        size_t slot = entityToSlot(entity);
        ensureSparseSize(slot + 1);

        if (hasSlot(slot)) {
            size_t denseIndex = static_cast<size_t>(sparse[slot] - 1);
            if (entities[denseIndex] == entity) {
                data[denseIndex] = std::forward<TValue>(value);
                return data[denseIndex];
            }
            sparse[slot] = 0;
        }

        entities.push_back(entity);
        data.push_back(std::forward<TValue>(value));
        sparse[slot] = static_cast<uint32_t>(data.size());
        return data.back();
    }

    std::vector<T> data;
    std::vector<EntityId> entities;
    std::vector<uint32_t> sparse;
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
        capsuleColliders2D.remove(entity);
        boxColliders3D.remove(entity);
        sphereColliders3D.remove(entity);
        spriteRenderers2D.remove(entity);
        tilemaps2D.remove(entity);
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
        layerMasks.remove(entity);

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

    physics::CapsuleCollider2D& addCapsuleCollider2D(EntityId entity, const physics::CapsuleCollider2D& value = physics::CapsuleCollider2D()) {
        requireAlive(entity);
        return capsuleColliders2D.assign(entity, value);
    }

    physics::BoxCollider3D& addBoxCollider3D(EntityId entity, const physics::BoxCollider3D& value = physics::BoxCollider3D()) {
        requireAlive(entity);
        return boxColliders3D.assign(entity, value);
    }

    physics::SphereCollider3D& addSphereCollider3D(EntityId entity, const physics::SphereCollider3D& value = physics::SphereCollider3D()) {
        requireAlive(entity);
        return sphereColliders3D.assign(entity, value);
    }

    physics::CapsuleCollider3D& addCapsuleCollider3D(EntityId entity, const physics::CapsuleCollider3D& value = physics::CapsuleCollider3D()) {
        requireAlive(entity);
        return capsuleColliders3D.assign(entity, value);
    }

    SpriteRenderer2D& addSpriteRenderer2D(EntityId entity, const SpriteRenderer2D& value = SpriteRenderer2D()) {
        requireAlive(entity);
        return spriteRenderers2D.assign(entity, value);
    }

    Tilemap2DComponent& addTilemap2D(EntityId entity, const Tilemap2DComponent& value = Tilemap2DComponent()) {
        requireAlive(entity);
        return tilemaps2D.assign(entity, value);
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

    LayerMaskComponent& addLayerMask(EntityId entity, const LayerMaskComponent& value = LayerMaskComponent()) {
        requireAlive(entity);
        LayerMaskComponent normalized = value;
        normalized.layers = normalizeLayerBits(normalized.layers);
        return layerMasks.assign(entity, normalized);
    }

    bool hasName(EntityId entity) const { return names.has(entity); }
    bool hasTag(EntityId entity) const { return tags.has(entity); }
    bool hasTransform2D(EntityId entity) const { return transforms2D.has(entity); }
    bool hasTransform3D(EntityId entity) const { return transforms3D.has(entity); }
    bool hasRigidBody2D(EntityId entity) const { return rigidBodies2D.has(entity); }
    bool hasRigidBody3D(EntityId entity) const { return rigidBodies3D.has(entity); }
    bool hasBoxCollider2D(EntityId entity) const { return boxColliders2D.has(entity); }
    bool hasCircleCollider2D(EntityId entity) const { return circleColliders2D.has(entity); }
    bool hasCapsuleCollider2D(EntityId entity) const { return capsuleColliders2D.has(entity); }
    bool hasBoxCollider3D(EntityId entity) const { return boxColliders3D.has(entity); }
    bool hasSphereCollider3D(EntityId entity) const { return sphereColliders3D.has(entity); }
    bool hasSpriteRenderer2D(EntityId entity) const { return spriteRenderers2D.has(entity); }
    bool hasTilemap2D(EntityId entity) const { return tilemaps2D.has(entity); }
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
    bool hasLayerMask(EntityId entity) const { return layerMasks.has(entity); }

    NameComponent* getName(EntityId entity) { return names.get(entity); }
    TagComponent* getTag(EntityId entity) { return tags.get(entity); }
    Transform2D* getTransform2D(EntityId entity) { return transforms2D.get(entity); }
    Transform3D* getTransform3D(EntityId entity) { return transforms3D.get(entity); }
    physics::RigidBody2D* getRigidBody2D(EntityId entity) { return rigidBodies2D.get(entity); }
    physics::RigidBody3D* getRigidBody3D(EntityId entity) { return rigidBodies3D.get(entity); }
    physics::BoxCollider2D* getBoxCollider2D(EntityId entity) { return boxColliders2D.get(entity); }
    physics::CircleCollider2D* getCircleCollider2D(EntityId entity) { return circleColliders2D.get(entity); }
    physics::CapsuleCollider2D* getCapsuleCollider2D(EntityId entity) { return capsuleColliders2D.get(entity); }
    physics::BoxCollider3D* getBoxCollider3D(EntityId entity) { return boxColliders3D.get(entity); }
    physics::SphereCollider3D* getSphereCollider3D(EntityId entity) { return sphereColliders3D.get(entity); }
    physics::CapsuleCollider3D* getCapsuleCollider3D(EntityId entity) { return capsuleColliders3D.get(entity); }
    SpriteRenderer2D* getSpriteRenderer2D(EntityId entity) { return spriteRenderers2D.get(entity); }
    Tilemap2DComponent* getTilemap2D(EntityId entity) { return tilemaps2D.get(entity); }
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
    LayerMaskComponent* getLayerMask(EntityId entity) { return layerMasks.get(entity); }

    const NameComponent* getName(EntityId entity) const { return names.get(entity); }
    const TagComponent* getTag(EntityId entity) const { return tags.get(entity); }
    const Transform2D* getTransform2D(EntityId entity) const { return transforms2D.get(entity); }
    const Transform3D* getTransform3D(EntityId entity) const { return transforms3D.get(entity); }
    const physics::RigidBody2D* getRigidBody2D(EntityId entity) const { return rigidBodies2D.get(entity); }
    const physics::RigidBody3D* getRigidBody3D(EntityId entity) const { return rigidBodies3D.get(entity); }
    const physics::BoxCollider2D* getBoxCollider2D(EntityId entity) const { return boxColliders2D.get(entity); }
    const physics::CircleCollider2D* getCircleCollider2D(EntityId entity) const { return circleColliders2D.get(entity); }
    const physics::CapsuleCollider2D* getCapsuleCollider2D(EntityId entity) const { return capsuleColliders2D.get(entity); }
    const physics::BoxCollider3D* getBoxCollider3D(EntityId entity) const { return boxColliders3D.get(entity); }
    const physics::SphereCollider3D* getSphereCollider3D(EntityId entity) const { return sphereColliders3D.get(entity); }
    const physics::CapsuleCollider3D* getCapsuleCollider3D(EntityId entity) const { return capsuleColliders3D.get(entity); }
    const SpriteRenderer2D* getSpriteRenderer2D(EntityId entity) const { return spriteRenderers2D.get(entity); }
    const Tilemap2DComponent* getTilemap2D(EntityId entity) const { return tilemaps2D.get(entity); }
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
    const LayerMaskComponent* getLayerMask(EntityId entity) const { return layerMasks.get(entity); }

    void removeName(EntityId entity) { names.remove(entity); }
    void removeTag(EntityId entity) { tags.remove(entity); }
    void removeTransform2D(EntityId entity) { transforms2D.remove(entity); }
    void removeTransform3D(EntityId entity) { transforms3D.remove(entity); }
    void removeRigidBody2D(EntityId entity) { rigidBodies2D.remove(entity); }
    void removeRigidBody3D(EntityId entity) { rigidBodies3D.remove(entity); }
    void removeBoxCollider2D(EntityId entity) { boxColliders2D.remove(entity); }
    void removeCircleCollider2D(EntityId entity) { circleColliders2D.remove(entity); }
    void removeCapsuleCollider2D(EntityId entity) { capsuleColliders2D.remove(entity); }
    void removeBoxCollider3D(EntityId entity) { boxColliders3D.remove(entity); }
    void removeSphereCollider3D(EntityId entity) { sphereColliders3D.remove(entity); }
    void removeSpriteRenderer2D(EntityId entity) { spriteRenderers2D.remove(entity); }
    void removeTilemap2D(EntityId entity) { tilemaps2D.remove(entity); }
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
    void removeLayerMask(EntityId entity) { layerMasks.remove(entity); }

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

    bool destroyEntityHierarchy(EntityId entity) {
        if (!isAlive(entity)) {
            return false;
        }

        std::vector<EntityId> children = childrenOf(entity);
        for (EntityId child : children) {
            destroyEntityHierarchy(child);
        }

        return destroyEntity(entity);
    }

    EntityId cloneEntity(EntityId source, const std::string& entityName = "") {
        if (!isAlive(source)) {
            return EntityId::invalid();
        }

        EntityId clone = createEntity();
        copyEntityComponents(source, clone);
        if (!entityName.empty()) {
            addName(clone, entityName);
        }
        return clone;
    }

    EntityId cloneEntityHierarchy(EntityId source, const std::string& entityName = "") {
        if (!isAlive(source)) {
            return EntityId::invalid();
        }

        EntityId clone = cloneEntity(source, entityName);
        for (EntityId child : childrenOf(source)) {
            EntityId clonedChild = cloneEntityHierarchy(child);
            if (clonedChild) {
                setParent(clonedChild, clone);
            }
        }
        return clone;
    }

    EntityId instantiatePrefab2D(EntityId source, float posX, float posY, const std::string& entityName = "") {
        EntityId instance = cloneEntityHierarchy(source, entityName);
        if (instance) {
            if (Transform2D* transform = transforms2D.get(instance)) {
                transform->position = physics::Vec2(posX, posY);
            }
            if (physics::RigidBody2D* body = rigidBodies2D.get(instance)) {
                body->position = physics::Vec2(posX, posY);
            }
        }
        return instance;
    }

    EntityId instantiatePrefab3D(EntityId source, float posX, float posY, float posZ, const std::string& entityName = "") {
        EntityId instance = cloneEntityHierarchy(source, entityName);
        if (instance) {
            if (Transform3D* transform = transforms3D.get(instance)) {
                transform->position = physics::Vec3(posX, posY, posZ);
            }
            if (physics::RigidBody3D* body = rigidBodies3D.get(instance)) {
                body->position = physics::Vec3(posX, posY, posZ);
            }
        }
        return instance;
    }

    uint32_t entityLayerBits(EntityId entity) const {
        if (const LayerMaskComponent* layerMask = layerMasks.get(entity)) {
            return normalizeLayerBits(layerMask->layers);
        }
        return 1u;
    }

    uint32_t entityMaskBits(EntityId entity) const {
        if (const LayerMaskComponent* layerMask = layerMasks.get(entity)) {
            return layerMask->mask;
        }
        return 0xFFFFFFFFu;
    }

    bool canInteract(EntityId first, EntityId second) const {
        if (!isAlive(first) || !isAlive(second)) {
            return false;
        }

        return layerMaskMatches(entityMaskBits(first), entityLayerBits(second)) &&
               layerMaskMatches(entityMaskBits(second), entityLayerBits(first));
    }

    std::optional<EntityId> primaryCamera2D() const {
        for (size_t i = 0; i < cameras2D.size(); ++i) {
            const Camera2DComponent& camera = cameras2D.componentAt(i);
            if (camera.primary) {
                return cameras2D.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryAudioListener2D() const {
        for (size_t i = 0; i < audioListeners2D.size(); ++i) {
            const AudioListener2DComponent& listener = audioListeners2D.componentAt(i);
            if (listener.primary) {
                return audioListeners2D.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryCamera3D() const {
        for (size_t i = 0; i < cameras3D.size(); ++i) {
            const Camera3DComponent& camera = cameras3D.componentAt(i);
            if (camera.primary) {
                return cameras3D.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> primaryAudioListener3D() const {
        for (size_t i = 0; i < audioListeners3D.size(); ++i) {
            const AudioListener3DComponent& listener = audioListeners3D.componentAt(i);
            if (listener.primary) {
                return audioListeners3D.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> findByName(const std::string& name) const {
        for (size_t i = 0; i < names.size(); ++i) {
            const NameComponent& component = names.componentAt(i);
            if (component.value == name) {
                return names.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::optional<EntityId> findByTag(const std::string& tag) const {
        for (size_t i = 0; i < tags.size(); ++i) {
            const TagComponent& component = tags.componentAt(i);
            if (component.value == tag) {
                return tags.entityAt(i);
            }
        }
        return std::nullopt;
    }

    std::vector<EntityId> spriteEntities2D() const {
        return copyEntities(spriteRenderers2D);
    }

    std::vector<EntityId> tilemapEntities2D() const {
        return copyEntities(tilemaps2D);
    }

    std::vector<EntityId> meshEntities3D() const {
        return copyEntities(meshRenderers3D);
    }

    std::vector<EntityId> pointLightEntities3D() const {
        return copyEntities(pointLights3D);
    }

    std::vector<EntityId> directionalLightEntities3D() const {
        return copyEntities(directionalLights3D);
    }

    std::vector<EntityId> audioSourceEntities2D() const {
        return copyEntities(audioSources2D);
    }

    std::vector<EntityId> audioSourceEntities3D() const {
        return copyEntities(audioSources3D);
    }

    std::vector<EntityId> rigidBodyEntities2D() const {
        return copyEntities(rigidBodies2D);
    }

    std::vector<EntityId> rigidBodyEntities3D() const {
        return copyEntities(rigidBodies3D);
    }

    std::vector<EntityId> boxColliderEntities2D() const {
        return copyEntities(boxColliders2D);
    }

    std::vector<EntityId> circleColliderEntities2D() const {
        return copyEntities(circleColliders2D);
    }

    std::vector<EntityId> capsuleColliderEntities2D() const {
        return copyEntities(capsuleColliders2D);
    }

    std::vector<EntityId> boxColliderEntities3D() const {
        return copyEntities(boxColliders3D);
    }

    std::vector<EntityId> sphereColliderEntities3D() const {
        return copyEntities(sphereColliders3D);
    }

    size_t spriteRendererCount2D() const {
        return spriteRenderers2D.size();
    }

    size_t tilemapCount2D() const {
        return tilemaps2D.size();
    }

    template <typename Fn>
    void forEachSpriteRenderer2D(Fn&& fn) const {
        forEachComponent(spriteRenderers2D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachTilemap2D(Fn&& fn) {
        forEachComponent(tilemaps2D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachTilemap2D(Fn&& fn) const {
        forEachComponent(tilemaps2D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachRigidBody2D(Fn&& fn) {
        forEachComponent(rigidBodies2D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachRigidBody2D(Fn&& fn) const {
        forEachComponent(rigidBodies2D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachRigidBody3D(Fn&& fn) {
        forEachComponent(rigidBodies3D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachRigidBody3D(Fn&& fn) const {
        forEachComponent(rigidBodies3D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachCharacter3D(Fn&& fn) {
        forEachComponent(characters3D, std::forward<Fn>(fn));
    }

    template <typename Fn>
    void forEachCharacter3D(Fn&& fn) const {
        forEachComponent(characters3D, std::forward<Fn>(fn));
    }

    bool overlaps2D(EntityId first, EntityId second) const {
        if (!isAlive(first) || !isAlive(second)) {
            return false;
        }
        if (!canInteract(first, second)) {
            return false;
        }

        const physics::Vec2 firstPosition = entityPosition2D(first);
        const physics::Vec2 secondPosition = entityPosition2D(second);
        const physics::BoxCollider2D* firstBox = boxColliders2D.get(first);
        const physics::CircleCollider2D* firstCircle = circleColliders2D.get(first);
        const physics::CapsuleCollider2D* firstCapsule = capsuleColliders2D.get(first);
        const physics::BoxCollider2D* secondBox = boxColliders2D.get(second);
        const physics::CircleCollider2D* secondCircle = circleColliders2D.get(second);
        const physics::CapsuleCollider2D* secondCapsule = capsuleColliders2D.get(second);

        if (firstBox != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstBox, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondCircle != nullptr && physics::overlaps(*firstBox, firstPosition, *secondCircle, secondPosition)) {
                return true;
            }
            if (secondCapsule != nullptr && physics::overlaps(*firstBox, firstPosition, *secondCapsule, secondPosition)) {
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
            if (secondCapsule != nullptr && physics::overlaps(*firstCircle, firstPosition, *secondCapsule, secondPosition)) {
                return true;
            }
        }

        if (firstCapsule != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstCapsule, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondCircle != nullptr && physics::overlaps(*firstCapsule, firstPosition, *secondCircle, secondPosition)) {
                return true;
            }
            if (secondCapsule != nullptr && physics::overlaps(*firstCapsule, firstPosition, *secondCapsule, secondPosition)) {
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
        if (const physics::CapsuleCollider2D* capsule = capsuleColliders2D.get(entity)) {
            if (physics::containsPoint(*capsule, position, point)) {
                return true;
            }
        }
        return false;
    }

    RaycastHit2DResult raycast2D(
        float originX,
        float originY,
        float directionX,
        float directionY,
        float maxDistance
    ) const {
        return raycast2D(originX, originY, directionX, directionY, maxDistance, 0xFFFFFFFFu);
    }

    RaycastHit2DResult raycast2D(
        float originX,
        float originY,
        float directionX,
        float directionY,
        float maxDistance,
        uint32_t layerMask
    ) const {
        RaycastHit2DResult bestHit;
        if (maxDistance < 0.0f) {
            return bestHit;
        }

        const physics::Ray2D ray{
            physics::Vec2(originX, originY),
            physics::Vec2(directionX, directionY)
        };

        float bestDistance = maxDistance;
        const auto considerHit = [&](EntityId entity, float distance, const physics::Vec2& point, const physics::Vec2& normal) {
            if (distance > bestDistance) {
                return;
            }
            bestDistance = distance;
            bestHit.hit = true;
            bestHit.entity = entity;
            bestHit.distance = distance;
            bestHit.pointX = point.x;
            bestHit.pointY = point.y;
            bestHit.normalX = normal.x;
            bestHit.normalY = normal.y;
        };

        const std::vector<EntityId>& boxEntities = boxColliders2D.entityList();
        const std::vector<physics::BoxCollider2D>& boxData = boxColliders2D.componentData();
        for (size_t i = 0; i < boxData.size(); ++i) {
            EntityId entity = boxEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::BoxCollider2D& collider = boxData[i];
            float distance = 0.0f;
            physics::Vec2 point;
            physics::Vec2 normal;
            if (physics::raycast(ray, collider, entityPosition2D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        const std::vector<EntityId>& circleEntities = circleColliders2D.entityList();
        const std::vector<physics::CircleCollider2D>& circleData = circleColliders2D.componentData();
        for (size_t i = 0; i < circleData.size(); ++i) {
            EntityId entity = circleEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::CircleCollider2D& collider = circleData[i];
            float distance = 0.0f;
            physics::Vec2 point;
            physics::Vec2 normal;
            if (physics::raycast(ray, collider, entityPosition2D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        const std::vector<EntityId>& capsuleEntities = capsuleColliders2D.entityList();
        const std::vector<physics::CapsuleCollider2D>& capsuleData = capsuleColliders2D.componentData();
        for (size_t i = 0; i < capsuleData.size(); ++i) {
            EntityId entity = capsuleEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::CapsuleCollider2D& collider = capsuleData[i];
            float distance = 0.0f;
            physics::Vec2 point;
            physics::Vec2 normal;
            if (physics::raycast(ray, collider, entityPosition2D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        return bestHit;
    }

    bool overlaps3D(EntityId first, EntityId second) const {
        if (!isAlive(first) || !isAlive(second)) {
            return false;
        }
        if (!canInteract(first, second)) {
            return false;
        }

        const physics::Vec3 firstPosition = entityPosition3D(first);
        const physics::Vec3 secondPosition = entityPosition3D(second);
        const physics::BoxCollider3D* firstBox = boxColliders3D.get(first);
        const physics::SphereCollider3D* firstSphere = sphereColliders3D.get(first);
        const physics::BoxCollider3D* secondBox = boxColliders3D.get(second);
        const physics::SphereCollider3D* secondSphere = sphereColliders3D.get(second);

        if (firstBox != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstBox, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondSphere != nullptr && physics::overlaps(*firstBox, firstPosition, *secondSphere, secondPosition)) {
                return true;
            }
        }

        if (firstSphere != nullptr) {
            if (secondBox != nullptr && physics::overlaps(*firstSphere, firstPosition, *secondBox, secondPosition)) {
                return true;
            }
            if (secondSphere != nullptr && physics::overlaps(*firstSphere, firstPosition, *secondSphere, secondPosition)) {
                return true;
            }
        }

        return false;
    }

    bool containsPoint3D(EntityId entity, const physics::Vec3& point) const {
        if (!isAlive(entity)) {
            return false;
        }

        const physics::Vec3 position = entityPosition3D(entity);
        if (const physics::BoxCollider3D* box = boxColliders3D.get(entity)) {
            if (physics::containsPoint(*box, position, point)) {
                return true;
            }
        }
        if (const physics::SphereCollider3D* sphere = sphereColliders3D.get(entity)) {
            if (physics::containsPoint(*sphere, position, point)) {
                return true;
            }
        }
        return false;
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
        return raycast3D(originX, originY, originZ, directionX, directionY, directionZ, maxDistance, 0xFFFFFFFFu);
    }

    RaycastHit3DResult raycast3D(
        float originX,
        float originY,
        float originZ,
        float directionX,
        float directionY,
        float directionZ,
        float maxDistance,
        uint32_t layerMask
    ) const {
        RaycastHit3DResult bestHit;
        if (maxDistance < 0.0f) {
            return bestHit;
        }

        const physics::Ray3D ray{
            physics::Vec3(originX, originY, originZ),
            physics::Vec3(directionX, directionY, directionZ)
        };

        float bestDistance = maxDistance;
        const auto considerHit = [&](EntityId entity, float distance, const physics::Vec3& point, const physics::Vec3& normal) {
            if (distance > bestDistance) {
                return;
            }
            bestDistance = distance;
            bestHit.hit = true;
            bestHit.entity = entity;
            bestHit.distance = distance;
            bestHit.pointX = point.x;
            bestHit.pointY = point.y;
            bestHit.pointZ = point.z;
            bestHit.normalX = normal.x;
            bestHit.normalY = normal.y;
            bestHit.normalZ = normal.z;
        };

        const std::vector<EntityId>& boxEntities = boxColliders3D.entityList();
        const std::vector<physics::BoxCollider3D>& boxData = boxColliders3D.componentData();
        for (size_t i = 0; i < boxData.size(); ++i) {
            EntityId entity = boxEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::BoxCollider3D& collider = boxData[i];
            float distance = 0.0f;
            physics::Vec3 point;
            physics::Vec3 normal;
            if (physics::raycast(ray, collider, entityPosition3D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        const std::vector<EntityId>& sphereEntities = sphereColliders3D.entityList();
        const std::vector<physics::SphereCollider3D>& sphereData = sphereColliders3D.componentData();
        for (size_t i = 0; i < sphereData.size(); ++i) {
            EntityId entity = sphereEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::SphereCollider3D& collider = sphereData[i];
            float distance = 0.0f;
            physics::Vec3 point;
            physics::Vec3 normal;
            if (physics::raycast(ray, collider, entityPosition3D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        const std::vector<EntityId>& capsuleEntities = capsuleColliders3D.entityList();
        const std::vector<physics::CapsuleCollider3D>& capsuleData = capsuleColliders3D.componentData();
        for (size_t i = 0; i < capsuleData.size(); ++i) {
            EntityId entity = capsuleEntities[i];
            if (!layerMaskMatches(layerMask, entityLayerBits(entity))) {
                continue;
            }
            const physics::CapsuleCollider3D& collider = capsuleData[i];
            float distance = 0.0f;
            physics::Vec3 point;
            physics::Vec3 normal;
            if (physics::raycast(ray, collider, entityPosition3D(entity), bestDistance, distance, point, normal)) {
                considerHit(entity, distance, point, normal);
            }
        }

        return bestHit;
    }

    void syncPhysicsFromTransforms2D() {
        const std::vector<EntityId>& bodyEntities = rigidBodies2D.entityList();
        std::vector<physics::RigidBody2D>& bodyData = rigidBodies2D.componentData();
        for (size_t i = 0; i < bodyData.size(); ++i) {
            EntityId entity = bodyEntities[i];
            Transform2D* transform = transforms2D.get(entity);
            physics::RigidBody2D& body = bodyData[i];
            if (transform == nullptr) {
                continue;
            }
            body.position = transform->position;
        }
    }

    void syncTransformsFromPhysics2D() {
        const std::vector<EntityId>& bodyEntities = rigidBodies2D.entityList();
        const std::vector<physics::RigidBody2D>& bodyData = rigidBodies2D.componentData();
        for (size_t i = 0; i < bodyData.size(); ++i) {
            EntityId entity = bodyEntities[i];
            Transform2D* transform = transforms2D.get(entity);
            const physics::RigidBody2D& body = bodyData[i];
            if (transform == nullptr) {
                continue;
            }
            transform->position = body.position;
        }
    }

    void syncPhysicsFromTransforms3D() {
        const std::vector<EntityId>& bodyEntities = rigidBodies3D.entityList();
        std::vector<physics::RigidBody3D>& bodyData = rigidBodies3D.componentData();
        for (size_t i = 0; i < bodyData.size(); ++i) {
            EntityId entity = bodyEntities[i];
            Transform3D* transform = transforms3D.get(entity);
            physics::RigidBody3D& body = bodyData[i];
            if (transform == nullptr) {
                continue;
            }
            body.position = transform->position;
        }
    }

    void syncTransformsFromPhysics3D() {
        const std::vector<EntityId>& bodyEntities = rigidBodies3D.entityList();
        const std::vector<physics::RigidBody3D>& bodyData = rigidBodies3D.componentData();
        for (size_t i = 0; i < bodyData.size(); ++i) {
            EntityId entity = bodyEntities[i];
            Transform3D* transform = transforms3D.get(entity);
            const physics::RigidBody3D& body = bodyData[i];
            if (transform == nullptr) {
                continue;
            }
            transform->position = body.position;
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

    static uint32_t normalizeLayerBits(uint32_t layers) {
        return layers == 0u ? 1u : layers;
    }

    static bool layerMaskMatches(uint32_t maskBits, uint32_t layerBits) {
        return (maskBits & normalizeLayerBits(layerBits)) != 0u;
    }

    void copyEntityComponents(EntityId source, EntityId target) {
        if (const NameComponent* component = names.get(source)) {
            names.assign(target, *component);
        }
        if (const TagComponent* component = tags.get(source)) {
            tags.assign(target, *component);
        }
        if (const Transform2D* component = transforms2D.get(source)) {
            transforms2D.assign(target, *component);
        }
        if (const Transform3D* component = transforms3D.get(source)) {
            transforms3D.assign(target, *component);
        }
        if (const physics::RigidBody2D* component = rigidBodies2D.get(source)) {
            rigidBodies2D.assign(target, *component);
        }
        if (const physics::RigidBody3D* component = rigidBodies3D.get(source)) {
            rigidBodies3D.assign(target, *component);
        }
        if (const physics::BoxCollider2D* component = boxColliders2D.get(source)) {
            boxColliders2D.assign(target, *component);
        }
        if (const physics::CircleCollider2D* component = circleColliders2D.get(source)) {
            circleColliders2D.assign(target, *component);
        }
        if (const physics::CapsuleCollider2D* component = capsuleColliders2D.get(source)) {
            capsuleColliders2D.assign(target, *component);
        }
        if (const physics::BoxCollider3D* component = boxColliders3D.get(source)) {
            boxColliders3D.assign(target, *component);
        }
        if (const physics::SphereCollider3D* component = sphereColliders3D.get(source)) {
            sphereColliders3D.assign(target, *component);
        }
        if (const SpriteRenderer2D* component = spriteRenderers2D.get(source)) {
            spriteRenderers2D.assign(target, *component);
        }
        if (const Tilemap2DComponent* component = tilemaps2D.get(source)) {
            tilemaps2D.assign(target, *component);
        }
        if (const Character2DComponent* component = characters2D.get(source)) {
            characters2D.assign(target, *component);
        }
        if (const AudioSource2DComponent* component = audioSources2D.get(source)) {
            audioSources2D.assign(target, *component);
        }
        if (const MeshRenderer3D* component = meshRenderers3D.get(source)) {
            meshRenderers3D.assign(target, *component);
        }
        if (const Character3DComponent* component = characters3D.get(source)) {
            characters3D.assign(target, *component);
        }
        if (const AudioSource3DComponent* component = audioSources3D.get(source)) {
            audioSources3D.assign(target, *component);
        }
        if (const PointLight3DComponent* component = pointLights3D.get(source)) {
            pointLights3D.assign(target, *component);
        }
        if (const DirectionalLight3DComponent* component = directionalLights3D.get(source)) {
            directionalLights3D.assign(target, *component);
        }
        if (const Camera2DComponent* component = cameras2D.get(source)) {
            Camera2DComponent camera = *component;
            camera.primary = false;
            cameras2D.assign(target, camera);
        }
        if (const AudioListener2DComponent* component = audioListeners2D.get(source)) {
            AudioListener2DComponent listener = *component;
            listener.primary = false;
            audioListeners2D.assign(target, listener);
        }
        if (const Camera3DComponent* component = cameras3D.get(source)) {
            Camera3DComponent camera = *component;
            camera.primary = false;
            cameras3D.assign(target, camera);
        }
        if (const AudioListener3DComponent* component = audioListeners3D.get(source)) {
            AudioListener3DComponent listener = *component;
            listener.primary = false;
            audioListeners3D.assign(target, listener);
        }
        if (const LayerMaskComponent* component = layerMasks.get(source)) {
            layerMasks.assign(target, *component);
        }
    }

    template <typename T>
    std::vector<EntityId> copyEntities(const ComponentPool<T>& pool) const {
        return std::vector<EntityId>(pool.entityList().begin(), pool.entityList().end());
    }

    template <typename T, typename Fn>
    static void forEachComponent(ComponentPool<T>& pool, Fn&& fn) {
        const std::vector<EntityId>& entities = pool.entityList();
        std::vector<T>& components = pool.componentData();
        for (size_t i = 0; i < components.size(); ++i) {
            fn(entities[i], components[i]);
        }
    }

    template <typename T, typename Fn>
    static void forEachComponent(const ComponentPool<T>& pool, Fn&& fn) {
        const std::vector<EntityId>& entities = pool.entityList();
        const std::vector<T>& components = pool.componentData();
        for (size_t i = 0; i < components.size(); ++i) {
            fn(entities[i], components[i]);
        }
    }

    template <typename T>
    std::vector<EntityId> collectEntities(const ComponentPool<T>& pool) const {
        std::vector<EntityId> entities;
        entities.reserve(pool.size());
        forEachComponent(pool, [&entities](EntityId entity, const T&) {
            entities.push_back(entity);
        });
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

    physics::Vec3 entityPosition3D(EntityId entity) const {
        if (const Transform3D* transform = transforms3D.get(entity)) {
            return transform->position;
        }
        if (const physics::RigidBody3D* body = rigidBodies3D.get(entity)) {
            return body->position;
        }
        return physics::Vec3();
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
    ComponentPool<physics::CapsuleCollider2D> capsuleColliders2D;
    ComponentPool<physics::BoxCollider3D> boxColliders3D;
    ComponentPool<physics::SphereCollider3D> sphereColliders3D;
    ComponentPool<physics::CapsuleCollider3D> capsuleColliders3D;
    ComponentPool<SpriteRenderer2D> spriteRenderers2D;
    ComponentPool<Tilemap2DComponent> tilemaps2D;
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
    ComponentPool<LayerMaskComponent> layerMasks;
};

} // namespace game
} // namespace zenith

#endif // ZENITH_WORLD_H
