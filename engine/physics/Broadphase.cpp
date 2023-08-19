#include "Broadphase.h"
#include <algorithm>
#include <set>
#include <cmath>
#include <limits>
#include <thread>

namespace nebula {

void Broadphase::insert(RigidBody* body) {
    i64 key = hashPosition(body->position);
    grid[key].push_back(body);
    bodyCells[body] = key;

    if (useDBVT) {
        DBVTNode* leaf = createLeaf(body);
        m_dbvtRoot = insertLeaf(std::move(m_dbvtRoot), leaf);
    }
}

void Broadphase::remove(RigidBody* body) {
    auto it = bodyCells.find(body);
    if (it == bodyCells.end()) return;
    i64 key = it->second;
    auto& bucket = grid[key];
    bucket.erase(std::remove(bucket.begin(), bucket.end(), body), bucket.end());
    if (bucket.empty()) grid.erase(key);
    bodyCells.erase(it);

    if (useDBVT) {
        auto leafIt = m_dbvtLeaves.find(body);
        if (leafIt != m_dbvtLeaves.end()) {
            m_dbvtRoot = removeLeafFromTree(std::move(m_dbvtRoot), leafIt->second);
            m_dbvtLeaves.erase(leafIt);
        }
    }
}

void Broadphase::update(RigidBody* body) {
    i64 newKey = hashPosition(body->position);
    auto it = bodyCells.find(body);
    if (it != bodyCells.end() && it->second == newKey) {
        if (useDBVT) {
            updateDBVT(body);
        }
        return;
    }
    remove(body);
    insert(body);
}

void Broadphase::clear() {
    grid.clear();
    bodyCells.clear();
    m_dbvtRoot.reset();
    m_dbvtLeaves.clear();
}

void Broadphase::autoConfigureCellSize() {
    if (bodies.size() < 2) return;

    f32 totalWidth = 0.0f;
    f32 totalHeight = 0.0f;
    i32 count = 0;

    for (const auto& [key, bucket] : grid) {
        for (auto* body : bucket) {
            if (body->collider) {
                Rectf bounds = body->collider->getBounds();
                totalWidth += bounds.width;
                totalHeight += bounds.height;
                count++;
            }
        }
    }

    if (count > 0) {
        f32 avgWidth = totalWidth / count;
        f32 avgHeight = totalHeight / count;
        cellSize = std::max(avgWidth, avgHeight) * 2.0f;
    }

    clear();
    for (auto& body : bodies) {
        insert(body);
    }
}

void Broadphase::setUseDBVT(bool enabled) {
    useDBVT = enabled;
    if (enabled) {
        rebuildDBVT();
    } else {
        m_dbvtRoot.reset();
        m_dbvtLeaves.clear();
    }
}

void Broadphase::setUseMultiThreading(bool enabled) {
    useMultiThreading = enabled;
}

BroadphaseStats Broadphase::getStats() const {
    BroadphaseStats stats;
    stats.cellCount = grid.size();
    stats.dbvtNodes = m_dbvtLeaves.size();
    stats.dbvtHeight = getHeight(m_dbvtRoot.get());

    std::size_t totalPairs = 0;
    for (const auto& [key, bucket] : grid) {
        totalPairs += bucket.size();
    }
    stats.totalPairs = totalPairs;

    if (stats.cellCount > 0) {
        stats.averagePairsPerCell = static_cast<f32>(totalPairs) / stats.cellCount;
    }

    return stats;
}

void Broadphase::rebuildDBVT() {
    m_dbvtRoot.reset();
    m_dbvtLeaves.clear();

    std::vector<RigidBody*> sortedBodies(bodies.begin(), bodies.end());
    std::sort(sortedBodies.begin(), sortedBodies.end(),
        [](RigidBody* a, RigidBody* b) {
            return a->position.x < b->position.x;
        });

    for (auto& body : sortedBodies) {
        DBVTNode* leaf = createLeaf(body);
        m_dbvtRoot = insertLeaf(std::move(m_dbvtRoot), leaf);
    }
}

void Broadphase::getCellRange(const Rectf& rect, i32& minCX, i32& minCY, i32& maxCX, i32& maxCY) const {
    minCX = static_cast<i32>(std::floor(rect.getMinX() / cellSize));
    minCY = static_cast<i32>(std::floor(rect.getMinY() / cellSize));
    maxCX = static_cast<i32>(std::floor(rect.getMaxX() / cellSize));
    maxCY = static_cast<i32>(std::floor(rect.getMaxY() / cellSize));
}

std::vector<ColliderPair> Broadphase::queryRange(const Rectf& rect) const {
    if (useDBVT) {
        std::vector<ColliderPair> pairs;
        queryDBVT(rect, pairs);
        return pairs;
    }

    i32 minCX, minCY, maxCX, maxCY;
    getCellRange(rect, minCX, minCY, maxCX, maxCY);

    std::set<RigidBody*> candidates;
    for (i32 cx = minCX; cx <= maxCX; ++cx) {
        for (i32 cy = minCY; cy <= maxCY; ++cy) {
            i64 key = hashCell(cx, cy);
            auto it = grid.find(key);
            if (it != grid.end()) {
                for (auto* body : it->second) {
                    candidates.insert(body);
                }
            }
        }
    }

    std::vector<ColliderPair> result;
    std::vector<RigidBody*> list(candidates.begin(), candidates.end());
    for (size_t i = 0; i < list.size(); ++i) {
        for (size_t j = i + 1; j < list.size(); ++j) {
            result.emplace_back(list[i], list[j]);
        }
    }
    return result;
}

std::vector<RigidBody*> Broadphase::queryPoint(const Vector2f& point) const {
    if (useDBVT) {
        Rectf smallRect(point.x - 0.001f, point.y - 0.001f, 0.002f, 0.002f);
        std::vector<ColliderPair> pairs;
        queryDBVT(smallRect, pairs);
        std::set<RigidBody*> bodies;
        for (const auto& pair : pairs) {
            bodies.insert(pair.bodyA);
            bodies.insert(pair.bodyB);
        }
        return std::vector<RigidBody*>(bodies.begin(), bodies.end());
    }

    i64 key = hashPosition(point);
    auto it = grid.find(key);
    if (it == grid.end()) return {};
    return it->second;
}

std::vector<ColliderPair> Broadphase::queryRay(const Vector2f& origin, const Vector2f& direction, f32 maxDist) const {
    Vector2f end = origin + direction.normalized() * maxDist;
    Rectf rayRect(
        std::min(origin.x, end.x), std::min(origin.y, end.y),
        std::abs(end.x - origin.x), std::abs(end.y - origin.y)
    );
    return queryRange(rayRect);
}

std::vector<ColliderPair> Broadphase::getPotentialCollisions() const {
    if (useDBVT) {
        std::vector<ColliderPair> pairs;
        if (m_dbvtRoot) {
            queryDBVTRecursive(m_dbvtRoot.get(), Rectf(0, 0, 0, 0), pairs);
        }
        return pairs;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    std::set<ColliderPair, std::function<bool(const ColliderPair&, const ColliderPair&)>>
        uniquePairs([](const ColliderPair& a, const ColliderPair& b) {
            return (a.bodyA < b.bodyA) || (a.bodyA == b.bodyA && a.bodyB < b.bodyB);
        });

    for (const auto& [key, bucket] : grid) {
        for (size_t i = 0; i < bucket.size(); ++i) {
            for (size_t j = i + 1; j < bucket.size(); ++j) {
                RigidBody* a = bucket[i];
                RigidBody* b = bucket[j];
                ColliderPair pair(a, b);
                if (uniquePairs.find(pair) == uniquePairs.end()) {
                    uniquePairs.insert(pair);
                }
            }
        }
    }

    std::set<ColliderPair> neighborPairs;
    for (const auto& [key, bucket] : grid) {
        for (auto* body : bucket) {
            Vector2f pos = body->position;
            for (i32 dx = -1; dx <= 1; ++dx) {
                for (i32 dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    i64 neighborKey = hashCell(
                        static_cast<i32>(std::floor(pos.x / cellSize)) + dx,
                        static_cast<i32>(std::floor(pos.y / cellSize)) + dy
                    );
                    auto it = grid.find(neighborKey);
                    if (it != grid.end()) {
                        for (auto* other : it->second) {
                            if (body != other) {
                                ColliderPair pair(body, other);
                                if (neighborPairs.find(pair) == neighborPairs.end()) {
                                    neighborPairs.insert(pair);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<ColliderPair> result;
    for (const auto& pair : uniquePairs) {
        if (pair.bodyA->isStatic() && pair.bodyB->isStatic()) continue;
        result.push_back(pair);
    }
    for (const auto& pair : neighborPairs) {
        if (pair.bodyA->isStatic() && pair.bodyB->isStatic()) continue;
        bool found = false;
        for (const auto& existing : result) {
            if (existing == pair) { found = true; break; }
        }
        if (!found) result.push_back(pair);
    }
    return result;
}

DBVTNode* Broadphase::createLeaf(RigidBody* body) {
    auto leaf = std::make_unique<DBVTNode>();
    leaf->body = body;
    leaf->isLeaf = true;
    leaf->height = 1;
    leaf->left = nullptr;
    leaf->right = nullptr;
    leaf->parent = nullptr;

    if (body->collider) {
        leaf->aabb = body->collider->getBounds();
        leaf->aabb.offset(body->position);
    } else {
        leaf->aabb = Rectf(body->position.x, body->position.y, 0.1f, 0.1f);
    }

    DBVTNode* raw = leaf.get();
    m_dbvtLeaves[body] = raw;
    return raw;
}

void Broadphase::removeLeaf(DBVTNode* leaf) {
    (void)leaf;
}

DBVTNode* Broadphase::insertLeaf(DBVTNode* root, DBVTNode* leaf) {
    if (!root) {
        auto newRoot = std::make_unique<DBVTNode>();
        newRoot->aabb = leaf->aabb;
        newRoot->height = 2;
        newRoot->isLeaf = false;
        newRoot->body = nullptr;

        leaf->parent = newRoot.get();
        return newRoot.release();
    }

    if (root->isLeaf) {
        auto newRoot = std::make_unique<DBVTNode>();
        newRoot->aabb = combineRect(root->aabb, leaf->aabb);
        newRoot->height = 2;
        newRoot->isLeaf = false;
        newRoot->body = nullptr;

        root->parent = newRoot.get();
        leaf->parent = newRoot.get();

        newRoot->left.reset(root);
        newRoot->right.reset(leaf);
        return newRoot.release();
    }

    Rectf combined = combineRect(root->aabb, leaf->aabb);
    f32 rootCost = combined.getWidth() * combined.getHeight();

    f32 leftCost = getPerimeter(combineRect(root->left->aabb, leaf->aabb));
    f32 rightCost = getPerimeter(combineRect(root->right->aabb, leaf->aabb));

    if (leftCost < rightCost) {
        root->aabb = combined;
        root->height = 1 + std::max(root->left->height, root->right->height);
        root->left.reset(insertLeaf(root->left.release(), leaf));
    } else {
        root->aabb = combined;
        root->height = 1 + std::max(root->left->height, root->right->height);
        root->right.reset(insertLeaf(root->right.release(), leaf));
    }

    return root;
}

DBVTNode* Broadphase::removeLeafFromTree(DBVTNode* root, DBVTNode* leaf) {
    if (!root || !leaf) return root;

    if (root == leaf) return nullptr;

    DBVTNode* parent = leaf->parent;
    if (!parent) return root;

    DBVTNode* sibling = (parent->left.get() == leaf) ?
        parent->right.release() : parent->left.release();

    DBVTNode* grandParent = parent->parent;
    sibling->parent = grandParent;

    if (grandParent) {
        if (grandParent->left.get() == parent) {
            grandParent->left.reset(sibling);
        } else {
            grandParent->right.reset(sibling);
        }

        DBVTNode* node = grandParent;
        while (node) {
            node->aabb = combineRect(node->left->aabb, node->right->aabb);
            node->height = 1 + std::max(node->left->height, node->right->height);
            node = node->parent;
        }
    } else {
        return sibling;
    }

    return root;
}

void Broadphase::updateDBVT(RigidBody* body) {
    auto it = m_dbvtLeaves.find(body);
    if (it == m_dbvtLeaves.end()) return;

    DBVTNode* leaf = it->second;
    Rectf newAABB;
    if (body->collider) {
        newAABB = body->collider->getBounds();
        newAABB.offset(body->position);
    } else {
        newAABB = Rectf(body->position.x, body->position.y, 0.1f, 0.1f);
    }

    if (newAABB.getMinX() >= leaf->aabb.getMinX() &&
        newAABB.getMaxX() <= leaf->aabb.getMaxX() &&
        newAABB.getMinY() >= leaf->aabb.getMinY() &&
        newAABB.getMaxY() <= leaf->aabb.getMaxY()) {
        return;
    }

    leaf->aabb = newAABB;
    m_dbvtRoot = removeLeafFromTree(m_dbvtRoot.release(), leaf);
    m_dbvtRoot = insertLeaf(m_dbvtRoot.release(), leaf);
}

void Broadphase::queryDBVT(const Rectf& rect, std::vector<ColliderPair>& pairs) const {
    if (m_dbvtRoot) {
        queryDBVTRecursive(m_dbvtRoot.get(), rect, pairs);
    }
}

void Broadphase::queryDBVTRecursive(DBVTNode* node, const Rectf& rect, std::vector<ColliderPair>& pairs) const {
    if (!node) return;

    Rectf nodeBounds = node->aabb;

    if (node->isLeaf) {
        if (nodeBounds.intersects(rect) || (rect.width == 0 && rect.height == 0)) {
            std::vector<RigidBody*> allLeaves;
            collectLeaves(node, allLeaves);
            for (size_t i = 0; i < allLeaves.size(); ++i) {
                for (size_t j = i + 1; j < allLeaves.size(); ++j) {
                    RigidBody* a = allLeaves[i];
                    RigidBody* b = allLeaves[j];
                    if (a->isStatic() && b->isStatic()) continue;

                    Rectf boundsA, boundsB;
                    if (a->collider) { boundsA = a->collider->getBounds(); boundsA.offset(a->position); }
                    if (b->collider) { boundsB = b->collider->getBounds(); boundsB.offset(b->position); }

                    if (boundsA.intersects(boundsB)) {
                        pairs.emplace_back(a, b);
                    }
                }
            }
        }
        return;
    }

    if (rect.width == 0 && rect.height == 0) {
        if (node->left) queryDBVTRecursive(node->left.get(), rect, pairs);
        if (node->right) queryDBVTRecursive(node->right.get(), rect, pairs);
        return;
    }

    if (nodeBounds.intersects(rect)) {
        if (node->left) queryDBVTRecursive(node->left.get(), rect, pairs);
        if (node->right) queryDBVTRecursive(node->right.get(), rect, pairs);
    }
}

void Broadphase::syncDBVTToGrid() {
    (void)0;
}

void Broadphase::collectLeaves(DBVTNode* node, std::vector<RigidBody*>& leaves) const {
    if (!node) return;
    if (node->isLeaf && node->body) {
        leaves.push_back(node->body);
    }
    if (node->left) collectLeaves(node->left.get(), leaves);
    if (node->right) collectLeaves(node->right.get(), leaves);
}

i32 Broadphase::getHeight(DBVTNode* node) const {
    if (!node) return 0;
    return node->height;
}

i32 Broadphase::getBalance(DBVTNode* node) const {
    if (!node || node->isLeaf) return 0;
    return getHeight(node->right.get()) - getHeight(node->left.get());
}

f32 Broadphase::getPerimeter(const Rectf& rect) const {
    return 2.0f * (rect.width + rect.height);
}

Rectf Broadphase::combineRect(const Rectf& a, const Rectf& b) const {
    f32 minX = std::min(a.getMinX(), b.getMinX());
    f32 minY = std::min(a.getMinY(), b.getMinY());
    f32 maxX = std::max(a.getMaxX(), b.getMaxX());
    f32 maxY = std::max(a.getMaxY(), b.getMaxY());
    return Rectf(minX, minY, maxX - minX, maxY - minY);
}

} // namespace nebula
