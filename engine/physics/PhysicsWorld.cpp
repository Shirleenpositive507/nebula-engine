#include "PhysicsWorld.h"
#include "PhysicsDebug.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace nebula {

PhysicsWorld::PhysicsWorld()
    : gravity(0.0f, -9.81f)
    , debugDraw(false)
    , debugDrawMode(DebugDrawMode::None)
    , fixedTimestep(1.0f / 60.0f)
    , accumulator(0.0f)
    , velocityIterations(8)
    , positionIterations(3)
    , allowedPenetration(0.01f, 0.01f)
    , baumgarteCoefficient(0.2f)
    , previousCollisions([](const ColliderPair& a, const ColliderPair& b) {
          return (a.bodyA < b.bodyA) || (a.bodyA == b.bodyA && a.bodyB < b.bodyB);
      }) {}

PhysicsWorld::PhysicsWorld(const Vector2f& g)
    : gravity(g)
    , debugDraw(false)
    , debugDrawMode(DebugDrawMode::None)
    , debugRenderer(nullptr)
    , fixedTimestep(1.0f / 60.0f)
    , accumulator(0.0f)
    , velocityIterations(8)
    , positionIterations(3)
    , allowedPenetration(0.01f, 0.01f)
    , baumgarteCoefficient(0.2f)
    , previousCollisions([](const ColliderPair& a, const ColliderPair& b) {
          return (a.bodyA < b.bodyA) || (a.bodyA == b.bodyA && a.bodyB < b.bodyB);
      }) {}

PhysicsWorld::~PhysicsWorld() {
    clear();
}

void PhysicsWorld::addBody(RigidBody* body) {
    bodies.push_back(body);
    if (body->collider) {
        colliders.push_back(body->collider);
    }
    broadphase.insert(body);
    m_ccdEnabled[body] = false;
}

void PhysicsWorld::removeBody(RigidBody* body) {
    auto it = std::find(bodies.begin(), bodies.end(), body);
    if (it != bodies.end()) bodies.erase(it);
    broadphase.remove(body);
    if (body->collider) {
        auto cit = std::find(colliders.begin(), colliders.end(), body->collider);
        if (cit != colliders.end()) colliders.erase(cit);
    }
    m_ccdEnabled.erase(body);
}

void PhysicsWorld::addCollider(Collider* collider) {
    colliders.push_back(collider);
}

void PhysicsWorld::removeCollider(Collider* collider) {
    auto it = std::find(colliders.begin(), colliders.end(), collider);
    if (it != colliders.end()) colliders.erase(it);
}

void PhysicsWorld::addJoint(Joint* joint) {
    joints.push_back(joint);
}

void PhysicsWorld::removeJoint(Joint* joint) {
    auto it = std::find(joints.begin(), joints.end(), joint);
    if (it != joints.end()) joints.erase(it);
}

void PhysicsWorld::clear() {
    bodies.clear();
    colliders.clear();
    joints.clear();
    broadphase.clear();
    previousCollisions.clear();
    m_ccdEnabled.clear();
    m_triggerEvents.clear();
    accumulator = 0.0f;
}

void PhysicsWorld::step(f32 dt) {
    accumulator += dt;
    while (accumulator >= fixedTimestep) {
        integrateForces(fixedTimestep);
        broadphaseDetection();
        narrowphaseDetection(broadphase.getPotentialCollisions());
        performCCDSweeps(fixedTimestep);
        integrateVelocities(fixedTimestep);
        accumulator -= fixedTimestep;
    }
}

void PhysicsWorld::integrateForces(f32 dt) {
    for (auto* body : bodies) {
        if (body->isStatic()) continue;
        body->force += gravity * body->mass;
        body->integrateForces(dt);
    }
}

void PhysicsWorld::broadphaseDetection() {
    for (auto* body : bodies) {
        broadphase.update(body);
    }

    std::vector<ColliderPair> pairs = broadphase.getPotentialCollisions();
    std::vector<CollisionInfo> collisions = detector.detectCollisions(pairs);

    generateContacts(collisions);
    resolveContacts(collisions, fixedTimestep);
    detectEvents(collisions);
    detectTriggerEvents(pairs);

    std::set<ColliderPair, std::function<bool(const ColliderPair&, const ColliderPair&)>>
        currentSet([](const ColliderPair& a, const ColliderPair& b) {
            return (a.bodyA < b.bodyA) || (a.bodyA == b.bodyA && a.bodyB < b.bodyB);
        });

    for (const auto& pair : pairs) {
        currentSet.insert(pair);
    }

    if (onCollisionExit) {
        for (const auto& prev : previousCollisions) {
            if (currentSet.find(prev) == currentSet.end()) {
                onCollisionExit(prev.bodyA, prev.bodyB);
            }
        }
    }

    previousCollisions = currentSet;
}

