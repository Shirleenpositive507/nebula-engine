#include "Raycast.h"
#include <limits>
#include <algorithm>
#include <cmath>

namespace nebula {

bool Ray::intersectAABB(const Vector2f& min, const Vector2f& max, f32& t) const {
    f32 tMin = 0.0f, tMax = std::numeric_limits<f32>::max();

    if (std::abs(direction.x) < 1e-8f) {
        if (origin.x < min.x || origin.x > max.x) return false;
    } else {
        f32 invDir = 1.0f / direction.x;
        f32 t1 = (min.x - origin.x) * invDir;
        f32 t2 = (max.x - origin.x) * invDir;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
    }

    if (std::abs(direction.y) < 1e-8f) {
        if (origin.y < min.y || origin.y > max.y) return false;
    } else {
        f32 invDir = 1.0f / direction.y;
        f32 t1 = (min.y - origin.y) * invDir;
        f32 t2 = (max.y - origin.y) * invDir;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
    }

    t = tMin;
    return tMax >= tMin && tMax >= 0;
}

bool Ray::intersectCircle(const Vector2f& center, f32 radius, f32& t) const {
    Vector2f oc = origin - center;
    f32 a = direction.dot(direction);
    f32 b = 2.0f * oc.dot(direction);
    f32 c = oc.dot(oc) - radius * radius;
    f32 disc = b * b - 4 * a * c;

    if (disc < 0) return false;

    f32 sqrtDisc = std::sqrt(disc);
    f32 t0 = (-b - sqrtDisc) / (2.0f * a);
    f32 t1 = (-b + sqrtDisc) / (2.0f * a);

    t = (t0 >= 0) ? t0 : t1;
    return t >= 0;
}

bool Ray::intersectPolygon(const std::vector<Vector2f>& vertices, f32& t, Vector2f& normal) const {
    t = std::numeric_limits<f32>::max();
    bool hit = false;

    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector2f& a = vertices[i];
        const Vector2f& b = vertices[(i + 1) % vertices.size()];
        Vector2f edge = b - a;
        Vector2f edgeNormal(-edge.y, edge.x);
        edgeNormal.normalize();

        f32 denom = direction.dot(edgeNormal);
        if (std::abs(denom) < 1e-8f) continue;

        f32 tEdge = (a - origin).dot(edgeNormal) / denom;
        if (tEdge < 0 || tEdge > t) continue;

        Vector2f point = origin + direction * tEdge;
        Vector2f ap = point - a;
        f32 tAlong = ap.dot(edge) / edge.lengthSquared();

        if (tAlong >= 0 && tAlong <= 1) {
            t = tEdge;
            normal = edgeNormal;
            hit = true;
        }
    }
    return hit;
}

std::vector<RayHit> Ray::cast(const std::vector<RigidBody*>& bodies, f32 maxDist) {
    auto hits = castAll(bodies, maxDist);
    if (hits.empty()) return {};

    std::vector<RayHit> result;
    result.push_back(hits[0]);
    return result;
}

std::vector<RayHit> Ray::castAll(const std::vector<RigidBody*>& bodies, f32 maxDist) {
    std::vector<RayHit> hits;

    for (auto* body : bodies) {
        if (!body->collider) continue;

        RayHit hit;
        bool intersected = false;

        switch (body->collider->getType()) {
            case ColliderType::Box: {
                auto* box = static_cast<BoxCollider*>(body->collider);
                Rectf bounds = box->getBounds();
                bounds.offset(body->position);
                f32 t;
                if (intersectAABB(bounds.getMin(), bounds.getMax(), t) && t <= maxDist) {
                    intersected = true;
                    hit.point = getPoint(t);
                    hit.distance = t;
                    if (std::abs(hit.point.x - bounds.getMinX()) < 1e-6f) hit.normal = Vector2f(-1, 0);
                    else if (std::abs(hit.point.x - bounds.getMaxX()) < 1e-6f) hit.normal = Vector2f(1, 0);
                    else if (std::abs(hit.point.y - bounds.getMinY()) < 1e-6f) hit.normal = Vector2f(0, -1);
                    else if (std::abs(hit.point.y - bounds.getMaxY()) < 1e-6f) hit.normal = Vector2f(0, 1);
                }
                break;
            }
            case ColliderType::Circle: {
                auto* circle = static_cast<CircleCollider*>(body->collider);
                f32 t;
                Vector2f center = body->position + circle->offset;
                if (intersectCircle(center, circle->radius, t) && t <= maxDist) {
                    intersected = true;
                    hit.point = getPoint(t);
                    hit.normal = (hit.point - center).normalized();
                    hit.distance = t;
                }
                break;
            }
            case ColliderType::Polygon: {
                auto* poly = static_cast<PolygonCollider*>(body->collider);
                auto verts = Narrowphase::getTransformedVertices(*poly, body->position, body->rotation);
                f32 t;
                Vector2f normal;
                if (intersectPolygon(verts, t, normal) && t <= maxDist) {
                    intersected = true;
                    hit.point = getPoint(t);
                    hit.normal = normal;
                    hit.distance = t;
                }
                break;
            }
        }

        if (intersected) {
            hit.collider = body->collider;
            hit.rigidBody = body;
            hits.push_back(hit);
        }
    }

    std::sort(hits.begin(), hits.end(), [](const RayHit& a, const RayHit& b) {
        return a.distance < b.distance;
    });
    return hits;
}

std::vector<RayHit> Ray::castWithMask(const std::vector<RigidBody*>& bodies,
                                       f32 maxDist, u32 layerMask) {
    (void)layerMask;
    return castAll(bodies, maxDist);
}

} // namespace nebula
