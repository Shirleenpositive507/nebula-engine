#include "Narrowphase.h"
#include <limits>

namespace nebula {

CollisionInfo Narrowphase::AABBvsAABB(const BoxCollider& a, const Vector2f& posA,
                                       const BoxCollider& b, const Vector2f& posB) {
    CollisionInfo info;
    Rectf rectA = a.getBounds();
    rectA.offset(posA);
    Rectf rectB = b.getBounds();
    rectB.offset(posB);

    if (!rectA.intersects(rectB)) {
        info.colliding = false;
        return info;
    }

    Vector2f centerA = rectA.getCenter();
    Vector2f centerB = rectB.getCenter();
    Vector2f diff = centerB - centerA;
    f32 overlapX = (rectA.width + rectB.width) / 2.0f - std::abs(diff.x);
    f32 overlapY = (rectA.height + rectB.height) / 2.0f - std::abs(diff.y);

    info.colliding = true;
    if (overlapX < overlapY) {
        info.penetration = overlapX;
        info.normal = Vector2f(diff.x > 0 ? 1.0f : -1.0f, 0.0f);
    } else {
        info.penetration = overlapY;
        info.normal = Vector2f(0.0f, diff.y > 0 ? 1.0f : -1.0f);
    }
    info.contactPoints.push_back(centerA + info.normal * info.penetration);
    return info;
}

CollisionInfo Narrowphase::CirclevsCircle(const CircleCollider& a, const Vector2f& posA,
                                           const CircleCollider& b, const Vector2f& posB) {
    CollisionInfo info;
    Vector2f centerA = posA + a.offset;
    Vector2f centerB = posB + b.offset;
    Vector2f diff = centerB - centerA;
    f32 distSq = diff.lengthSquared();
    f32 radiusSum = a.radius + b.radius;

    if (distSq > radiusSum * radiusSum) {
        info.colliding = false;
        return info;
    }

    f32 dist = std::sqrt(distSq);
    info.colliding = true;
    info.penetration = radiusSum - dist;
    if (dist < 1e-8f) {
        info.normal = Vector2f(1.0f, 0.0f);
        info.contactPoints.push_back(centerA);
    } else {
        info.normal = diff / dist;
        info.contactPoints.push_back(centerA + info.normal * (a.radius - info.penetration / 2.0f));
    }
    return info;
}

CollisionInfo Narrowphase::AABBvsCircle(const BoxCollider& box, const Vector2f& boxPos,
                                         const CircleCollider& circle, const Vector2f& circlePos) {
    CollisionInfo info;
    Rectf boxRect = box.getBounds();
    boxRect.offset(boxPos);
    Vector2f circleCenter = circlePos + circle.offset;

    Vector2f closest(
        std::max(boxRect.getMinX(), std::min(circleCenter.x, boxRect.getMaxX())),
        std::max(boxRect.getMinY(), std::min(circleCenter.y, boxRect.getMaxY()))
    );

    Vector2f diff = circleCenter - closest;
    f32 distSq = diff.lengthSquared();

    if (distSq > circle.radius * circle.radius) {
        info.colliding = false;
        return info;
    }

    f32 dist = std::sqrt(distSq);
    info.colliding = true;
    if (dist < 1e-8f) {
        Vector2f centerDiff = circleCenter - boxRect.getCenter();
        if (centerDiff.lengthSquared() < 1e-8f) centerDiff = Vector2f(0.0f, -1.0f);
        info.normal = centerDiff.normalized();
        info.penetration = circle.radius;
    } else {
        info.normal = diff / dist;
        info.penetration = circle.radius - dist;
    }
    info.contactPoints.push_back(closest);
    return info;
}

std::vector<Vector2f> Narrowphase::getTransformedVertices(const PolygonCollider& poly,
                                                           const Vector2f& pos, f32 rot) {
    std::vector<Vector2f> result;
    result.reserve(poly.vertices.size());
    for (const auto& v : poly.vertices) {
        Vector2f r = v.rotated(rot);
        result.push_back(r + pos + poly.offset);
    }
    return result;
}

std::vector<Vector2f> Narrowphase::getBoxVertices(const BoxCollider& box, const Vector2f& pos) {
    Vector2f center = pos + box.offset;
    f32 hw = box.width / 2.0f;
    f32 hh = box.height / 2.0f;
    return {
        center + Vector2f(-hw, -hh),
        center + Vector2f( hw, -hh),
        center + Vector2f( hw,  hh),
        center + Vector2f(-hw,  hh)
    };
}

void Narrowphase::getAxes(const std::vector<Vector2f>& vertices, std::vector<Vector2f>& axes) {
    axes.clear();
    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector2f& a = vertices[i];
        const Vector2f& b = vertices[(i + 1) % vertices.size()];
        Vector2f edge = b - a;
        Vector2f axis(-edge.y, edge.x);
        f32 len = axis.length();
        if (len > 1e-8f) {
            axes.push_back(axis / len);
        }
    }
}

