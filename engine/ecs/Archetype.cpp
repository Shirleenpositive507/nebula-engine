#include "Archetype.h"
#include <algorithm>

namespace nebula {

ComponentMask ArchetypeGraph::buildMask(const std::vector<ComponentType>& types) const {
    ComponentMask mask;
    for (auto type : types) {
        mask.set(type);
    }
    return mask;
}

Archetype* ArchetypeGraph::getOrCreateArchetype(const std::vector<ComponentType>& types) {
    ComponentMask mask = buildMask(types);
    auto it = mArchetypeMap.find(mask);
    if (it != mArchetypeMap.end()) {
        return it->second;
    }

    auto arch = std::make_unique<Archetype>(types);
    Archetype* ptr = arch.get();
    mArchetypes.push_back(std::move(arch));
    mArchetypeMap[mask] = ptr;

    for (u32 i = 0; i < 4; ++i) {
        auto chunk = new ComponentChunk(sizeof(ComponentType) * types.size());
        ptr->addChunk(chunk);
    }

    return ptr;
}

Archetype* ArchetypeGraph::addComponent(Archetype* src, ComponentType type) {
    if (!src) return nullptr;
    if (src->hasComponent(type)) return src;

    auto types = src->getComponentTypes();
    types.push_back(type);

    Archetype* dst = getOrCreateArchetype(types);

    ArchetypeEdge edge;
    edge.added = type;
    edge.removed = INVALID_COMPONENT_TYPE;
    edge.target = dst;

    bool found = false;
    for (auto& e : mTransitions[src]) {
        if (e.target == dst) {
            found = true;
            break;
        }
    }
    if (!found) {
        mTransitions[src].push_back(edge);
    }

    return dst;
}

Archetype* ArchetypeGraph::removeComponent(Archetype* src, ComponentType type) {
    if (!src) return nullptr;
    if (!src->hasComponent(type)) return src;

    auto types = src->getComponentTypes();
    auto it = std::find(types.begin(), types.end(), type);
    if (it != types.end()) {
        types.erase(it);
    }

    Archetype* dst = getOrCreateArchetype(types);

    ArchetypeEdge edge;
    edge.added = INVALID_COMPONENT_TYPE;
    edge.removed = type;
    edge.target = dst;

    bool found = false;
    for (auto& e : mTransitions[src]) {
        if (e.target == dst) {
            found = true;
            break;
        }
    }
    if (!found) {
        mTransitions[src].push_back(edge);
    }

    return dst;
}

bool ArchetypeGraph::hasTransition(Archetype* src, Archetype* dst) const {
    auto it = mTransitions.find(src);
    if (it == mTransitions.end()) return false;

    for (const auto& edge : it->second) {
        if (edge.target == dst) return true;
    }
    return false;
}

std::vector<Archetype*> ArchetypeGraph::findTransitionPath(Archetype* src, Archetype* dst) const {
    if (!src || !dst) return {};

    std::unordered_map<Archetype*, Archetype*> cameFrom;
    std::queue<Archetype*> q;
    std::unordered_set<Archetype*> visited;

    q.push(src);
    visited.insert(src);

    while (!q.empty()) {
        Archetype* current = q.front();
        q.pop();

        if (current == dst) {
            std::vector<Archetype*> path;
            Archetype* node = dst;
            while (node != src) {
                path.push_back(node);
                node = cameFrom[node];
            }
            path.push_back(src);
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto it = mTransitions.find(current);
        if (it != mTransitions.end()) {
            for (const auto& edge : it->second) {
                if (!visited.count(edge.target)) {
                    visited.insert(edge.target);
                    cameFrom[edge.target] = current;
                    q.push(edge.target);
                }
            }
        }
    }

    return {};
}

void ArchetypeGraph::cleanupEmptyArchetypes() {
    for (auto it = mArchetypes.begin(); it != mArchetypes.end(); ) {
        Archetype* arch = it->get();
        if (arch->isEmpty()) {
            mArchetypeMap.erase(arch->getMask());
            mTransitions.erase(arch);
            it = mArchetypes.erase(it);
        } else {
            ++it;
        }
    }
}

void ArchetypeGraph::clear() {
    mArchetypes.clear();
    mArchetypeMap.clear();
    mTransitions.clear();
}

}
