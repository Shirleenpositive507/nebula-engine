#include "System.h"
#include "SystemScheduler.h"
#include <cassert>
#include <queue>

namespace nebula {

void DependencyGraph::addEdge(System* from, System* to) {
    mAdjacency[from].push_back(to);
}

void DependencyGraph::removeEdge(System* from, System* to) {
    auto it = mAdjacency.find(from);
    if (it != mAdjacency.end()) {
        auto& vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), to), vec.end());
    }
}

bool DependencyGraph::dfs(System* node, std::unordered_set<System*>& visited,
                          std::unordered_set<System*>& recStack) const {
    if (recStack.count(node)) return true;
    if (visited.count(node)) return false;

    visited.insert(node);
    recStack.insert(node);

    auto it = mAdjacency.find(node);
    if (it != mAdjacency.end()) {
        for (auto* neighbor : it->second) {
            if (dfs(neighbor, visited, recStack)) return true;
        }
    }

    recStack.erase(node);
    return false;
}

bool DependencyGraph::hasCycle() const {
    std::unordered_set<System*> visited;
    std::unordered_set<System*> recStack;

    for (const auto& pair : mAdjacency) {
        if (dfs(pair.first, visited, recStack)) return true;
    }
    return false;
}

std::vector<System*> DependencyGraph::topologicalSort(const std::vector<System*>& systems) const {
    std::unordered_map<System*, int> inDegree;
    for (auto* sys : systems) {
        inDegree[sys] = 0;
    }

    for (const auto& pair : mAdjacency) {
        for (auto* neighbor : pair.second) {
            inDegree[neighbor]++;
        }
    }

    std::queue<System*> q;
    for (auto* sys : systems) {
        if (inDegree[sys] == 0) {
            q.push(sys);
        }
    }

    std::vector<System*> result;
    while (!q.empty()) {
        auto* sys = q.front();
        q.pop();
        result.push_back(sys);

        auto it = mAdjacency.find(sys);
        if (it != mAdjacency.end()) {
            for (auto* neighbor : it->second) {
                if (--inDegree[neighbor] == 0) {
                    q.push(neighbor);
                }
            }
        }
    }

    return result;
}

void DependencyGraph::clear() {
    mAdjacency.clear();
}

void SystemGroupContainer::addSystem(System* system) {
    mSystems.push_back(system);
}

void SystemGroupContainer::removeSystem(System* system) {
    auto it = std::find(mSystems.begin(), mSystems.end(), system);
    if (it != mSystems.end()) {
        mSystems.erase(it);
    }
}

void SystemGroupContainer::sort() {
    std::sort(mSystems.begin(), mSystems.end(),
        [](System* a, System* b) {
            return a->getPriority() < b->getPriority();
        });
}

void SystemGroupContainer::updateAll(f32 dt) {
    for (auto* system : mSystems) {
        if (system->isEnabled() && system->isRuntimeEnabled()) {
            system->update(dt);
        }
    }
}

void SystemGroupContainer::renderAll(f32 dt) {
    for (auto* system : mSystems) {
        if (system->isEnabled() && system->isRuntimeEnabled()) {
            system->render(dt);
        }
    }
}

template <typename T, typename... Args>
T* SystemManager::registerSystem(Args&&... args) {
    size_t hash = typeid(T).hash_code();
    auto it = mSystems.find(hash);
    if (it != mSystems.end()) {
        return static_cast<T*>(it->second.get());
    }

    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = system.get();

    size_t groupIdx = static_cast<size_t>(ptr->getGroup());
    mGroups[groupIdx].addSystem(ptr);
    mGroups[groupIdx].sort();

    for (auto* dep : ptr->getDependencies()) {
        mDependencyGraph.addEdge(dep, ptr);
    }

    mSystems[hash] = std::move(system);
    return ptr;
}
}

template <typename T>
T* SystemManager::getSystem() const {
    size_t hash = typeid(T).hash_code();
    auto it = mSystems.find(hash);
    if (it != mSystems.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template <typename T>
void SystemManager::removeSystem() {
    size_t hash = typeid(T).hash_code();
    auto it = mSystems.find(hash);
    if (it != mSystems.end()) {
        System* sys = it->second.get();
        size_t groupIdx = static_cast<size_t>(sys->getGroup());
        mGroups[groupIdx].removeSystem(sys);
        mSystems.erase(it);
    }
}

void SystemManager::updateAll(f32 dt, SystemGroup group) {
    size_t idx = static_cast<size_t>(group);
    mGroups[idx].updateAll(dt);
}

void SystemManager::renderAll(f32 dt, SystemGroup group) {
    size_t idx = static_cast<size_t>(group);
    mGroups[idx].renderAll(dt);
}

void SystemManager::runPhase(f32 dt, SystemPhase phase) {
    for (auto& pair : mSystems) {
        if (pair.second->isEnabled() && pair.second->isRuntimeEnabled()
            && pair.second->getPhase() == phase) {
            pair.second->phase(dt, phase);
        }
    }
}

void SystemManager::setSystemEnabled(const std::string& name, bool enabled) {
    for (auto& pair : mSystems) {
        if (pair.second->getName() == name) {
            pair.second->setEnabled(enabled);
            return;
        }
    }
}

void SystemManager::setSystemEnabled(System* system, bool enabled) {
    system->setEnabled(enabled);
}

std::vector<System*> SystemManager::getSystemList() const {
    std::vector<System*> result;
    result.reserve(mSystems.size());
    for (const auto& pair : mSystems) {
        result.push_back(pair.second.get());
    }
    return result;
}

void SystemManager::executeDependencyOrdered(f32 dt) {
    auto allSystems = getSystemList();
    auto sorted = mDependencyGraph.topologicalSort(allSystems);

    for (auto* sys : sorted) {
        if (sys->isEnabled() && sys->isRuntimeEnabled()) {
            sys->update(dt);
        }
    }
}

void SystemManager::clear() {
    mSystems.clear();
    mDependencyGraph.clear();
    for (size_t i = 0; i < static_cast<size_t>(SystemGroup::Count); ++i) {
        mGroups[i].getSystems().clear();
    }
}

}
