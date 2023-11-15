#include "SystemScheduler.h"
#include <algorithm>
#include <queue>

namespace nebula {

SystemScheduler::SystemScheduler()
    : m_taskSystem(nullptr)
    , m_parallel(true)
    , m_profilerEnabled(false)
    , m_dirty(false)
{
}

SystemScheduler::~SystemScheduler() {
    clear();
}

void SystemScheduler::scheduleSystem(System* system) {
    if (!system) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_systemMap.find(system) != m_systemMap.end()) return;

    auto scheduled = std::make_unique<ScheduledSystem>();
    scheduled->system = system;
    scheduled->phase = system->getPhase();
    scheduled->order = system->getPriority();

    for (auto* dep : system->getDependencies()) {
        scheduled->dependencies.push_back(dep);
    }

    ScheduledSystem* ptr = scheduled.get();
    m_systemMap[system] = ptr;
    m_scheduled.push_back(std::move(scheduled));
    m_dirty = true;
}

void SystemScheduler::removeSystem(System* system) {
    if (!system) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_systemMap.find(system);
    if (it == m_systemMap.end()) return;

    m_scheduled.erase(
        std::remove_if(m_scheduled.begin(), m_scheduled.end(),
            [system](const std::unique_ptr<ScheduledSystem>& s) {
                return s->system == system;
            }),
        m_scheduled.end()
    );
    m_systemMap.erase(it);
    m_dirty = true;
}

void SystemScheduler::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scheduled.clear();
    m_systemMap.clear();
    m_executionGroups.clear();
    m_dirty = false;
}

void SystemScheduler::topologicalSort() {
    if (!m_dirty) return;
    m_executionGroups.clear();

    std::unordered_map<ScheduledSystem*, int> inDegree;
    for (const auto& s : m_scheduled) {
        inDegree[s.get()] = static_cast<int>(s->dependencies.size());
    }

    std::queue<ScheduledSystem*> q;
    for (const auto& s : m_scheduled) {
        if (inDegree[s.get()] == 0) {
            q.push(s.get());
        }
    }

    while (!q.empty()) {
        std::vector<ScheduledSystem*> currentGroup;
        std::queue<ScheduledSystem*> nextQ;

        while (!q.empty()) {
            auto* sys = q.front();
            q.pop();
            currentGroup.push_back(sys);
        }

        for (auto* sys : currentGroup) {
            for (auto* dep : sys->dependencies) {
                auto it = m_systemMap.find(dep);
                if (it != m_systemMap.end()) {
                    ScheduledSystem* depSys = it->second;
                    depSys->dependents.push_back(sys);
                }
            }
        }

        for (auto* sys : currentGroup) {
            for (auto* dependent : sys->dependents) {
                if (--inDegree[dependent] == 0) {
                    nextQ.push(dependent);
                }
            }
        }

        if (!currentGroup.empty()) {
            m_executionGroups.push_back(currentGroup);
        }
        q = nextQ;
    }

    m_dirty = false;
}

void SystemScheduler::executeSystem(ScheduledSystem* scheduled, f32 dt) {
    if (!scheduled || !scheduled->system) return;
    if (!scheduled->system->isEnabled() || !scheduled->system->isRuntimeEnabled()) return;

    if (m_profilerEnabled) {
        NEBULA_PROFILE_SCOPE(scheduled->system->getName());
    }

    scheduled->system->update(dt);
}

void SystemScheduler::executeGroup(const std::vector<ScheduledSystem*>& group, f32 dt) {
    if (m_parallel && m_taskSystem && group.size() > 1) {
        for (auto* scheduled : group) {
            m_taskSystem->submit(TaskPriority::Normal, [this, scheduled, dt]() {
                executeSystem(scheduled, dt);
            });
        }
        m_taskSystem->waitForAll();
    } else {
        for (auto* scheduled : group) {
            executeSystem(scheduled, dt);
        }
    }
}

void SystemScheduler::executePhase(f32 dt, SystemPhase phase) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_dirty) topologicalSort();

    for (auto& group : m_executionGroups) {
        std::vector<ScheduledSystem*> phaseGroup;
        for (auto* s : group) {
            if (s->phase == phase) {
                phaseGroup.push_back(s);
            }
        }
        if (!phaseGroup.empty()) {
            executeGroup(phaseGroup, dt);
        }
    }
}

void SystemScheduler::executeAll(f32 dt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_dirty) topologicalSort();

    for (auto& group : m_executionGroups) {
        executeGroup(group, dt);
    }
}

}

