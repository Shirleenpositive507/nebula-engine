#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/Types.h"
#include "math/Vector2.h"
#include "math/Rect.h"
#include "Collider.h"

namespace nebula {

struct ContactInfo {
    Vector2f point;
    Vector2f normal;
    f32 penetration;

    ContactInfo() : point(0, 0), normal(0, 0), penetration(0) {}
    ContactInfo(const Vector2f& p, const Vector2f& n, f32 pen)
        : point(p), normal(n), penetration(pen) {}
};

struct CollisionInfo {
    bool colliding;
    f32 penetration;
    Vector2f normal;
    std::vector<Vector2f> contactPoints;

    CollisionInfo() : colliding(false), penetration(0), normal(0, 0) {}
};

class Narrowphase {
public:
    static CollisionInfo AABBvsAABB(const BoxCollider& a, const Vector2f& posA,
                                     const BoxCollider& b, const Vector2f& posB);
    static CollisionInfo CirclevsCircle(const CircleCollider& a, const Vector2f& posA,
                                         const CircleCollider& b, const Vector2f& posB);
    static CollisionInfo AABBvsCircle(const BoxCollider& box, const Vector2f& boxPos,
                                       const CircleCollider& circle, const Vector2f& circlePos);
    static CollisionInfo SAT(const PolygonCollider& a, const Vector2f& posA, f32 rotA,
                              const PolygonCollider& b, const Vector2f& posB, f32 rotB);
    static CollisionInfo CirclevsPolygon(const CircleCollider& circle, const Vector2f& circlePos,
                                          const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot);
    static CollisionInfo AABBvsPolygon(const BoxCollider& box, const Vector2f& boxPos,
                                        const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot);

private:
    static std::vector<Vector2f> getTransformedVertices(const PolygonCollider& poly,
                                                         const Vector2f& pos, f32 rot);
    static std::vector<Vector2f> getBoxVertices(const BoxCollider& box, const Vector2f& pos);
    static void getAxes(const std::vector<Vector2f>& vertices, std::vector<Vector2f>& axes);
    static void project(const std::vector<Vector2f>& vertices, const Vector2f& axis,
                        f32& min, f32& max);
    static Vector2f getClosestPointOnPolygon(const Vector2f& point,
                                              const std::vector<Vector2f>& vertices);
};

} // namespace nebula
