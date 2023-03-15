#pragma once
#include <cstdint>
#include <functional>
#include "../core/Types.h"

namespace nebula {

class EntityManager;

enum class EntityFlag {
    Dirty = 1 << 0,
    PendingDestroy = 1 << 1,
    Visible = 1 << 2
};

enum class EntityAccessMode {
    Read,
    Write,
    Exclusive
};

struct Entity {
    u64 id;
    u32 version;
    u32 flags;
    u64 prefabId;

    Entity() : id(0), version(0), flags(static_cast<u32>(EntityFlag::Visible)), prefabId(0) {}
    Entity(u64 id, u32 version) : id(id), version(version), flags(static_cast<u32>(EntityFlag::Visible)), prefabId(0) {}

    u64 getId() const { return id; }
    u32 getVersion() const { return version; }
    bool isValid() const { return id != 0; }

    void setFlag(EntityFlag flag) { flags |= static_cast<u32>(flag); }
    void clearFlag(EntityFlag flag) { flags &= ~static_cast<u32>(flag); }
    bool hasFlag(EntityFlag flag) const { return (flags & static_cast<u32>(flag)) != 0; }

    bool isVisible() const { return hasFlag(EntityFlag::Visible); }
    bool isPendingDestroy() const { return hasFlag(EntityFlag::PendingDestroy); }

    void setPrefabLink(u64 prefab) { prefabId = prefab; }
    u64 getPrefabLink() const { return prefabId; }
    bool hasPrefabLink() const { return prefabId != 0; }

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