void PhysicsWorld::narrowphaseDetection(const std::vector<ColliderPair>& pairs) {
    std::vector<CollisionInfo> collisions = detector.detectCollisions(pairs);
    generateContacts(collisions);
}

void PhysicsWorld::generateContacts(const std::vector<CollisionInfo>& collisions) {
    (void)collisions;
}

void PhysicsWorld::resolveContacts(const std::vector<CollisionInfo>& collisions, f32 dt) {
    for (i32 iter = 0; iter < positionIterations; ++iter) {
        for (const auto& info : collisions) {
            resolveCollision(nullptr, nullptr, info, dt);
        }
    }
}

void PhysicsWorld::resolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info, f32 dt) {
    (void)bodyA;
    (void)bodyB;

    for (const auto& contact : info.contactPoints) {
        Vector2f normal = info.normal;
        f32 penetration = info.penetration;

        if (penetration <= 0.0f) continue;

        RigidBody* a = nullptr;
        RigidBody* b = nullptr;
        f32 totalInvMass = 0.0f;

        for (auto* body : bodies) {
            if (!body->collider) continue;
            Rectf bounds = body->collider->getBounds();
            bounds.offset(body->position);
            if (bounds.contains(contact)) {
                if (!a) { a = body; totalInvMass += body->inverseMass; }
                else if (!b) { b = body; totalInvMass += body->inverseMass; }
            }
        }

        if (!a && !b) return;

        if (totalInvMass < 1e-8f) return;

        f32 baumgarte = baumgarteCoefficient / dt;
        f32 slop = allowedPenetration.x;
        f32 correctionMag = std::max(penetration - slop, 0.0f) * baumgarte;

        if (correctionMag > 0.2f) {
            correctionMag = 0.2f;
        }

        Vector2f correction = normal * correctionMag;

        if (a && !a->isStatic()) {
            a->position += correction * (a->inverseMass / totalInvMass);
        }
        if (b && !b->isStatic()) {
            b->position -= correction * (b->inverseMass / totalInvMass);
        }

        if (a && b && a->isDynamic() && b->isDynamic()) {
            Vector2f relVel = b->linearVelocity - a->linearVelocity;
            f32 relVelAlongNormal = relVel.dot(normal);

            if (relVelAlongNormal > 0) continue;

            f32 e = std::min(a->restitution, b->restitution);
            f32 j = -(1.0f + e) * relVelAlongNormal / totalInvMass;

            Vector2f impulse = normal * j;
            a->linearVelocity -= impulse * a->inverseMass;
            b->linearVelocity += impulse * b->inverseMass;

            Vector2f tangent = relVel - normal * relVelAlongNormal;
            if (tangent.lengthSquared() > 1e-8f) {
                tangent.normalize();
                f32 friction = std::sqrt(a->staticFriction * b->staticFriction);
                f32 jt = -relVel.dot(tangent) / totalInvMass;
                if (std::abs(jt) < j * friction) {
                    a->linearVelocity -= tangent * jt * a->inverseMass;
                    b->linearVelocity += tangent * jt * b->inverseMass;
                } else {
                    f32 dynFriction = std::sqrt(a->dynamicFriction * b->dynamicFriction);
                    a->linearVelocity -= tangent * j * dynFriction * a->inverseMass;
                    b->linearVelocity += tangent * j * dynFriction * b->inverseMass;
                }
            }
        }
    }
}

void PhysicsWorld::integrateVelocities(f32 dt) {
    for (auto* body : bodies) {
        body->integrateVelocities(dt);
    }
}

void PhysicsWorld::applyDamping(f32 dt) {
    for (auto* body : bodies) {
        if (body->linearDamping > 0.0f) {
            body->linearVelocity *= std::max(1.0f - body->linearDamping * dt, 0.0f);
        }
        if (body->angularDamping > 0.0f) {
            body->angularVelocity *= std::max(1.0f - body->angularDamping * dt, 0.0f);
        }
    }
}

