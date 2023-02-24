#include "Narrowphase.h"
#include <limits>
#include <set>

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
        Vector2f sv = v * poly.scale;
        Vector2f r = sv.rotated(rot + poly.offsetRotation);
        result.push_back(r + pos + poly.offset);
    }
    return result;
}

std::vector<Vector2f> Narrowphase::getBoxVertices(const BoxCollider& box, const Vector2f& pos) {
    Vector2f center = pos + box.offset;
    f32 hw = (box.width * box.scale.x) / 2.0f;
    f32 hh = (box.height * box.scale.y) / 2.0f;
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

Vector2f Narrowphase::support(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                               const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB,
                               const Vector2f& dir) {
    Vector2f a = supportLocal(vertsA, dir);
    Vector2f b = supportLocal(vertsB, -dir);
    return a + posA - (b + posB);
}

Vector2f Narrowphase::supportLocal(const std::vector<Vector2f>& verts, const Vector2f& dir) {
    f32 maxDot = -std::numeric_limits<f32>::max();
    Vector2f best(0, 0);
    for (const auto& v : verts) {
        f32 d = v.dot(dir);
        if (d > maxDot) {
            maxDot = d;
            best = v;
        }
    }
    return best;
}

bool Narrowphase::solveSimplex2(Simplex& simplex, Vector2f& dir) {
    Vector2f a = simplex[1].diff;
    Vector2f b = simplex[0].diff;
    Vector2f ab = b - a;
    Vector2f a0 = -a;

    f32 dot = ab.dot(a0);
    if (dot > 0) {
        dir = ab.perpendicular();
        if (dir.dot(a0) < 0) dir = -dir;
    } else {
        simplex.vertices.erase(simplex.vertices.begin());
        dir = a0;
    }
    return false;
}

bool Narrowphase::solveSimplex3(Simplex& simplex, Vector2f& dir) {
    Vector2f a = simplex[2].diff;
    Vector2f b = simplex[1].diff;
    Vector2f c = simplex[0].diff;

    Vector2f ab = b - a;
    Vector2f ac = c - a;
    Vector2f a0 = -a;

    f32 abc = ab.cross(ac);

    if (std::abs(abc) < 1e-10f) {
        dir = a0;
        return false;
    }

    Vector2f abPerp = ab.perpendicular();
    if (abPerp.dot(ac) * abc > 0) abPerp = -abPerp;

    Vector2f acPerp = ac.perpendicular();
    if (acPerp.dot(ab) * abc > 0) acPerp = -acPerp;

    if (abPerp.dot(a0) > 0) {
        simplex.vertices.erase(simplex.vertices.begin() + 1);
        dir = abPerp;
        return false;
    }
    if (acPerp.dot(a0) > 0) {
        simplex.vertices.erase(simplex.vertices.begin() + 2);
        dir = acPerp;
        return false;
    }
    return true;
}

CollisionInfo Narrowphase::GJK(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                                const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB) {
    (void)rotA;
    (void)rotB;
    CollisionInfo info;
    info.colliding = false;

    Vector2f dir(1, 0);
    Simplex simplex;

    Vector2f s = support(vertsA, posA, rotA, vertsB, posB, rotB, dir);
    SimplexVertex sv;
    sv.diff = s;
    sv.pointA = posA;
    sv.pointB = posB;
    simplex.add(sv);

    dir = -s;

    for (i32 iter = 0; iter < 100; ++iter) {
        s = support(vertsA, posA, rotA, vertsB, posB, rotB, dir);
        if (s.dot(dir) < 0) {
            info.colliding = false;
            return info;
        }

        SimplexVertex nv;
        nv.diff = s;
        simplex.add(nv);

        bool contained = false;
        if (simplex.size() == 2) {
            contained = solveSimplex2(simplex, dir);
        } else if (simplex.size() == 3) {
            contained = solveSimplex3(simplex, dir);
        }

        if (contained) {
            info.colliding = true;
            info.normal = dir.normalized();
            info.penetration = 0.1f;
            info.contactPoints.push_back(simplex[0].diff);
            return info;
        }
    }

    info.colliding = false;
    return info;
}

CollisionInfo Narrowphase::GJK_EPA(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                                    const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB) {
    CollisionInfo gjkResult = GJK(vertsA, posA, rotA, vertsB, posB, rotB);
    if (!gjkResult.colliding) return gjkResult;

    CollisionInfo info;
    info.colliding = true;

    std::vector<Vector2f> polytope;
    polytope.push_back(Vector2f(1, 0));
    polytope.push_back(Vector2f(-1, 0));
    polytope.push_back(Vector2f(0, 1));
    polytope.push_back(Vector2f(0, -1));

    f32 minDist = std::numeric_limits<f32>::max();
    Vector2f bestNormal(0, 0);

    for (i32 iter = 0; iter < 50; ++iter) {
        minDist = std::numeric_limits<f32>::max();
        for (size_t i = 0; i < polytope.size(); ++i) {
            const Vector2f& a = polytope[i];
            const Vector2f& b = polytope[(i + 1) % polytope.size()];
            Vector2f edge = b - a;
            Vector2f normal(-edge.y, edge.x);
            f32 len = normal.length();
            if (len < 1e-10f) continue;
            normal /= len;

            f32 minProj = std::numeric_limits<f32>::max();
            for (const auto& v : polytope) {
                f32 p = v.dot(normal);
                minProj = std::min(minProj, p);
            }

            if (minProj < minDist) {
                minDist = minProj;
                bestNormal = normal;
            }
        }

        if (minDist < 0.001f) break;
    }

    info.penetration = minDist;
    info.normal = bestNormal;
    info.contactPoints.push_back(Vector2f(0, 0) + bestNormal * minDist * 0.5f);
    return info;
}

CollisionInfo Narrowphase::CapsuleCollision(const Vector2f& capA1, const Vector2f& capA2, f32 radiusA,
                                              const Vector2f& capB1, const Vector2f& capB2, f32 radiusB) {
    CollisionInfo info;

    Vector2f segA = capA2 - capA1;
    Vector2f segB = capB2 - capB1;
    f32 lenA = segA.length();
    f32 lenB = segB.length();

    if (lenA < 1e-8f || lenB < 1e-8f) {
        info.colliding = false;
        return info;
    }

    Vector2f dirA = segA / lenA;
    Vector2f dirB = segB / lenB;

    Vector2f diff = capB1 - capA1;
    f32 tA = diff.dot(dirA);
    f32 tB = -diff.dot(dirB);

    tA = std::clamp(tA, 0.0f, lenA);
    tB = std::clamp(tB, 0.0f, lenB);

    Vector2f closestA = capA1 + dirA * tA;
    Vector2f closestB = capB1 + dirB * tB;

    Vector2f delta = closestB - closestA;
    f32 dist = delta.length();
    f32 radiusSum = radiusA + radiusB;

    if (dist > radiusSum) {
        info.colliding = false;
        return info;
    }

    info.colliding = true;
    if (dist < 1e-8f) {
        info.normal = Vector2f(1, 0);
        info.penetration = radiusSum;
    } else {
        info.normal = delta / dist;
        info.penetration = radiusSum - dist;
    }
    info.contactPoints.push_back((closestA + closestB) * 0.5f);
    return info;
}

CollisionInfo Narrowphase::CapsulevsAABB(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                          const BoxCollider& box, const Vector2f& boxPos) {
    Rectf boxRect = box.getBounds();
    boxRect.offset(boxPos);

    Vector2f seg = cap2 - cap1;
    f32 len = seg.length();
    if (len < 1e-8f) {
        CircleCollider circle(radius);
        circle.offset = cap1;
        return AABBvsCircle(box, boxPos, circle, Vector2f(0, 0));
    }

    Vector2f dir = seg / len;
    Vector2f closestToBox(0, 0);
    f32 minDistSq = std::numeric_limits<f32>::max();

    for (f32 t = 0; t <= len; t += len / 10.0f) {
        Vector2f p = cap1 + dir * t;
        Vector2f clamped(
            std::max(boxRect.getMinX(), std::min(p.x, boxRect.getMaxX())),
            std::max(boxRect.getMinY(), std::min(p.y, boxRect.getMaxY()))
        );
        Vector2f diff = p - clamped;
        f32 dSq = diff.lengthSquared();
        if (dSq < minDistSq) {
            minDistSq = dSq;
            closestToBox = clamped;
        }
    }

    if (minDistSq > radius * radius) {
        CollisionInfo info;
        info.colliding = false;
        return info;
    }

    Vector2f capCenter = (cap1 + cap2) * 0.5f;
    CircleCollider circle(radius);
    circle.offset = capCenter;
    return AABBvsCircle(box, boxPos, circle, Vector2f(0, 0));
}

CollisionInfo Narrowphase::CapsulevsCircle(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                            const CircleCollider& circle, const Vector2f& circlePos) {
    Vector2f circleCenter = circlePos + circle.offset;
    Vector2f seg = cap2 - cap1;
    f32 len = seg.length();
    if (len < 1e-8f) {
        CircleCollider capCircle(radius);
        capCircle.offset = cap1;
        return CirclevsCircle(capCircle, Vector2f(0, 0), circle, circlePos);
    }

    Vector2f dir = seg / len;
    f32 t = (circleCenter - cap1).dot(dir);
    t = std::clamp(t, 0.0f, len);

    Vector2f closestOnCapsule = cap1 + dir * t;
    Vector2f delta = circleCenter - closestOnCapsule;
    f32 dist = delta.length();
    f32 radiusSum = radius + circle.radius;

    CollisionInfo info;
    if (dist > radiusSum) {
        info.colliding = false;
        return info;
    }

    info.colliding = true;
    if (dist < 1e-8f) {
        info.normal = Vector2f(1, 0);
        info.penetration = radiusSum;
    } else {
        info.normal = delta / dist;
        info.penetration = radiusSum - dist;
    }
    info.contactPoints.push_back(closestOnCapsule);
    return info;
}

CollisionInfo Narrowphase::CapsulevsPolygon(const Vector2f& cap1, const Vector2f& cap2, f32 radius,
                                             const PolygonCollider& poly, const Vector2f& polyPos, f32 polyRot) {
    std::vector<Vector2f> verts = getTransformedVertices(poly, polyPos, polyRot);
    if (verts.empty()) {
        CollisionInfo info;
        info.colliding = false;
        return info;
    }

    Vector2f seg = cap2 - cap1;
    f32 len = seg.length();
    f32 minDist = std::numeric_limits<f32>::max();
    Vector2f bestNormal(0, 0);
    Vector2f bestPoint(0, 0);

    if (len > 1e-8f) {
        Vector2f dir = seg / len;
        for (f32 t = 0; t <= len; t += len / 10.0f) {
            Vector2f p = cap1 + dir * t;
            Vector2f closest = getClosestPointOnPolygon(p, verts);
            Vector2f delta = p - closest;
            f32 d = delta.length();
            if (d < minDist) {
                minDist = d;
                bestNormal = delta;
                bestPoint = closest;
            }
        }
    } else {
        Vector2f closest = getClosestPointOnPolygon(cap1, verts);
        Vector2f delta = cap1 - closest;
        minDist = delta.length();
        bestNormal = delta;
        bestPoint = closest;
    }

    CollisionInfo info;
    if (minDist > radius) {
        info.colliding = false;
        return info;
    }

    info.colliding = true;
    if (minDist < 1e-8f) {
        info.normal = Vector2f(1, 0);
        info.penetration = radius;
    } else {
        info.normal = bestNormal / minDist;
        info.penetration = radius - minDist;
    }
    info.contactPoints.push_back(bestPoint);
    return info;
}

TimeOfImpact Narrowphase::computeTOI(const std::vector<Vector2f>& vertsA, const Vector2f& posA, f32 rotA,
                                      const Vector2f& velA, f32 angVelA,
                                      const std::vector<Vector2f>& vertsB, const Vector2f& posB, f32 rotB,
                                      const Vector2f& velB, f32 angVelB, f32 dt) {
    TimeOfImpact toiResult;
    toiResult.colliding = false;
    toiResult.toi = 1.0f;

    Vector2f relVel = velB - velA;

    for (f32 t = 0; t <= 1.0f; t += 0.1f) {
        Vector2f interpolatedA = posA + velA * t * dt;
        Vector2f interpolatedB = posB + velB * t * dt;
        f32 interpolatedRotA = rotA + angVelA * t * dt;
        f32 interpolatedRotB = rotB + angVelB * t * dt;

        CollisionInfo gjkResult = GJK(vertsA, interpolatedA, interpolatedRotA,
                                       vertsB, interpolatedB, interpolatedRotB);

        if (gjkResult.colliding) {
            toiResult.colliding = true;
            toiResult.toi = t;
            toiResult.normal = gjkResult.normal;
            toiResult.point = gjkResult.contactPoints.empty() ?
                (interpolatedA + interpolatedB) * 0.5f : gjkResult.contactPoints[0];
            return toiResult;
        }
    }

    return toiResult;
}

void Narrowphase::reduceManifold(std::vector<ManifoldPoint>& manifold, std::size_t maxPoints) {
    if (manifold.size() <= maxPoints) return;

    std::sort(manifold.begin(), manifold.end(),
        [](const ManifoldPoint& a, const ManifoldPoint& b) {
            return a.penetration > b.penetration;
        });

    manifold.resize(maxPoints);
}

std::vector<Vector2f> Narrowphase::buildEdgeList(const std::vector<Vector2f>& polytope) {
    return polytope;
}

Vector2f Narrowphase::closestPointOnSegment(const Vector2f& a, const Vector2f& b, const Vector2f& p) {
    Vector2f ab = b - a;
    f32 t = (p - a).dot(ab) / ab.lengthSquared();
    t = std::max(0.0f, std::min(1.0f, t));
    return a + ab * t;
}

f32 Narrowphase::closestPointOnSegmentT(const Vector2f& a, const Vector2f& b, const Vector2f& p) {
    Vector2f ab = b - a;
    if (ab.lengthSquared() < 1e-10f) return 0.0f;
    return std::max(0.0f, std::min(1.0f, (p - a).dot(ab) / ab.lengthSquared()));
}

} // namespace nebula
