#pragma once

#include "System.h"
#include "../core/TaskSystem.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace nebula {

struct ScheduledSystem {
    System* system;
    SystemPhase phase;
    int order;
    std::vector<System*> dependencies;
    std::vector<ScheduledSystem*> dependents;
    bool visited;
    bool inProgress;

    ScheduledSystem() : system(nullptr), phase(SystemPhase::Update), order(0), visited(false), inProgress(false) {}
};

class SystemScheduler {
public:
    SystemScheduler();
    ~SystemScheduler();

    void setTaskSystem(TaskSystem* taskSystem) { m_taskSystem = taskSystem; }
    TaskSystem* getTaskSystem() const { return m_taskSystem; }

    void scheduleSystem(System* system);
    void removeSystem(System* system);
    void clear();

    void executePhase(f32 dt, SystemPhase phase);
    void executeAll(f32 dt);

    void setParallelExecution(bool enabled) { m_parallel = enabled; }
    bool isParallelExecution() const { return m_parallel; }

    std::size_t getScheduledCount() const { return m_scheduled.size(); }

    void setProfilerEnabled(bool enabled) { m_profilerEnabled = enabled; }
    bool isProfilerEnabled() const { return m_profilerEnabled; }

private:
    void topologicalSort();
    void executeSystem(ScheduledSystem* scheduled, f32 dt);
    void executeGroup(const std::vector<ScheduledSystem*>& group, f32 dt);

    TaskSystem* m_taskSystem;
    std::vector<std::unique_ptr<ScheduledSystem>> m_scheduled;
    std::unordered_map<System*, ScheduledSystem*> m_systemMap;
    std::vector<std::vector<ScheduledSystem*>> m_executionGroups;
    bool m_parallel;
    bool m_profilerEnabled;
    bool m_dirty;
    std::mutex m_mutex;
};

}

