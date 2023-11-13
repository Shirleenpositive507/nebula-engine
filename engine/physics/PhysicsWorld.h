#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <set>
#include <cstdint>
#include "core/Types.h"
#include "math/Vector2.h"
#include "Broadphase.h"
#include "Narrowphase.h"
#include "CollisionDetector.h"
#include "RigidBody.h"
#include "Collider.h"
#include "Joint.h"

class PhysicsDebug;

namespace nebula {

struct CollisionEvent {
    RigidBody* bodyA;
    RigidBody* bodyB;
    CollisionInfo info;

    CollisionEvent() : bodyA(nullptr), bodyB(nullptr) {}
};

struct TriggerEvent {
    RigidBody* triggerBody;
    RigidBody* otherBody;
    bool entered;

    TriggerEvent() : triggerBody(nullptr), otherBody(nullptr), entered(false) {}
};

enum class DebugDrawMode {
    None,
    WireframeShapes,
    ContactPoints,
    Normals,
    CollisionVolumes,
    All
};

struct RaycastResult {
    bool hit;
    Vector2f point;
    Vector2f normal;
    f32 distance;
    RigidBody* body;
    Collider* collider;

    RaycastResult() : hit(false), distance(0), body(nullptr), collider(nullptr) {}
};

struct CircleCastResult {
    bool hit;
    Vector2f point;
    Vector2f normal;
    f32 distance;
    RigidBody* body;
    Collider* collider;

    CircleCastResult() : hit(false), distance(0), body(nullptr), collider(nullptr) {}
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

    std::function<void(RigidBody*, RigidBody*)> onTriggerEnter;
    std::function<void(RigidBody*, RigidBody*)> onTriggerStay;
    std::function<void(RigidBody*, RigidBody*)> onTriggerExit;

    bool debugDraw;
    DebugDrawMode debugDrawMode;
    PhysicsDebug* debugRenderer;
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

    RaycastResult raycast(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    std::vector<RaycastResult> raycastAll(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    CircleCastResult circleCast(const Vector2f& origin, f32 radius, const Vector2f& direction, f32 maxDistance);

    void setCCDEnabled(RigidBody* body, bool enabled);
    bool isCCDEnabled(RigidBody* body) const;

    void setDebugDrawMode(DebugDrawMode mode);
    DebugDrawMode getDebugDrawMode() const;
    void renderDebug();
    void setDebugRenderer(PhysicsDebug* renderer) { debugRenderer = renderer; }
    PhysicsDebug* getDebugRenderer() const { return debugRenderer; }
    void setDebugDrawEnabled(bool enabled) { debugDraw = enabled; }
    bool isDebugDrawEnabled() const { return debugDraw; }

    std::vector<TriggerEvent> getTriggerEvents() const;
    void clearTriggerEvents();

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
    void detectTriggerEvents(const std::vector<ColliderPair>& pairs);
    void performCCDSweeps(f32 dt);

    std::unordered_map<RigidBody*, bool> m_ccdEnabled;
    std::vector<TriggerEvent> m_triggerEvents;
    DebugDrawMode m_debugDrawMode = DebugDrawMode::None;
};

} // namespace nebula
