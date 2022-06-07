#pragma once

#include "EntityManager.h"

template <typename T, typename... Args>
T& EntityHandle::addComponent(Args&&... args) {
    return mManager->addComponent<T>(mEntity, std::forward<Args>(args)...);
}

template <typename T>
void EntityHandle::removeComponent() {
    mManager->removeComponent<T>(mEntity);
}

template <typename T>
T* EntityHandle::getComponent() {
    return mManager->getComponent<T>(mEntity);
}

template <typename T>
bool EntityHandle::hasComponent() {
    return mManager->hasComponent<T>(mEntity);
}
