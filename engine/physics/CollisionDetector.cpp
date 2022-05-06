#include "CollisionDetector.h"
#include <limits>
#include <algorithm>

namespace nebula {

std::vector<CollisionInfo> CollisionDetector::detectCollisions(const std::vector<ColliderPair>& pairs) {
    std::vector<CollisionInfo> results;
    for (const auto& pair : pairs) {
        RigidBody* a = pair.bodyA;
        RigidBody* b = pair.bodyB;
        if (!a->collider || !b->collider) continue;

        CollisionInfo info;
        ColliderType typeA = a->collider->getType();
        ColliderType typeB = b->collider->getType();

        if (typeA == ColliderType::Box && typeB == ColliderType::Box) {
            info = narrowphase.AABBvsAABB(
                *static_cast<BoxCollider*>(a->collider), a->position,
                *static_cast<BoxCollider*>(b->collider), b->position);
        } else if (typeA == ColliderType::Circle && typeB == ColliderType::Circle) {
            info = narrowphase.CirclevsCircle(
                *static_cast<CircleCollider*>(a->collider), a->position,
                *static_cast<CircleCollider*>(b->collider), b->position);
        } else if ((typeA == ColliderType::Box && typeB == ColliderType::Circle) ||
                   (typeA == ColliderType::Circle && typeB == ColliderType::Box)) {
            if (typeA == ColliderType::Box) {
                info = narrowphase.AABBvsCircle(
                    *static_cast<BoxCollider*>(a->collider), a->position,
                    *static_cast<CircleCollider*>(b->collider), b->position);
            } else {
                info = narrowphase.AABBvsCircle(
                    *static_cast<BoxCollider*>(b->collider), b->position,
                    *static_cast<CircleCollider*>(a->collider), a->position);
                info.normal = -info.normal;
            }
        } else if (typeA == ColliderType::Polygon && typeB == ColliderType::Polygon) {
            info = narrowphase.SAT(
                *static_cast<PolygonCollider*>(a->collider), a->position, a->rotation,
                *static_cast<PolygonCollider*>(b->collider), b->position, b->rotation);
        } else if ((typeA == ColliderType::Circle && typeB == ColliderType::Polygon) ||
                   (typeA == ColliderType::Polygon && typeB == ColliderType::Circle)) {
            if (typeA == ColliderType::Circle) {
                info = narrowphase.CirclevsPolygon(
                    *static_cast<CircleCollider*>(a->collider), a->position,
                    *static_cast<PolygonCollider*>(b->collider), b->position, b->rotation);
            } else {
                info = narrowphase.CirclevsPolygon(
                    *static_cast<CircleCollider*>(b->collider), b->position,
                    *static_cast<PolygonCollider*>(a->collider), a->position, a->rotation);
                info.normal = -info.normal;
            }
        } else if ((typeA == ColliderType::Box && typeB == ColliderType::Polygon) ||
                   (typeA == ColliderType::Polygon && typeB == ColliderType::Box)) {
            if (typeA == ColliderType::Box) {
                info = narrowphase.AABBvsPolygon(
                    *static_cast<BoxCollider*>(a->collider), a->position,
                    *static_cast<PolygonCollider*>(b->collider), b->position, b->rotation);
            } else {
                info = narrowphase.AABBvsPolygon(
                    *static_cast<BoxCollider*>(b->collider), b->position,
                    *static_cast<PolygonCollider*>(a->collider), a->position, a->rotation);
                info.normal = -info.normal;
            }
        }

        if (info.colliding) {
            results.push_back(info);
        }
    }
    return results;
}

bool CollisionDetector::rayAABBIntersect(const Vector2f& origin, const Vector2f& dir,
                                          const Rectf& bounds, f32& tMin, f32& tMax) {
    f32 invDirX = 1.0f / dir.x;
    f32 invDirY = 1.0f / dir.y;

    f32 t1 = (bounds.getMinX() - origin.x) * invDirX;
    f32 t2 = (bounds.getMaxX() - origin.x) * invDirX;
    f32 t3 = (bounds.getMinY() - origin.y) * invDirY;
    f32 t4 = (bounds.getMaxY() - origin.y) * invDirY;

    tMin = std::max(std::min(t1, t2), std::min(t3, t4));
    tMax = std::min(std::max(t1, t2), std::max(t3, t4));

    return tMax >= tMin && tMax >= 0;
}

bool CollisionDetector::rayCircleIntersect(const Vector2f& origin, const Vector2f& dir,
                                            const Vector2f& center, f32 radius, f32& t) {
    Vector2f oc = origin - center;
    f32 a = dir.dot(dir);
    f32 b = 2.0f * oc.dot(dir);
    f32 c = oc.dot(oc) - radius * radius;
    f32 disc = b * b - 4 * a * c;

    if (disc < 0) return false;

    f32 sqrtDisc = std::sqrt(disc);
    f32 t0 = (-b - sqrtDisc) / (2.0f * a);
    f32 t1 = (-b + sqrtDisc) / (2.0f * a);

    t = (t0 >= 0) ? t0 : t1;
    return t >= 0;
}

RaycastHit CollisionDetector::raycast(const Vector2f& origin, const Vector2f& direction,
                                       f32 maxDistance) {
    std::vector<RaycastHit> hits = raycastAll(origin, direction, maxDistance);
    if (hits.empty()) return RaycastHit();

    RaycastHit closest = hits[0];
    for (size_t i = 1; i < hits.size(); ++i) {
        if (hits[i].distance < closest.distance) {
            closest = hits[i];
        }
    }
    return closest;
}

std::vector<RaycastHit> CollisionDetector::raycastAll(const Vector2f& origin,
                                                       const Vector2f& direction,
                                                       f32 maxDistance) {
    std::vector<RaycastHit> hits;
    Vector2f dir = direction.normalized();
    Vector2f end = origin + dir * maxDistance;
    Rectf rayRect(
        std::min(origin.x, end.x), std::min(origin.y, end.y),
        std::abs(end.x - origin.x), std::abs(end.y - origin.y)
    );

    std::vector<ColliderPair> candidates = broadphase.queryRange(rayRect);
    std::set<RigidBody*> bodies;
    for (const auto& pair : candidates) {
        bodies.insert(pair.bodyA);
        bodies.insert(pair.bodyB);
    }

    for (auto* body : bodies) {
        if (!body->collider) continue;
        f32 t;
        bool hit = false;
        Vector2f hitPoint, hitNormal;

        switch (body->collider->getType()) {
            case ColliderType::Box: {
                auto* box = static_cast<BoxCollider*>(body->collider);
                Rectf bounds = box->getBounds();
                bounds.offset(body->position);
                f32 tMin, tMax;
                if (rayAABBIntersect(origin, dir, bounds, tMin, tMax) && tMin <= maxDistance) {
                    t = tMin;
                    hit = true;
                    hitPoint = origin + dir * t;
                    hitNormal = Vector2f(0, 0);
                    if (std::abs(hitPoint.x - bounds.getMinX()) < 1e-6f) hitNormal = Vector2f(-1, 0);
                    else if (std::abs(hitPoint.x - bounds.getMaxX()) < 1e-6f) hitNormal = Vector2f(1, 0);
                    else if (std::abs(hitPoint.y - bounds.getMinY()) < 1e-6f) hitNormal = Vector2f(0, -1);
                    else if (std::abs(hitPoint.y - bounds.getMaxY()) < 1e-6f) hitNormal = Vector2f(0, 1);
                }
                break;
            }
            case ColliderType::Circle: {
                auto* circle = static_cast<CircleCollider*>(body->collider);
                if (rayCircleIntersect(origin, dir, body->position + circle->offset, circle->radius, t) && t <= maxDistance) {
                    hit = true;
                    hitPoint = origin + dir * t;
                    hitNormal = (hitPoint - (body->position + circle->offset)).normalized();
                }
                break;
            }
            case ColliderType::Polygon: {
                auto* poly = static_cast<PolygonCollider*>(body->collider);
                auto verts = Narrowphase::getTransformedVertices(*poly, body->position, body->rotation);
                for (size_t i = 0; i < verts.size(); ++i) {
                    const Vector2f& a = verts[i];
                    const Vector2f& b = verts[(i + 1) % verts.size()];
                    Vector2f edge = b - a;
                    Vector2f normal(-edge.y, edge.x);
                    normal.normalize();
                    f32 denom = dir.dot(normal);
                    if (std::abs(denom) < 1e-8f) continue;
                    f32 tEdge = (a - origin).dot(normal) / denom;
                    if (tEdge < 0 || tEdge > maxDistance) continue;
                    Vector2f p = origin + dir * tEdge;
                    Vector2f ap = p - a;
                    f32 tAlong = ap.dot(edge) / edge.lengthSquared();
                    if (tAlong >= 0 && tAlong <= 1) {
                        hit = true;
                        t = tEdge;
                        hitPoint = p;
                        hitNormal = normal;
                        break;
                    }
                }
                break;
            }
        }

        if (hit) {
            RaycastHit rh;
            rh.point = hitPoint;
            rh.normal = hitNormal;
            rh.distance = t;
            rh.collider = body->collider;
            rh.rigidBody = body;
            hits.push_back(rh);
        }
    }

    std::sort(hits.begin(), hits.end(), [](const RaycastHit& a, const RaycastHit& b) {
        return a.distance < b.distance;
    });
    return hits;
}

RaycastHit CollisionDetector::raycastFiltered(const Vector2f& origin, const Vector2f& direction,
                                                f32 maxDistance, u32 layerMask) {
    (void)layerMask;
    return raycast(origin, direction, maxDistance);
}

RaycastHit CollisionDetector::circleCast(const Vector2f& origin, f32 radius,
                                          const Vector2f& direction, f32 maxDistance) {
    Vector2f dir = direction.normalized();
    Rectf sweepRect(
        origin.x - radius, origin.y - radius,
        std::abs(dir.x) * maxDistance + radius * 2.0f,
        std::abs(dir.y) * maxDistance + radius * 2.0f
    );
    if (dir.x < 0) sweepRect.left += dir.x * maxDistance;
    if (dir.y < 0) sweepRect.top += dir.y * maxDistance;

    std::vector<ColliderPair> candidates = broadphase.queryRange(sweepRect);
    std::set<RigidBody*> bodies;
    for (const auto& pair : candidates) {
        bodies.insert(pair.bodyA);
        bodies.insert(pair.bodyB);
    }

    RaycastHit closest;
    closest.distance = maxDistance;
    bool found = false;

    for (auto* body : bodies) {
        if (!body->collider) continue;
        if (body->collider->getType() != ColliderType::Circle) continue;
        auto* circle = static_cast<CircleCollider*>(body->collider);
        Vector2f center = body->position + circle->offset;
        f32 totalRadius = radius + circle->radius;

        f32 t;
        if (rayCircleIntersect(origin, dir, center, totalRadius, t) && t < closest.distance) {
            closest.point = origin + dir * t;
            closest.normal = (closest.point - center).normalized();
            closest.distance = t;
            closest.collider = body->collider;
            closest.rigidBody = body;
            found = true;
        }
    }

    return found ? closest : RaycastHit();
}

RaycastHit CollisionDetector::rectCast(const Rectf& rect, const Vector2f& direction,
                                        f32 maxDistance) {
    Vector2f origin = rect.getCenter();
    Vector2f dir = direction.normalized();
    return raycast(origin, dir, maxDistance);
}

RaycastHit CollisionDetector::ccdRaycast(RigidBody* body, const Vector2f& origin,
                                          const Vector2f& direction, f32 maxDistance) {
    Vector2f vel = body->linearVelocity;
    f32 speed = vel.length();
    if (speed < 1e-6f) return RaycastHit();

    Vector2f step = vel.normalized() * (speed * maxDistance);
    Rectf sweptBounds;
    if (body->collider) {
        sweptBounds = body->collider->getBounds();
        sweptBounds.offset(origin);
        Rectf endBounds = body->collider->getBounds();
        endBounds.offset(origin + step);
        sweptBounds = sweptBounds.united(endBounds);
    }

    std::vector<ColliderPair> candidates = broadphase.queryRange(sweptBounds);
    RaycastHit closest;
    closest.distance = maxDistance;
    bool found = false;

    for (const auto& pair : candidates) {
        RigidBody* other = (pair.bodyA == body) ? pair.bodyB : pair.bodyA;
        if (!other->collider) continue;

        RaycastHit hit = raycast(origin, vel.normalized(), maxDistance);
        if (hit.rigidBody == body) continue;
        if (hit.rigidBody && hit.distance < closest.distance) {
            closest = hit;
            found = true;
        }
    }

    return found ? closest : RaycastHit();
}

} // namespace nebula
