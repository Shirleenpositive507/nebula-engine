#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>
#include <mutex>
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

struct DBVTNode {
    Rectf aabb;
    RigidBody* body;
    std::unique_ptr<DBVTNode> left;
    std::unique_ptr<DBVTNode> right;
    DBVTNode* parent;
    i32 height;
    bool isLeaf;

    DBVTNode()
        : body(nullptr)
        , parent(nullptr)
        , height(1)
        , isLeaf(true) {}
};

struct BroadphaseStats {
    std::size_t totalPairs;
    std::size_t potentialPairs;
    std::size_t cellCount;
    std::size_t dbvtNodes;
    i32 dbvtHeight;
    f32 averagePairsPerCell;
    std::size_t queryTime;

    BroadphaseStats()
        : totalPairs(0)
        , potentialPairs(0)
        , cellCount(0)
        , dbvtNodes(0)
        , dbvtHeight(0)
        , averagePairsPerCell(0.0f)
        , queryTime(0) {}
};

class Broadphase {
public:
    f32 cellSize;
    std::unordered_map<i64, std::vector<RigidBody*>> grid;
    std::unordered_map<RigidBody*, i64> bodyCells;
    bool useDBVT;
    bool useMultiThreading;

    Broadphase() : cellSize(2.0f), useDBVT(false), useMultiThreading(false) {}
    explicit Broadphase(f32 cellSize)
        : cellSize(cellSize), useDBVT(false), useMultiThreading(false) {}

    void insert(RigidBody* body);
    void remove(RigidBody* body);
    void update(RigidBody* body);
    void clear();
    std::vector<ColliderPair> queryRange(const Rectf& rect) const;
    std::vector<RigidBody*> queryPoint(const Vector2f& point) const;
    std::vector<ColliderPair> queryRay(const Vector2f& origin, const Vector2f& direction, f32 maxDist) const;
    std::vector<ColliderPair> getPotentialCollisions() const;

    void autoConfigureCellSize();
    void setUseDBVT(bool enabled);
    void setUseMultiThreading(bool enabled);

    BroadphaseStats getStats() const;

    void rebuildDBVT();

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

    std::unique_ptr<DBVTNode> m_dbvtRoot;
    std::unordered_map<RigidBody*, DBVTNode*> m_dbvtLeaves;
    mutable std::mutex m_mutex;

    DBVTNode* createLeaf(RigidBody* body);
    void removeLeaf(DBVTNode* leaf);
    DBVTNode* insertLeaf(DBVTNode* root, DBVTNode* leaf);
    DBVTNode* removeLeafFromTree(DBVTNode* root, DBVTNode* leaf);
    void updateDBVT(RigidBody* body);
    void queryDBVT(const Rectf& rect, std::vector<ColliderPair>& pairs) const;
    void queryDBVTRecursive(DBVTNode* node, const Rectf& rect, std::vector<ColliderPair>& pairs) const;
    void collectLeaves(DBVTNode* node, std::vector<RigidBody*>& leaves) const;
    void syncDBVTToGrid();
    i32 getHeight(DBVTNode* node) const;
    i32 getBalance(DBVTNode* node) const;
    f32 getPerimeter(const Rectf& rect) const;
    Rectf combineRect(const Rectf& a, const Rectf& b) const;
};

} // namespace nebula
