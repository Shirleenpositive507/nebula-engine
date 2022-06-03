#pragma once
#include <cstdint>
#include <functional>
#include "../core/Types.h"

namespace nebula {

class EntityManager;

struct Entity {
    u64 id;
    u32 version;

    Entity() : id(0), version(0) {}
    Entity(u64 id, u32 version) : id(id), version(version) {}

    u64 getId() const { return id; }
    u32 getVersion() const { return version; }
    bool isValid() const { return id != 0; }

    bool operator==(const Entity& other) const {
        return id == other.id && version == other.version;
    }

    bool operator!=(const Entity& other) const {
        return id != other.id || version != other.version;
    }

    bool operator<(const Entity& other) const {
        return id < other.id || (id == other.id && version < other.version);
    }
};

const Entity NULL_ENTITY{};

class EntityHandle {
public:
    EntityHandle() : mEntity(NULL_ENTITY), mManager(nullptr) {}
    EntityHandle(Entity entity, EntityManager* manager)
        : mEntity(entity), mManager(manager) {}

    Entity getEntity() const { return mEntity; }
    bool isValid() const { return mEntity.isValid() && mManager != nullptr; }

    template <typename T, typename... Args>
    T& addComponent(Args&&... args) {
        return mManager->addComponent<T>(mEntity, std::forward<Args>(args)...);
    }

    template <typename T>
    void removeComponent() {
        mManager->removeComponent<T>(mEntity);
    }

    template <typename T>
    T* getComponent() {
        return mManager->getComponent<T>(mEntity);
    }

    template <typename T>
    bool hasComponent() {
        return mManager->hasComponent<T>(mEntity);
    }

    void destroy();

    EntityManager* getManager() const { return mManager; }

    bool operator==(const EntityHandle& other) const {
        return mEntity == other.mEntity && mManager == other.mManager;
    }

    bool operator!=(const EntityHandle& other) const {
        return !(*this == other);
    }

    operator bool() const { return isValid(); }

private:
    Entity mEntity;
    EntityManager* mManager;
};

}

namespace std {
    template <>
    struct hash<nebula::Entity> {
        size_t operator()(const nebula::Entity& e) const {
            return hash<nebula::u64>()(e.id) ^ (hash<nebula::u32>()(e.version) << 1);
        }
    };
}
