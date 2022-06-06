#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "../core/Types.h"

namespace nebula {

enum class SystemGroup {
    Logic = 0,
    Physics,
    Render,
    UI,
    PostProcess,
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

    void setEnabled(bool enabled) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    void setPriority(i32 priority) { mPriority = priority; }
    i32 getPriority() const { return mPriority; }

    void setName(const std::string& name) { mName = name; }
    const std::string& getName() const { return mName; }

    void setGroup(SystemGroup group) { mGroup = group; }
    SystemGroup getGroup() const { return mGroup; }

private:
    bool mEnabled = true;
    i32 mPriority = 0;
    std::string mName;
    SystemGroup mGroup = SystemGroup::Logic;
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

    void setSystemEnabled(const std::string& name, bool enabled);
    void setSystemEnabled(System* system, bool enabled);

    std::vector<System*> getSystemList() const;

    void clear();

private:
    std::unordered_map<size_t, std::unique_ptr<System>> mSystems;
    SystemGroupContainer mGroups[static_cast<size_t>(SystemGroup::Count)];
};

}