void Narrowphase::project(const std::vector<Vector2f>& vertices, const Vector2f& axis,
                            f32& min, f32& max) {
    min = std::numeric_limits<f32>::max();
    max = std::numeric_limits<f32>::lowest();
    for (const auto& v : vertices) {
        f32 proj = v.dot(axis);
        min = std::min(min, proj);
        max = std::max(max, proj);
    }
}

CollisionInfo Narrowphase::SAT(const PolygonCollider& a, const Vector2f& posA, f32 rotA,
                                const PolygonCollider& b, const Vector2f& posB, f32 rotB) {
    CollisionInfo info;
    std::vector<Vector2f> vertsA = getTransformedVertices(a, posA, rotA);
    std::vector<Vector2f> vertsB = getTransformedVertices(b, posB, rotB);

    if (vertsA.empty() || vertsB.empty()) {
        info.colliding = false;
        return info;
    }

    std::vector<Vector2f> axes;
    getAxes(vertsA, axes);
    getAxes(vertsB, axes);

    f32 minOverlap = std::numeric_limits<f32>::max();
    Vector2f minAxis(0, 0);

    for (const auto& axis : axes) {
        f32 minA, maxA, minB, maxB;
        project(vertsA, axis, minA, maxA);
        project(vertsB, axis, minB, maxB);

        f32 overlap = std::min(maxA - minB, maxB - minA);
        if (overlap <= 0) {
            info.colliding = false;
            return info;
        }

        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis = axis;
        }
    }

    Vector2f centerA(0, 0), centerB(0, 0);
    for (const auto& v : vertsA) centerA += v;
    for (const auto& v : vertsB) centerB += v;
    centerA /= static_cast<f32>(vertsA.size());
    centerB /= static_cast<f32>(vertsB.size());

    Vector2f centerDiff = centerB - centerA;
    if (centerDiff.dot(minAxis) < 0) minAxis = -minAxis;

    info.colliding = true;
    info.penetration = minOverlap;
    info.normal = minAxis;
    info.contactPoints.push_back(centerA + minAxis * (minOverlap / 2.0f));
    return info;
}

Vector2f Narrowphase::getClosestPointOnPolygon(const Vector2f& point,
                                                 const std::vector<Vector2f>& vertices) {
    Vector2f closest = vertices[0];
    f32 minDist = point.distanceSquared(vertices[0]);
    for (size_t i = 1; i < vertices.size(); ++i) {
        f32 d = point.distanceSquared(vertices[i]);
        if (d < minDist) {
            minDist = d;
            closest = vertices[i];
        }
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector2f& a = vertices[i];
        const Vector2f& b = vertices[(i + 1) % vertices.size()];
        Vector2f ab = b - a;
        f32 t = (point - a).dot(ab) / ab.lengthSquared();
        t = std::max(0.0f, std::min(1.0f, t));
        Vector2f proj = a + ab * t;
        f32 d = point.distanceSquared(proj);
        if (d < minDist) {
            minDist = d;
            closest = proj;
        }
    }
    return closest;
}

CollisionInfo Narrowphase::CirclevsPolygon(const CircleCollider& circle, const Vector2f& circlePos,
                                            const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot) {
    CollisionInfo info;
    std::vector<Vector2f> verts = getTransformedVertices(poly, polyPos, polyRot);
    if (verts.empty()) { info.colliding = false; return info; }

    Vector2f circleCenter = circlePos + circle.offset;
    Vector2f closest = getClosestPointOnPolygon(circleCenter, verts);
    Vector2f diff = circleCenter - closest;
    f32 distSq = diff.lengthSquared();

    if (distSq > circle.radius * circle.radius) {
        info.colliding = false;
        return info;
    }

    f32 dist = std::sqrt(distSq);
    info.colliding = true;
    if (dist < 1e-8f) {
        Vector2f polyCenter(0, 0);
        for (const auto& v : verts) polyCenter += v;
        polyCenter /= static_cast<f32>(verts.size());
        Vector2f dir = circleCenter - polyCenter;
        if (dir.lengthSquared() < 1e-8f) dir = Vector2f(0.0f, -1.0f);
        info.normal = dir.normalized();
        info.penetration = circle.radius;
    } else {
        info.normal = diff / dist;
        info.penetration = circle.radius - dist;
    }
    info.contactPoints.push_back(closest);
    return info;
}

CollisionInfo Narrowphase::AABBvsPolygon(const BoxCollider& box, const Vector2f& boxPos,
                                          const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot) {
    std::vector<Vector2f> boxVerts = getBoxVertices(box, boxPos);
    std::vector<Vector2f> polyVerts = getTransformedVertices(poly, polyPos, polyRot);

    PolygonCollider boxPoly;
    for (const auto& v : boxVerts) {
        boxPoly.addVertex(v - boxPos);
    }

    return SAT(boxPoly, Vector2f(0, 0), 0.0f, poly, polyPos, polyRot);
}

} // namespace nebula
