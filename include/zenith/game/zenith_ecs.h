#ifndef ZENITH_ECS_H
#define ZENITH_ECS_H

#include "../common/zenith_common.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdint>

namespace zenith {
namespace ecs {

using EntityID = std::uint32_t;
using ComponentTypeID = std::size_t;

inline ComponentTypeID getUniqueComponentID() {
    static ComponentTypeID lastID = 0;
    return lastID++;
}

template <typename T>
inline ComponentTypeID getComponentTypeID() {
    static ComponentTypeID typeID = getUniqueComponentID();
    return typeID;
}

class Entity {
public:
    EntityID id;
    std::uint32_t generation;

    bool operator==(const Entity& other) const {
        return id == other.id && generation == other.generation;
    }
    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }
};

class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void removeEntity(EntityID entity) = 0;
};

template <typename T>
class ComponentPool : public IComponentPool {
private:
    std::vector<T> m_components;
    std::unordered_map<EntityID, std::size_t> m_entityToIndex;
    std::unordered_map<std::size_t, EntityID> m_indexToEntity;

public:
    void assign(EntityID entity, const T& component) {
        if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
            m_components[m_entityToIndex[entity]] = component;
            return;
        }
        std::size_t newIndex = m_components.size();
        m_entityToIndex[entity] = newIndex;
        m_indexToEntity[newIndex] = entity;
        m_components.push_back(component);
    }

    T* get(EntityID entity) {
        auto it = m_entityToIndex.find(entity);
        if (it == m_entityToIndex.end()) return nullptr;
        return &m_components[it->second];
    }

    bool has(EntityID entity) const {
        return m_entityToIndex.find(entity) != m_entityToIndex.end();
    }

    void removeEntity(EntityID entity) override {
        auto it = m_entityToIndex.find(entity);
        if (it == m_entityToIndex.end()) return;

        std::size_t indexToRemove = it->second;
        std::size_t lastIndex = m_components.size() - 1;

        if (indexToRemove != lastIndex) {
            EntityID lastEntity = m_indexToEntity[lastIndex];
            m_components[indexToRemove] = std::move(m_components[lastIndex]);
            m_entityToIndex[lastEntity] = indexToRemove;
            m_indexToEntity[indexToRemove] = lastEntity;
        }

        m_components.pop_back();
        m_entityToIndex.erase(entity);
        m_indexToEntity.erase(lastIndex);
    }

    std::vector<T>& getComponents() { return m_components; }
    const std::unordered_map<std::size_t, EntityID>& getIndexToEntityMap() const { return m_indexToEntity; }
};

class World {
private:
    EntityID m_nextEntityID = 1;
    std::vector<EntityID> m_freeEntities;
    std::vector<std::shared_ptr<IComponentPool>> m_componentPools;

    template <typename T>
    std::shared_ptr<ComponentPool<T>> getPool() {
        ComponentTypeID typeID = getComponentTypeID<T>();
        if (typeID >= m_componentPools.size()) {
            m_componentPools.resize(typeID + 1);
        }
        if (!m_componentPools[typeID]) {
            m_componentPools[typeID] = std::make_shared<ComponentPool<T>>();
        }
        return std::static_pointer_cast<ComponentPool<T>>(m_componentPools[typeID]);
    }

public:
    World() = default;

    Entity createEntity() {
        EntityID id;
        if (!m_freeEntities.empty()) {
            id = m_freeEntities.back();
            m_freeEntities.pop_back();
        } else {
            id = m_nextEntityID++;
        }
        return Entity{id, 1};
    }

    void destroyEntity(Entity entity) {
        for (auto& pool : m_componentPools) {
            if (pool) {
                pool->removeEntity(entity.id);
            }
        }
        m_freeEntities.push_back(entity.id);
    }

    template <typename T>
    void addComponent(Entity entity, const T& component) {
        getPool<T>()->assign(entity.id, component);
    }

    template <typename T>
    T* getComponent(Entity entity) {
        return getPool<T>()->get(entity.id);
    }

    template <typename T>
    bool hasComponent(Entity entity) {
        return getPool<T>()->has(entity.id);
    }

    template <typename CompA, typename CompB, typename Func>
    void query(Func func) {
        auto poolA = getPool<CompA>();
        auto poolB = getPool<CompB>();
        auto& componentsA = poolA->getComponents();
        auto& indexToEntityMap = poolA->getIndexToEntityMap();

        for (std::size_t i = 0; i < componentsA.size(); ++i) {
            EntityID entity = indexToEntityMap.at(i);
            CompB* compB = poolB->get(entity);
            if (compB) {
                func(Entity{entity, 1}, componentsA[i], *compB);
            }
        }
    }
};

} // namespace ecs
} // namespace zenith

#endif // ZENITH_ECS_H
