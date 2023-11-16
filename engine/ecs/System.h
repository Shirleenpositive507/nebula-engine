#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include "../core/Types.h"
#include <mutex>

class SystemScheduler;

namespace nebula {

enum class SystemGroup {
    Logic = 0,
    Physics,
    Render,
    UI,
    PostProcess,
    Count
};

enum class SystemPhase {
    PreUpdate = 0,
    Update,
    PostUpdate,
    PreRender,
    Render,
    PostRender,
    UI,
    Debug,
    Count
};

class System {
public:
    System() = default;
    explicit System(const std::string& name)
        : mName(name) {}

    virtual ~System() = default;

    virtual void update(f32 dt) = 0;
    virtual void render(f32 dt) {}
    virtual void phase(f32 dt, SystemPhase phase) { (void)dt; (void)phase; }

    void setEnabled(bool enabled) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    void setPriority(i32 priority) { mPriority = priority; }
    i32 getPriority() const { return mPriority; }

    void setName(const std::string& name) { mName = name; }
    const std::string& getName() const { return mName; }

    void setGroup(SystemGroup group) { mGroup = group; }
    SystemGroup getGroup() const { return mGroup; }

    void setPhase(SystemPhase phase) { mPhase = phase; }
    SystemPhase getPhase() const { return mPhase; }

    void addDependency(System* dependency) {
        if (dependency && dependency != this) {
            mDependencies.push_back(dependency);
        }
    }
    const std::vector<System*>& getDependencies() const { return mDependencies; }
    void clearDependencies() { mDependencies.clear(); }

    void setRuntimeEnabled(bool enabled) { mRuntimeEnabled = enabled; }
    bool isRuntimeEnabled() const { return mRuntimeEnabled; }

    void setScheduler(SystemScheduler* scheduler) { mScheduler = scheduler; }
    SystemScheduler* getScheduler() const { return mScheduler; }

    void lock() { mMutex.lock(); }
    void unlock() { mMutex.unlock(); }
    bool tryLock() { return mMutex.try_lock(); }

private:
    bool mEnabled = true;
    bool mRuntimeEnabled = true;
    i32 mPriority = 0;
    std::string mName;
    SystemGroup mGroup = SystemGroup::Logic;
    SystemPhase mPhase = SystemPhase::Update;
    std::vector<System*> mDependencies;
    SystemScheduler* mScheduler = nullptr;
    std::mutex mMutex;
};

class SystemGroupContainer {
public:
    void addSystem(System* system);
    void removeSystem(System* system);
    void sort();

    std::vector<System*>& getSystems() { return mSystems; }
    const std::vector<System*>& getSystems() const { return mSystems; }

    void updateAll(f32 dt);
    void renderAll(f32 dt);

private:
    std::vector<System*> mSystems;
};

class DependencyGraph {
public:
    void addEdge(System* from, System* to);
    void removeEdge(System* from, System* to);
    std::vector<System*> topologicalSort(const std::vector<System*>& systems) const;
    bool hasCycle() const;
    void clear();

private:
    std::unordered_map<System*, std::vector<System*>> mAdjacency;
    bool dfs(System* node, std::unordered_set<System*>& visited,
             std::unordered_set<System*>& recStack) const;
};

class SystemManager {
public:
    SystemManager() = default;
    ~SystemManager() = default;

    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    template <typename T, typename... Args>
    T* registerSystem(Args&&... args);

    template <typename T>
    T* getSystem() const;

    template <typename T>
    void removeSystem();

    void updateAll(f32 dt, SystemGroup group);
    void renderAll(f32 dt, SystemGroup group);
    void runPhase(f32 dt, SystemPhase phase);

    void setSystemEnabled(const std::string& name, bool enabled);
    void setSystemEnabled(System* system, bool enabled);

    std::vector<System*> getSystemList() const;

    void executeDependencyOrdered(f32 dt);

    void clear();

private:
    std::unordered_map<size_t, std::unique_ptr<System>> mSystems;
    SystemGroupContainer mGroups[static_cast<size_t>(SystemGroup::Count)];
    DependencyGraph mDependencyGraph;
};

}