void PhysicsWorld::updateJoints(f32 dt) {
    for (auto* joint : joints) {
        joint->solve(dt);
    }
}

void PhysicsWorld::detectEvents(const std::vector<CollisionInfo>& collisions) {
    for (const auto& info : collisions) {
        if (info.colliding) {
            if (onCollisionEnter) {
                onCollisionEnter(nullptr, nullptr, info);
            }
        }
    }
}

void PhysicsWorld::detectTriggerEvents(const std::vector<ColliderPair>& pairs) {
    for (const auto& pair : pairs) {
        RigidBody* a = pair.bodyA;
        RigidBody* b = pair.bodyB;
        if (!a->collider || !b->collider) continue;

        bool aIsTrigger = a->collider->isTrigger;
        bool bIsTrigger = b->collider->isTrigger;

        if (!aIsTrigger && !bIsTrigger) continue;

        RigidBody* triggerBody = aIsTrigger ? a : b;
        RigidBody* otherBody = aIsTrigger ? b : a;

        Rectf boundsA = a->collider->getBounds();
        boundsA.offset(a->position);
        Rectf boundsB = b->collider->getBounds();
        boundsB.offset(b->position);

        if (boundsA.intersects(boundsB)) {
            TriggerEvent evt;
            evt.triggerBody = triggerBody;
            evt.otherBody = otherBody;
            evt.entered = true;
            m_triggerEvents.push_back(evt);

            if (onTriggerEnter) {
                onTriggerEnter(triggerBody, otherBody);
            }
        }
    }
}

void PhysicsWorld::performCCDSweeps(f32 dt) {
    for (auto* body : bodies) {
        auto it = m_ccdEnabled.find(body);
        if (it == m_ccdEnabled.end() || !it->second) continue;
        if (!body->isDynamic() || !body->collider) continue;

        f32 speed = body->linearVelocity.length();
        if (speed < 0.1f) continue;

        Vector2f step = body->linearVelocity * dt;
        f32 stepLen = step.length();

        Rectf bodyBounds = body->collider->getBounds();
        bodyBounds.offset(body->position);
        f32 minDim = std::min(bodyBounds.width, bodyBounds.height);

        if (stepLen > minDim * 0.5f) {
            Vector2f ccdOrigin = body->position;
            Vector2f ccdDir = body->linearVelocity.normalized();

            RaycastResult hit = raycast(ccdOrigin, ccdDir, stepLen);
            if (hit.hit && hit.body != body && hit.distance < stepLen) {
                body->position = hit.point - ccdDir * 0.01f;
                body->linearVelocity = body->linearVelocity * 0.1f;
            }
        }
    }
}

void PhysicsWorld::setCCDEnabled(RigidBody* body, bool enabled) {
    m_ccdEnabled[body] = enabled;
}

bool PhysicsWorld::isCCDEnabled(RigidBody* body) const {
    auto it = m_ccdEnabled.find(body);
    if (it != m_ccdEnabled.end()) {
        return it->second;
    }
    return false;
}

void PhysicsWorld::setDebugDrawMode(DebugDrawMode mode) {
    m_debugDrawMode = mode;
    debugDraw = (mode != DebugDrawMode::None);
}

DebugDrawMode PhysicsWorld::getDebugDrawMode() const {
    return m_debugDrawMode;
}

void PhysicsWorld::renderDebug() {
    if (!debugDraw) return;
    if (debugRenderer) {
        debugRenderer->setEnabled(true);
        debugRenderer->drawWorld(*this);
    }
}

std::vector<TriggerEvent> PhysicsWorld::getTriggerEvents() const {
    return m_triggerEvents;
}

void PhysicsWorld::clearTriggerEvents() {
    m_triggerEvents.clear();
}

RaycastResult PhysicsWorld::raycast(const Vector2f& origin, const Vector2f& direction, f32 maxDistance) {
    std::vector<RaycastResult> hits = raycastAll(origin, direction, maxDistance);
    if (hits.empty()) return RaycastResult();
    return hits[0];
}

