#pragma once
#include <vector>
#include <functional>
#include <bitset>
#include <algorithm>
#include <thread>
#include <mutex>
#include <memory>
#include "Component.h"
#include "Archetype.h"
#include "Entity.h"

namespace nebula {

class EntityManager;

enum class SortOrder {
    Ascending,
    Descending
};

template <typename T>
struct OrderBy {
    ComponentType componentType;
    SortOrder order;
};

struct QueryChangeInfo {
    std::vector<Entity> added;
    std::vector<Entity> removed;
    u32 frameNumber;
};

struct ChunkInfo {
    size_t startIndex;
    size_t endIndex;
    size_t chunkSize;

    ChunkInfo() : startIndex(0), endIndex(0), chunkSize(0) {}
    ChunkInfo(size_t start, size_t end) : startIndex(start), endIndex(end), chunkSize(end - start) {}
};

class Query {
public:
    Query() = default;

    Query& withAll(ComponentType type);
    Query& withAny(ComponentType type);
    Query& withNone(ComponentType type);

    Query& withAll(const std::vector<ComponentType>& types);
    Query& withAny(const std::vector<ComponentType>& types);
    Query& withNone(const std::vector<ComponentType>& types);

    Query& cacheResults(bool enable);
    Query& orderBy(ComponentType type, SortOrder order = SortOrder::Ascending);

    void each(std::function<void(Entity)> callback) const;
    void each(std::function<void(EntityHandle)> callback) const;

    template <typename... Components>
    void forEach(std::function<void(Entity, Components&...)> callback) const;

    void parallelForEach(std::function<void(Entity)> callback) const;
    void parallelForEach(std::function<void(EntityHandle)> callback) const;

    void parallelForEachChunked(std::function<void(const ChunkInfo&, const std::vector<Entity>&)> callback,
                                 size_t chunkSize = 64) const;

    bool matches(const Archetype& archetype) const;
    void invalidate();
    void forceRefresh();
    bool isValid() const { return mValid; }

    void setEntityManager(EntityManager* manager) { mManager = manager; }

    void setUseJobSystem(bool enable) { mUseJobSystem = enable; }
    bool isUsingJobSystem() const { return mUseJobSystem; }
    void setMaxWorkerThreads(u32 count) { mMaxWorkerThreads = count; }
    u32 getMaxWorkerThreads() const { return mMaxWorkerThreads; }

    const std::vector<ComponentType>& getWithAll() const { return mWithAll; }
    const std::vector<ComponentType>& getWithAny() const { return mWithAny; }
    const std::vector<ComponentType>& getWithNone() const { return mWithNone; }

    const QueryChangeInfo& getChangeInfo() const { return mChangeInfo; }
    void resetChangeTracking();
    u32 getFrameNumber() const { return mFrameNumber; }
    void setFrameNumber(u32 frame) { mFrameNumber = frame; }

private:
    std::vector<ComponentType> mWithAll;
    std::vector<ComponentType> mWithAny;
    std::vector<ComponentType> mWithNone;
    mutable bool mValid = false;
    mutable bool mCacheEnabled = false;
    mutable std::vector<Entity> mCachedResults;
    EntityManager* mManager = nullptr;
    ComponentType mOrderByType = INVALID_COMPONENT_TYPE;
    SortOrder mOrderDirection = SortOrder::Ascending;
    mutable QueryChangeInfo mChangeInfo;
    u32 mFrameNumber = 0;
    mutable u32 mLastCacheFrame = 0;
    bool mUseJobSystem = false;
    u32 mMaxWorkerThreads = 0;

    void dispatchChunks(const std::vector<Entity>& entities,
                        std::function<void(const ChunkInfo&, const std::vector<Entity>&)> callback,
                        size_t chunkSize) const;
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
