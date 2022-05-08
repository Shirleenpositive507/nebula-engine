#pragma once
#include <vector>
#include "core/Types.h"
#include "math/Vector2.h"
#include "RigidBody.h"
#include "Collider.h"

namespace nebula {

struct RayHit {
    Vector2f point;
    Vector2f normal;
    f32 distance;
    Collider* collider;
    RigidBody* rigidBody;

    RayHit()
        : point(0, 0), normal(0, 0), distance(0)
        , collider(nullptr), rigidBody(nullptr) {}
};

class Ray {
public:
    Vector2f origin;
    Vector2f direction;

    Ray() : origin(0, 0), direction(1, 0) {}
    Ray(const Vector2f& o, const Vector2f& d) : origin(o), direction(d.normalized()) {}

    Vector2f getPoint(f32 t) const { return origin + direction * t; }

    std::vector<RayHit> cast(const std::vector<RigidBody*>& bodies, f32 maxDist);
    std::vector<RayHit> castAll(const std::vector<RigidBody*>& bodies, f32 maxDist);

    std::vector<RayHit> castWithMask(const std::vector<RigidBody*>& bodies,
                                      f32 maxDist, u32 layerMask);

private:
    bool intersectAABB(const Vector2f& min, const Vector2f& max, f32& t) const;
    bool intersectCircle(const Vector2f& center, f32 radius, f32& t) const;
    bool intersectPolygon(const std::vector<Vector2f>& vertices, f32& t, Vector2f& normal) const;
};

} // namespace nebula
