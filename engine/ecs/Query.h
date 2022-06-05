#pragma once
#include <vector>
#include <functional>
#include <bitset>
#include <algorithm>
#include "Component.h"
#include "Archetype.h"
#include "Entity.h"

namespace nebula {

class EntityManager;

class Query {
public:
    Query() = default;

    Query& withAll(ComponentType type);
    Query& withAny(ComponentType type);
    Query& withNone(ComponentType type);

    Query& withAll(const std::vector<ComponentType>& types);
    Query& withAny(const std::vector<ComponentType>& types);
    Query& withNone(const std::vector<ComponentType>& types);

    void each(std::function<void(Entity)> callback) const;
    void each(std::function<void(EntityHandle)> callback) const;

    template <typename... Components>
    void forEach(std::function<void(Entity, Components&...)> callback) const;

    bool matches(const Archetype& archetype) const;
    void invalidate();
    bool isValid() const { return mValid; }

    void setEntityManager(EntityManager* manager) { mManager = manager; }

    const std::vector<ComponentType>& getWithAll() const { return mWithAll; }
    const std::vector<ComponentType>& getWithAny() const { return mWithAny; }
    const std::vector<ComponentType>& getWithNone() const { return mWithNone; }

private:
    std::vector<ComponentType> mWithAll;
    std::vector<ComponentType> mWithAny;
    std::vector<ComponentType> mWithNone;
    mutable bool mValid = false;
    mutable std::vector<Entity> mCachedResults;
    EntityManager* mManager = nullptr;
};

class View {
public:
    View() = default;
    View(EntityManager* manager, const std::vector<ComponentType>& types)
        : mManager(manager), mComponentTypes(types) {}

    template <typename T>
    T* getComponentArray();

    u32 size() const;
    bool empty() const;

private:
    EntityManager* mManager = nullptr;
    std::vector<ComponentType> mComponentTypes;
};

}
