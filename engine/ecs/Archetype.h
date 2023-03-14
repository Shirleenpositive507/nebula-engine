#pragma once
#include <bitset>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <queue>
#include <new>
#include "Component.h"

namespace nebula {

using ComponentMask = std::bitset<MAX_COMPONENT_TYPES>;

constexpr u32 CHUNK_CAPACITY = 64;

struct ComponentChunk {
    u8* data;
    u32 entityCount;
    u32 capacity;

    ComponentChunk(u32 compSize, u32 cap = CHUNK_CAPACITY)
        : data(static_cast<u8*>(::operator new(compSize * cap)))
        , entityCount(0)
        , capacity(cap) {}

    ~ComponentChunk() { ::operator delete(data); }
    ComponentChunk(const ComponentChunk&) = delete;
    ComponentChunk& operator=(const ComponentChunk&) = delete;
    ComponentChunk(ComponentChunk&& other) noexcept
        : data(other.data), entityCount(other.entityCount), capacity(other.capacity) {
        other.data = nullptr;
        other.entityCount = 0;
        other.capacity = 0;
    }
};

class Archetype {
public:
    Archetype() = default;
    explicit Archetype(const std::vector<ComponentType>& types)
        : mComponentTypes(types)
    {
        for (auto type : types) {
            mMask.set(type);
        }
        buildComponentIndex();
    }

    const ComponentMask& getMask() const { return mMask; }

    void addComponent(ComponentType type) {
        if (!mMask.test(type)) {
            mMask.set(type);
            mComponentTypes.push_back(type);
            buildComponentIndex();
        }
    }

    void removeComponent(ComponentType type) {
        if (mMask.test(type)) {
            mMask.reset(type);
            auto it = std::find(mComponentTypes.begin(), mComponentTypes.end(), type);
            if (it != mComponentTypes.end()) {
                mComponentTypes.erase(it);
            }
            buildComponentIndex();
        }
    }

    bool hasComponent(ComponentType type) const {
        return mMask.test(type);
    }

    const std::vector<ComponentType>& getComponentTypes() const {
        return mComponentTypes;
    }

    i32 getComponentIndex(ComponentType type) const {
        auto it = mComponentIndex.find(type);
        return it != mComponentIndex.end() ? it->second : -1;
    }

    u32 getEntityCount() const { return mEntityCount; }
    void setEntityCount(u32 count) { mEntityCount = count; }
    void incrementEntityCount() { mEntityCount++; }
    void decrementEntityCount() { if (mEntityCount > 0) mEntityCount--; }

    u32 getChunkCount() const { return static_cast<u32>(mChunks.size()); }
    bool hasAvailableChunkSlot() const {
        return mChunks.empty() || mChunks.back()->entityCount < mChunks.back()->capacity;
    }
    void addChunk(ComponentChunk* chunk) { mChunks.push_back(chunk); }
    const std::vector<ComponentChunk*>& getChunks() const { return mChunks; }

    bool isEmpty() const { return mEntityCount == 0; }
    void markEmpty() { mEntityCount = 0; mChunks.clear(); }

    bool operator==(const Archetype& other) const {
        return mMask == other.mMask;
    }

    bool operator!=(const Archetype& other) const {
        return mMask != other.mMask;
    }

    bool operator<(const Archetype& other) const {
        return mMask.to_ullong() < other.mMask.to_ullong();
    }

private:
    void buildComponentIndex() {
        mComponentIndex.clear();
        for (size_t i = 0; i < mComponentTypes.size(); ++i) {
            mComponentIndex[mComponentTypes[i]] = static_cast<i32>(i);
        }
    }

    ComponentMask mMask;
    std::vector<ComponentType> mComponentTypes;
    std::unordered_map<ComponentType, i32> mComponentIndex;
    u32 mEntityCount = 0;
    std::vector<ComponentChunk*> mChunks;
};

struct ArchetypeEdge {
    ComponentType added;
    ComponentType removed;
    Archetype* target;
};

class ArchetypeGraph {
public:
    Archetype* getOrCreateArchetype(const std::vector<ComponentType>& types);
    Archetype* addComponent(Archetype* src, ComponentType type);
    Archetype* removeComponent(Archetype* src, ComponentType type);
    bool hasTransition(Archetype* src, Archetype* dst) const;
    std::vector<Archetype*> findTransitionPath(Archetype* src, Archetype* dst) const;
    void cleanupEmptyArchetypes();
    void clear();

private:
    std::vector<std::unique_ptr<Archetype>> mArchetypes;
    std::unordered_map<ComponentMask, Archetype*> mArchetypeMap;
    std::unordered_map<Archetype*, std::vector<ArchetypeEdge>> mTransitions;
    ComponentMask buildMask(const std::vector<ComponentType>& types) const;
};

}

namespace std {
    template <>
    struct hash<nebula::Archetype> {
        size_t operator()(const nebula::Archetype& a) const {
            return a.getMask().to_ullong();
        }
    };
}
