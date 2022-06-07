#include "EntityManager.h"
#include "System.h"

namespace nebula {

EntityManager::EntityManager()
    : mNextId(1)
    , mEntityCount(0)
    , mActiveCount(0)
    , mEmptyArchetype(nullptr)
    , mSystemManager(nullptr)
{
    mEmptyArchetype = mArchetypeGraph.getOrCreateArchetype({});
}

EntityManager::~EntityManager() {
    destroyAll();
}

u64 EntityManager::generateId() {
    if (!mFreeIds.empty()) {
        u64 id = mFreeIds.back();
        mFreeIds.pop_back();
        return id;
    }
    return mNextId++;
}

void EntityManager::recycleId(u64 id) {
    mFreeIds.push_back(id);
}

Entity EntityManager::createEntity() {
    u64 id = generateId();
    Entity entity;
    entity.id = id;
    entity.version = 0;

    EntityData data;
    data.version = 0;
    data.archetype = mEmptyArchetype;
    data.archetypeIndex = mEmptyArchetype->getEntityCount();

    mEntities[id] = data;
    mActiveEntities.push_back(entity);
    mEntityCount++;
    mActiveCount++;

    mEmptyArchetype->incrementEntityCount();

    return entity;
}

std::vector<Entity> EntityManager::createEntities(u32 count) {
    std::vector<Entity> entities;
    entities.reserve(count);
    for (u32 i = 0; i < count; ++i) {
        entities.push_back(createEntity());
    }
    return entities;
}

bool EntityManager::entityExists(Entity entity) const {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return false;
    return it->second.version == entity.version;
}

void EntityManager::destroyEntity(Entity entity) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;
    if (it->second.version != entity.version) return;

    EntityData& data = it->second;
    Archetype* arch = data.archetype;

    for (auto type : arch->getComponentTypes()) {
        auto poolIt = mPools.find(std::type_index(typeid(Component)));
        if (poolIt != mPools.end()) {
            poolIt->second->destroy(reinterpret_cast<void*>(static_cast<uintptr_t>(entity.id)));
        }
    }

    arch->decrementEntityCount();

    auto aeIt = std::find_if(mActiveEntities.begin(), mActiveEntities.end(),
        [&](const Entity& e) { return e.id == entity.id; });
    if (aeIt != mActiveEntities.end()) {
        mActiveEntities.erase(aeIt);
    }

    data.version++;
    recycleId(entity.id);

    mActiveCount--;
    onStructureChange();
}

void EntityManager::destroyEntities(const std::vector<Entity>& entities) {
    for (const auto& entity : entities) {
        destroyEntity(entity);
    }
}

void EntityManager::destroyAll() {
    for (auto& pair : mEntities) {
        EntityData& data = pair.second;
        Archetype* arch = data.archetype;

        for (auto type : arch->getComponentTypes()) {
            auto poolIt = mPools.find(std::type_index(typeid(Component)));
            if (poolIt != mPools.end()) {
                poolIt->second->destroy(
                    reinterpret_cast<void*>(static_cast<uintptr_t>(pair.first)));
            }
        }
    }

    mEntities.clear();
    mActiveEntities.clear();
    mFreeIds.clear();
    mNextId = 1;
    mEntityCount = 0;
    mActiveCount = 0;
    mPools.clear();
    mArchetypeGraph.clear();
    mEmptyArchetype = mArchetypeGraph.getOrCreateArchetype({});
    onStructureChange();
}

void EntityManager::migrateEntity(Entity entity, Archetype* newArchetype) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;

    EntityData& data = it->second;
    Archetype* oldArch = data.archetype;

    for (auto type : oldArch->getComponentTypes()) {
        auto poolIt = mPools.find(std::type_index(typeid(Component)));
        if (poolIt != mPools.end()) {
            void* comp = poolIt->second->get(
                reinterpret_cast<void*>(static_cast<uintptr_t>(entity.id)));
            if (comp) {
                poolIt->second->destroy(
                    reinterpret_cast<void*>(static_cast<uintptr_t>(entity.id)));
            }
        }
    }

    oldArch->decrementEntityCount();
    data.archetype = newArchetype;
    data.archetypeIndex = newArchetype->getEntityCount();
    newArchetype->incrementEntityCount();
}

void EntityManager::updateArchetypeTracking(Entity entity, Archetype* oldArch, Archetype* newArch) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;

    EntityData& data = it->second;
    if (oldArch) oldArch->decrementEntityCount();
    data.archetype = newArch;
    data.archetypeIndex = newArch->getEntityCount();
    if (newArch) newArch->incrementEntityCount();
}

void EntityManager::onStructureChange() {
    for (auto& pair : mEntities) {
        (void)pair;
    }
}

void EntityManager::queryEntities(const Query& query,
    std::function<void(Entity)> callback)
{
    for (const auto& entity : mActiveEntities) {
        auto it = mEntities.find(entity.id);
        if (it == mEntities.end()) continue;

        if (query.matches(*it->second.archetype)) {
            callback(entity);
        }
    }
}

void EntityManager::queryEntities(const Query& query,
    std::function<void(EntityHandle)> callback)
{
    for (const auto& entity : mActiveEntities) {
        auto it = mEntities.find(entity.id);
        if (it == mEntities.end()) continue;

        if (query.matches(*it->second.archetype)) {
            callback(EntityHandle(entity, this));
        }
    }
}

}
