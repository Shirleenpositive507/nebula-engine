#pragma once
#include <vector>
#include <unordered_map>
#include <utility>
#include <functional>
#include "core/Types.h"
#include "math/Vector2.h"
#include "math/Rect.h"
#include "RigidBody.h"

namespace nebula {

struct ColliderPair {
    RigidBody* bodyA;
    RigidBody* bodyB;

    ColliderPair() : bodyA(nullptr), bodyB(nullptr) {}
    ColliderPair(RigidBody* a, RigidBody* b) : bodyA(a), bodyB(b) {}

    bool operator==(const ColliderPair& other) const {
        return (bodyA == other.bodyA && bodyB == other.bodyB) ||
               (bodyA == other.bodyB && bodyB == other.bodyA);
    }
};

struct ColliderPairHash {
    size_t operator()(const ColliderPair& p) const {
        size_t a = reinterpret_cast<size_t>(p.bodyA);
        size_t b = reinterpret_cast<size_t>(p.bodyB);
        return a ^ (b << 1);
    }
};

class Broadphase {
public:
    f32 cellSize;
    std::unordered_map<i64, std::vector<RigidBody*>> grid;
    std::unordered_map<RigidBody*, i64> bodyCells;

    Broadphase() : cellSize(2.0f) {}
    explicit Broadphase(f32 cellSize) : cellSize(cellSize) {}

    void insert(RigidBody* body);
    void remove(RigidBody* body);
    void update(RigidBody* body);
    void clear();
    std::vector<ColliderPair> queryRange(const Rectf& rect) const;
    std::vector<RigidBody*> queryPoint(const Vector2f& point) const;
    std::vector<ColliderPair> queryRay(const Vector2f& origin, const Vector2f& direction, f32 maxDist) const;
    std::vector<ColliderPair> getPotentialCollisions() const;

private:
    i64 hashCell(i32 cx, i32 cy) const {
        return (static_cast<i64>(cx) << 32) | static_cast<i64>(cy);
    }

    i64 hashPosition(const Vector2f& pos) const {
        i32 cx = static_cast<i32>(std::floor(pos.x / cellSize));
        i32 cy = static_cast<i32>(std::floor(pos.y / cellSize));
        return hashCell(cx, cy);
    }

    void getCellRange(const Rectf& rect, i32& minCX, i32& minCY, i32& maxCX, i32& maxCY) const;
};

} // namespace nebula
