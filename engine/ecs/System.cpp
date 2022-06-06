#include "System.h"
#include <cassert>

namespace nebula {

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
        if (system->isEnabled()) {
            system->update(dt);
        }
    }
}

void SystemGroupContainer::renderAll(f32 dt) {
    for (auto* system : mSystems) {
        if (system->isEnabled()) {
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

    mSystems[hash] = std::move(system);
    return ptr;
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

void SystemManager::clear() {
    mSystems.clear();
    for (size_t i = 0; i < static_cast<size_t>(SystemGroup::Count); ++i) {
        mGroups[i].getSystems().clear();
    }
}

}
