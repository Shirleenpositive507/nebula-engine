#pragma once

template <typename T, typename... Args>
T& EntityManager::addComponent(Entity entity, Args&&... args) {
    auto it = mEntities.find(entity.id);
    assert(it != mEntities.end());
    assert(it->second.version == entity.version);

    ComponentType typeId = ComponentID<T>::id;
    assert(typeId != INVALID_COMPONENT_TYPE);

    EntityData& data = it->second;
    Archetype* oldArch = data.archetype;
    Archetype* newArch = mArchetypeGraph.addComponent(oldArch, typeId);

    if (oldArch != newArch) {
        migrateEntity(entity, newArch);
    }

    ComponentPool<T>* pool = getOrCreatePool<T>();
    T component(std::forward<Args>(args)...);
    pool->add(static_cast<u32>(entity.id), component);

    onStructureChange();
    return *pool->get(static_cast<u32>(entity.id));
}

template <typename T>
void EntityManager::removeComponent(Entity entity) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return;
    if (it->second.version != entity.version) return;

    ComponentType typeId = ComponentID<T>::id;
    EntityData& data = it->second;
    Archetype* oldArch = data.archetype;

    if (!oldArch->hasComponent(typeId)) return;

    Archetype* newArch = mArchetypeGraph.removeComponent(oldArch, typeId);

    auto poolIt = mPools.find(std::type_index(typeid(T)));
    if (poolIt != mPools.end()) {
        poolIt->second->destroy(reinterpret_cast<void*>(static_cast<uintptr_t>(entity.id)));
    }

    if (oldArch != newArch) {
        migrateEntity(entity, newArch);
    }

    onStructureChange();
}

template <typename T>
T* EntityManager::getComponent(Entity entity) {
    auto it = mEntities.find(entity.id);
    if (it == mEntities.end()) return nullptr;
    if (it->second.version != entity.version) return nullptr;

    auto poolIt = mPools.find(std::type_index(typeid(T)));
    if (poolIt == mPools.end()) return nullptr;

    return static_cast<ComponentPool<T>*>(poolIt->second.get())->get(static_cast<u32>(entity.id));
}

template <typename T>
bool EntityManager::hasComponent(Entity entity) {
    return getComponent<T>(entity) != nullptr;
}

template <typename T>
ComponentPool<T>* EntityManager::getOrCreatePool() {
    auto it = mPools.find(std::type_index(typeid(T)));
    if (it != mPools.end()) {
        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    auto pool = std::make_unique<ComponentPool<T>>();
    ComponentPool<T>* ptr = pool.get();
    mPools[std::type_index(typeid(T))] = std::move(pool);
    return ptr;
}

template <typename T>
T* EntityManager::getComponentArray() {
    auto poolIt = mPools.find(std::type_index(typeid(T)));
    if (poolIt == mPools.end()) return nullptr;
    return static_cast<ComponentPool<T>*>(poolIt->second.get())->getDataArray();
}

template <typename... Components>
void EntityManager::queryComponents(const Query& query,
    std::function<void(Entity, Components&...)> callback)
{
    for (const auto& entity : mActiveEntities) {
        auto it = mEntities.find(entity.id);
        if (it == mEntities.end()) continue;

        if (!query.matches(*it->second.archetype)) continue;

        callback(entity, *getComponent<Components>(entity)...);
    }
}
