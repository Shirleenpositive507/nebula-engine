#include "Broadphase.h"
#include <algorithm>
#include <set>
#include <cmath>

namespace nebula {

void Broadphase::insert(RigidBody* body) {
    i64 key = hashPosition(body->position);
    grid[key].push_back(body);
    bodyCells[body] = key;
}

void Broadphase::remove(RigidBody* body) {
    auto it = bodyCells.find(body);
    if (it == bodyCells.end()) return;
    i64 key = it->second;
    auto& bucket = grid[key];
    bucket.erase(std::remove(bucket.begin(), bucket.end(), body), bucket.end());
    if (bucket.empty()) grid.erase(key);
    bodyCells.erase(it);
}

void Broadphase::update(RigidBody* body) {
    i64 newKey = hashPosition(body->position);
    auto it = bodyCells.find(body);
    if (it != bodyCells.end() && it->second == newKey) return;
    remove(body);
    insert(body);
}

void Broadphase::clear() {
    grid.clear();
    bodyCells.clear();
}

void Broadphase::getCellRange(const Rectf& rect, i32& minCX, i32& minCY, i32& maxCX, i32& maxCY) const {
    minCX = static_cast<i32>(std::floor(rect.getMinX() / cellSize));
    minCY = static_cast<i32>(std::floor(rect.getMinY() / cellSize));
    maxCX = static_cast<i32>(std::floor(rect.getMaxX() / cellSize));
    maxCY = static_cast<i32>(std::floor(rect.getMaxY() / cellSize));
}

std::vector<ColliderPair> Broadphase::queryRange(const Rectf& rect) const {
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

} // namespace nebula
