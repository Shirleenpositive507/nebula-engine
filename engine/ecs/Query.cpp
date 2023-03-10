#include "Query.h"
#include "EntityManager.h"

namespace nebula {

Query& Query::withAll(ComponentType type) {
    mWithAll.push_back(type);
    mValid = false;
    return *this;
}

Query& Query::withAny(ComponentType type) {
    mWithAny.push_back(type);
    mValid = false;
    return *this;
}

Query& Query::withNone(ComponentType type) {
    mWithNone.push_back(type);
    mValid = false;
    return *this;
}

Query& Query::withAll(const std::vector<ComponentType>& types) {
    mWithAll.insert(mWithAll.end(), types.begin(), types.end());
    mValid = false;
    return *this;
}

Query& Query::withAny(const std::vector<ComponentType>& types) {
    mWithAny.insert(mWithAny.end(), types.begin(), types.end());
    mValid = false;
    return *this;
}

Query& Query::withNone(const std::vector<ComponentType>& types) {
    mWithNone.insert(mWithNone.end(), types.begin(), types.end());
    mValid = false;
    return *this;
}

bool Query::matches(const Archetype& archetype) const {
    const auto& mask = archetype.getMask();

    for (auto type : mWithAll) {
        if (!mask.test(type)) return false;
    }

    if (!mWithAny.empty()) {
        bool foundAny = false;
        for (auto type : mWithAny) {
            if (mask.test(type)) {
                foundAny = true;
                break;
            }
        }
        if (!foundAny) return false;
    }

    for (auto type : mWithNone) {
        if (mask.test(type)) return false;
    }

    return true;
}

void Query::each(std::function<void(Entity)> callback) const {
    mManager->queryEntities(*this, callback);
}

void Query::each(std::function<void(EntityHandle)> callback) const {
    mManager->queryEntities(*this, callback);
}

void Query::invalidate() {
    if (mValid && mCacheEnabled && !mCachedResults.empty()) {
        mChangeInfo.removed = mCachedResults;
    }
    mValid = false;
    mCachedResults.clear();
    mChangeInfo.added.clear();
}

void Query::resetChangeTracking() {
    mChangeInfo.added.clear();
    mChangeInfo.removed.clear();
    mChangeInfo.frameNumber = mFrameNumber;
}

Query& Query::cacheResults(bool enable) {
    mCacheEnabled = enable;
    if (!enable) {
        mValid = false;
        mCachedResults.clear();
    }
    return *this;
}

Query& Query::orderBy(ComponentType type, SortOrder order) {
    mOrderByType = type;
    mOrderDirection = order;
    return *this;
}

void Query::parallelForEach(std::function<void(Entity)> callback) const {
    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) threadCount = 4;

    if (mCacheEnabled && mValid) {
        const auto& results = mCachedResults;
        size_t chunkSize = (results.size() + threadCount - 1) / threadCount;
        std::vector<std::thread> threads;

        for (unsigned int t = 0; t < threadCount; ++t) {
            size_t start = t * chunkSize;
            size_t end = std::min(start + chunkSize, results.size());
            if (start >= end) break;
            threads.emplace_back([&results, start, end, &callback]() {
                for (size_t i = start; i < end; ++i) {
                    callback(results[i]);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
        return;
    }

    mManager->queryEntities(*this, callback);
}

void Query::parallelForEach(std::function<void(EntityHandle)> callback) const {
    parallelForEach([&](Entity entity) {
        callback(EntityHandle(entity, mManager));
    });
}

template <typename... Components>
void Query::forEach(std::function<void(Entity, Components&...)> callback) const {
    mManager->queryComponents<Components...>(*this, callback);
}

template <typename T>
T* View::getComponentArray() {
    return mManager->getComponentArray<T>();
}

u32 View::size() const {
    return mManager ? mManager->getEntityCount() : 0;
}

bool View::empty() const {
    return size() == 0;
}

}
