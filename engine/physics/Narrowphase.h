#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
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

struct ManifoldPoint {
    Vector2f localPointA;
    Vector2f localPointB;
    Vector2f worldPoint;
    Vector2f normal;
    f32 penetration;
    f32 normalImpulse;
    f32 tangentImpulse;
    f32 warmStartFactor;
    bool active;

    ManifoldPoint()
        : penetration(0)
        , normalImpulse(0)
        , tangentImpulse(0)
        , warmStartFactor(0.95f)
        , active(true) {}
};

struct TimeOfImpact {
    bool colliding;
    f32 toi;
    Vector2f point;
    Vector2f normal;

    TimeOfImpact() : colliding(false), toi(1.0f) {}
};

struct SimplexVertex {
    Vector2f pointA;
    Vector2f pointB;
    Vector2f diff;
    Vector2f barycentric;

    SimplexVertex() : pointA(0, 0), pointB(0, 0), diff(0, 0), barycentric(0, 0) {}
};

struct Simplex {
    std::vector<SimplexVertex> vertices;

    void clear() { vertices.clear(); }
    void add(const SimplexVertex& v) { vertices.push_back(v); }
    std::size_t size() const { return vertices.size(); }
    SimplexVertex& operator[](std::size_t i) { return vertices[i]; }
    const SimplexVertex& operator[](std::size_t i) const { return vertices[i]; }
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

    static CollisionInfo GJK(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                              const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB);
    static CollisionInfo GJK_EPA(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                                  const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB);

    static CollisionInfo CapsuleCollision(const Vector2f& capA1, const Vector2f& capA2, f32 radiusA,
                                            const Vector2f& capB1, const Vector2f& capB2, f32 radiusB);
    static CollisionInfo CapsulevsAABB(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                        const BoxCollider& box, const Vector2f& boxPos);
    static CollisionInfo CapsulevsCircle(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                          const CircleCollider& circle, const Vector2f& circlePos);
    static CollisionInfo CapsulevsPolygon(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                           const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot);

    static TimeOfImpact computeTOI(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                                    const Vector2f& velA, f32 angVelA,
                                    const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB,
                                    const Vector2f& velB, f32 angVelB, f32 dt);

    static void reduceManifold(std::vector<ManifoldPoint>& manifold, std::size_t maxPoints = 4);

private:
    static std::vector<Vector2f> getTransformedVertices(const PolygonCollider& poly,
                                                         const Vector2f& pos, f32 rot);
    static std::vector<Vector2f> getBoxVertices(const BoxCollider& box, const Vector2f& pos);
    static void getAxes(const std::vector<Vector2f>& vertices, std::vector<Vector2f>& axes);
    static void project(const std::vector<Vector2f>& vertices, const Vector2f& axis,
                        f32& min, f32& max);
    static Vector2f getClosestPointOnPolygon(const Vector2f& point,
                                              const std::vector<Vector2f>& vertices);

    static Vector2f support(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                             const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB,
                             const Vector2f& dir);
    static Vector2f supportLocal(const std::vector<Vector2f>& verts, const Vector2f& dir);
    static bool solveSimplex2(Simplex& simplex, Vector2f& dir);
    static bool solveSimplex3(Simplex& simplex, Vector2f& dir);

    static std::vector<Vector2f> buildEdgeList(const std::vector<Vector2f>& polytope);
    static Vector2f closestPointOnSegment(const Vector2f& a, const Vector2f& b, const Vector2f& p);
    static f32 closestPointOnSegmentT(const Vector2f& a, const Vector2f& b, const Vector2f& p);
};

} // namespace nebula
