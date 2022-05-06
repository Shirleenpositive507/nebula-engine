#pragma once
#include <vector>
#include <functional>
#include "core/Types.h"
#include "math/Vector2.h"
#include "Broadphase.h"
#include "Narrowphase.h"
#include "RigidBody.h"
#include "Collider.h"

namespace nebula {

struct RaycastHit {
    Vector2f point;
    Vector2f normal;
    f32 distance;
    Collider* collider;
    RigidBody* rigidBody;

    RaycastHit()
        : point(0, 0), normal(0, 0), distance(0)
        , collider(nullptr), rigidBody(nullptr) {}
};

class CollisionDetector {
public:
    Broadphase broadphase;
    Narrowphase narrowphase;

    CollisionDetector() = default;
    explicit CollisionDetector(f32 cellSize) : broadphase(cellSize) {}

    std::vector<CollisionInfo> detectCollisions(const std::vector<ColliderPair>& pairs);

    RaycastHit raycast(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    std::vector<RaycastHit> raycastAll(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    RaycastHit raycastFiltered(const Vector2f& origin, const Vector2f& direction,
                                f32 maxDistance, u32 layerMask);

    RaycastHit circleCast(const Vector2f& origin, f32 radius,
                           const Vector2f& direction, f32 maxDistance);
    RaycastHit rectCast(const Rectf& rect, const Vector2f& direction, f32 maxDistance);

    RaycastHit ccdRaycast(RigidBody* body, const Vector2f& origin, const Vector2f& direction,
                           f32 maxDistance);

private:
    bool rayAABBIntersect(const Vector2f& origin, const Vector2f& dir,
                           const Rectf& bounds, f32& tMin, f32& tMax);
    bool rayCircleIntersect(const Vector2f& origin, const Vector2f& dir,
                            const Vector2f& center, f32 radius, f32& t);
};

} // namespace nebula
