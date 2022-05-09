#include "PhysicsWorld.h"
#include <algorithm>
#include <cmath>

namespace nebula {

PhysicsWorld::PhysicsWorld()
    : gravity(0.0f, -9.81f)
    , debugDraw(false)
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
}

void PhysicsWorld::removeBody(RigidBody* body) {
    auto it = std::find(bodies.begin(), bodies.end(), body);
    if (it != bodies.end()) bodies.erase(it);
    broadphase.remove(body);
    if (body->collider) {
        auto cit = std::find(colliders.begin(), colliders.end(), body->collider);
        if (cit != colliders.end()) colliders.erase(cit);
    }
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
    accumulator = 0.0f;
}

void PhysicsWorld::step(f32 dt) {
    accumulator += dt;
    while (accumulator >= fixedTimestep) {
        integrateForces(fixedTimestep);
        broadphaseDetection();
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
        f32 correctionMag = std::max(penetration - allowedPenetration.x, 0.0f) * baumgarte;

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
    (void)collisions;
}

} // namespace nebula