std::vector<RaycastResult> PhysicsWorld::raycastAll(const Vector2f& origin, const Vector2f& direction, f32 maxDistance) {
    std::vector<RaycastResult> results;
    Vector2f dir = direction.normalized();
    Vector2f end = origin + dir * maxDistance;

    Rectf rayRect(
        std::min(origin.x, end.x), std::min(origin.y, end.y),
        std::abs(end.x - origin.x), std::abs(end.y - origin.y)
    );

    std::vector<ColliderPair> candidates = broadphase.queryRange(rayRect);
    std::set<RigidBody*> hitBodies;
    for (const auto& pair : candidates) {
        hitBodies.insert(pair.bodyA);
        hitBodies.insert(pair.bodyB);
    }

    for (auto* body : hitBodies) {
        if (!body->collider) continue;

        f32 tMin, tMax;
        Rectf bounds = body->collider->getBounds();
        bounds.offset(body->position);

        Vector2f invDir(1.0f / dir.x, 1.0f / dir.y);
        f32 t1 = (bounds.getMinX() - origin.x) * invDir.x;
        f32 t2 = (bounds.getMaxX() - origin.x) * invDir.x;
        f32 t3 = (bounds.getMinY() - origin.y) * invDir.y;
        f32 t4 = (bounds.getMaxY() - origin.y) * invDir.y;
        tMin = std::max(std::min(t1, t2), std::min(t3, t4));
        tMax = std::min(std::max(t1, t2), std::max(t3, t4));

        if (tMax >= tMin && tMin >= 0 && tMin <= maxDistance) {
            RaycastResult result;
            result.hit = true;
            result.point = origin + dir * tMin;
            result.distance = tMin;
            result.body = body;
            result.collider = body->collider;

            result.normal = Vector2f(0, 0);
            if (std::abs(result.point.x - bounds.getMinX()) < 0.001f) result.normal = Vector2f(-1, 0);
            else if (std::abs(result.point.x - bounds.getMaxX()) < 0.001f) result.normal = Vector2f(1, 0);
            else if (std::abs(result.point.y - bounds.getMinY()) < 0.001f) result.normal = Vector2f(0, -1);
            else if (std::abs(result.point.y - bounds.getMaxY()) < 0.001f) result.normal = Vector2f(0, 1);

            results.push_back(result);
        }
    }

    std::sort(results.begin(), results.end(), [](const RaycastResult& a, const RaycastResult& b) {
        return a.distance < b.distance;
    });

    return results;
}

CircleCastResult PhysicsWorld::circleCast(const Vector2f& origin, f32 radius, const Vector2f& direction, f32 maxDistance) {
    CircleCastResult result;
    Vector2f dir = direction.normalized();
    Vector2f end = origin + dir * maxDistance;

    Rectf sweepRect(
        origin.x - radius, origin.y - radius,
        std::abs(dir.x) * maxDistance + radius * 2.0f,
        std::abs(dir.y) * maxDistance + radius * 2.0f
    );
    if (dir.x < 0) sweepRect.left += dir.x * maxDistance;
    if (dir.y < 0) sweepRect.top += dir.y * maxDistance;

    std::vector<ColliderPair> candidates = broadphase.queryRange(sweepRect);
    std::set<RigidBody*> hitBodies;
    for (const auto& pair : candidates) {
        hitBodies.insert(pair.bodyA);
        hitBodies.insert(pair.bodyB);
    }

    f32 closestDist = maxDistance;
    for (auto* body : hitBodies) {
        if (!body->collider) continue;
        if (body->collider->getType() != ColliderType::Circle) continue;
        auto* circle = static_cast<CircleCollider*>(body->collider);
        Vector2f center = body->position + circle->offset;
        f32 totalRadius = radius + circle->radius;

        Vector2f oc = origin - center;
        f32 a = dir.dot(dir);
        f32 b = 2.0f * oc.dot(dir);
        f32 c = oc.dot(oc) - totalRadius * totalRadius;
        f32 disc = b * b - 4 * a * c;

        if (disc < 0) continue;

        f32 sqrtDisc = std::sqrt(disc);
        f32 t = (-b - sqrtDisc) / (2.0f * a);
        if (t < 0 || t > maxDistance) {
            t = (-b + sqrtDisc) / (2.0f * a);
            if (t < 0 || t > maxDistance) continue;
        }

        if (t < closestDist) {
            closestDist = t;
            result.hit = true;
            result.point = origin + dir * t;
            result.normal = (result.point - center).normalized();
            result.distance = t;
            result.body = body;
            result.collider = body->collider;
        }
    }

    return result;
}

} // namespace nebula
