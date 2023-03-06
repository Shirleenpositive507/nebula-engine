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

Entity EntityManager::cloneEntity(Entity entity) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return NULL_ENTITY;
    if (it->second.version != entity.version) return NULL_ENTITY;

    Entity clone = createEntity();
    EntityData& srcData = it->second;

    for (auto type : srcData.archetype->getComponentTypes()) {
        auto poolIt = mPools.find(std::type_index(typeid(Component)));
        if (poolIt != mPools.end()) {
            void* comp = poolIt->second->get(
                reinterpret_cast<void*>(static_cast<uintptr_t>(entity.id)));
            if (comp) {
                auto clonePoolIt = mPools.find(std::type_index(typeid(Component)));
                if (clonePoolIt != mPools.end()) {
                    clonePoolIt->second->create(
                        reinterpret_cast<void*>(static_cast<uintptr_t>(clone.id)));
                }
            }
        }
    }

    auto tagIt = mEntityTags.find(entity.id);
    if (tagIt != mEntityTags.end()) {
        for (const auto& tag : tagIt->second) {
            tagEntity(clone, tag);
        }
    }

    return clone;
}

void EntityManager::tagEntity(Entity entity, const std::string& tag) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;
    if (it->second.version != entity.version) return;

    auto& tags = mEntityTags[entity.id];
    if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
        tags.push_back(tag);
        mTagIndex[tag].push_back(entity);
    }
}

void EntityManager::untagEntity(Entity entity, const std::string& tag) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;

    auto tagIt = mEntityTags.find(entity.id);
    if (tagIt == mEntityTags.end()) return;

    auto& tags = tagIt->second;
    auto tagPos = std::find(tags.begin(), tags.end(), tag);
    if (tagPos != tags.end()) {
        tags.erase(tagPos);
        auto& idx = mTagIndex[tag];
        idx.erase(std::remove(idx.begin(), idx.end(), entity), idx.end());
    }
}

bool EntityManager::hasTag(Entity entity, const std::string& tag) const {
    auto tagIt = mEntityTags.find(entity.id);
    if (tagIt == mEntityTags.end()) return false;
    return std::find(tagIt->second.begin(), tagIt->second.end(), tag) != tagIt->second.end();
}

std::vector<Entity> EntityManager::getEntitiesWithTag(const std::string& tag) const {
    auto it = mTagIndex.find(tag);
    if (it == mTagIndex.end()) return {};
    return it->second;
}

std::vector<std::string> EntityManager::getEntityTags(Entity entity) const {
    auto it = mEntityTags.find(entity.id);
    if (it == mEntityTags.end()) return {};
    return it->second;
}

void EntityManager::setEntityParent(Entity entity, Entity parent) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;

    EntityHierarchy& childHier = mHierarchy[entity.id];
    if (childHier.parent.isValid()) {
        auto& oldParentChildren = mHierarchy[childHier.parent.id].children;
        oldParentChildren.erase(
            std::remove(oldParentChildren.begin(), oldParentChildren.end(), entity),
            oldParentChildren.end());
    }

    childHier.parent = parent;

    if (parent.isValid()) {
        EntityHierarchy& parentHier = mHierarchy[parent.id];
        if (std::find(parentHier.children.begin(), parentHier.children.end(), entity)
            == parentHier.children.end()) {
            parentHier.children.push_back(entity);
        }
    }
}

Entity EntityManager::getEntityParent(Entity entity) const {
    auto it = mHierarchy.find(entity.id);
    if (it == mHierarchy.end()) return NULL_ENTITY;
    return it->second.parent;
}

std::vector<Entity> EntityManager::getEntityChildren(Entity entity) const {
    auto it = mHierarchy.find(entity.id);
    if (it == mHierarchy.end()) return {};
    return it->second.children;
}

void EntityManager::queryEntityGroup(const EntityGroup& group,
    std::function<void(Entity)> callback)
{
    for (const auto& entity : mActiveEntities) {
        auto it = mEntities.find(entity.id);
        if (it == mEntities.end()) continue;

        bool matches = true;
        for (auto type : group.requiredComponents) {
            if (!it->second.archetype->hasComponent(type)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            callback(entity);
        }
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
