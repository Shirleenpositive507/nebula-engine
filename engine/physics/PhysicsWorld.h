#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <set>
#include "core/Types.h"
#include "math/Vector2.h"
#include "Broadphase.h"
#include "Narrowphase.h"
#include "CollisionDetector.h"
#include "RigidBody.h"
#include "Collider.h"
#include "Joint.h"

namespace nebula {

struct CollisionEvent {
    RigidBody* bodyA;
    RigidBody* bodyB;
    CollisionInfo info;

    CollisionEvent() : bodyA(nullptr), bodyB(nullptr) {}
};

class PhysicsWorld {
public:
    std::vector<RigidBody*> bodies;
    std::vector<Collider*> colliders;
    std::vector<Joint*> joints;
    Vector2f gravity;
    Broadphase broadphase;
    Narrowphase narrowphase;
    CollisionDetector detector;

    std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> onCollisionEnter;
    std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> onCollisionStay;
    std::function<void(RigidBody*, RigidBody*)> onCollisionExit;

    bool debugDraw;
    f32 fixedTimestep;
    f32 accumulator;
    i32 velocityIterations;
    i32 positionIterations;
    Vector2f allowedPenetration;
    f32 baumgarteCoefficient;

    std::set<ColliderPair, std::function<bool(const ColliderPair&, const ColliderPair&)>> previousCollisions;

    PhysicsWorld();
    explicit PhysicsWorld(const Vector2f& gravity);
    ~PhysicsWorld();

    void addBody(RigidBody* body);
    void removeBody(RigidBody* body);
    void addCollider(Collider* collider);
    void removeCollider(Collider* collider);
    void addJoint(Joint* joint);
    void removeJoint(Joint* joint);

    void step(f32 dt);
    void clear();

    void setGravity(const Vector2f& g) { gravity = g; }
    Vector2f getGravity() const { return gravity; }

private:
    void integrateForces(f32 dt);
    void broadphaseDetection();
    void narrowphaseDetection(const std::vector<ColliderPair>& pairs);
    void generateContacts(const std::vector<CollisionInfo>& collisions);
    void resolveContacts(const std::vector<CollisionInfo>& collisions, f32 dt);
    void resolveCollision(RigidBody* a, RigidBody* b, const CollisionInfo& info, f32 dt);
    void integrateVelocities(f32 dt);
    void applyDamping(f32 dt);
    void updateJoints(f32 dt);
    void detectEvents(const std::vector<CollisionInfo>& collisions);
};

} // namespace nebula
