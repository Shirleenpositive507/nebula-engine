#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <typeindex>
#include <cassert>
#include <string>
#include "Entity.h"
#include "Component.h"
#include "ComponentStorage.h"
#include "Archetype.h"
#include "Query.h"

namespace nebula {

struct EntityGroup {
    std::vector<ComponentType> requiredComponents;
    std::string name;
};

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    Entity createEntity();
    std::vector<Entity> createEntities(u32 count);

    void destroyEntity(Entity entity);
    void destroyEntities(const std::vector<Entity>& entities);
    void destroyAll();

    bool entityExists(Entity entity) const;
    u32 getEntityCount() const { return mEntityCount; }
    u32 getActiveEntityCount() const { return mActiveCount; }

    const std::vector<Entity>& getActiveEntities() const { return mActiveEntities; }

    Entity cloneEntity(Entity entity);

    void tagEntity(Entity entity, const std::string& tag);
    void untagEntity(Entity entity, const std::string& tag);
    bool hasTag(Entity entity, const std::string& tag) const;
    std::vector<Entity> getEntitiesWithTag(const std::string& tag) const;
    std::vector<std::string> getEntityTags(Entity entity) const;

    void setEntityParent(Entity entity, Entity parent);
    Entity getEntityParent(Entity entity) const;
    std::vector<Entity> getEntityChildren(Entity entity) const;

    void queryEntityGroup(const EntityGroup& group,
        std::function<void(Entity)> callback);

    template <typename T, typename... Args>
    T& addComponent(Entity entity, Args&&... args);

    template <typename T>
    void removeComponent(Entity entity);

    template <typename T>
    T* getComponent(Entity entity);

    template <typename T>
    bool hasComponent(Entity entity);

    template <typename T>
    T* getComponentArray();

    template <typename... Components>
    void queryComponents(const Query& query,
        std::function<void(Entity, Components&...)> callback);

    void queryEntities(const Query& query,
        std::function<void(Entity)> callback);

    void queryEntities(const Query& query,
        std::function<void(EntityHandle)> callback);

    void onStructureChange();

    void setSystemManager(class SystemManager* mgr) { mSystemManager = mgr; }
    SystemManager* getSystemManager() const { return mSystemManager; }

private:
    struct EntityData {
        u32 version;
        Archetype* archetype;
        u32 archetypeIndex;
    };

    struct EntityHierarchy {
        Entity parent;
        std::vector<Entity> children;
    };

    u64 generateId();
    void recycleId(u64 id);
    void migrateEntity(Entity entity, Archetype* newArchetype);
    void updateArchetypeTracking(Entity entity, Archetype* oldArch, Archetype* newArch);

    template <typename T>
    ComponentPool<T>* getOrCreatePool();

    std::unordered_map<u64, EntityData> mEntities;

    std::vector<Entity> mActiveEntities;
    std::vector<u64> mFreeIds;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> mPools;

    ArchetypeGraph mArchetypeGraph;
    Archetype* mEmptyArchetype;

    u64 mNextId;
    u32 mEntityCount;
    u32 mActiveCount;

    SystemManager* mSystemManager;

    std::unordered_map<u64, std::vector<std::string>> mEntityTags;
    std::unordered_map<std::string, std::vector<Entity>> mTagIndex;

    std::unordered_map<u64, EntityHierarchy> mHierarchy;
};

}

#include "Entity.inl"
#include "EntityManager.inl"
