#pragma once
#include <cmath>
#include "core/Types.h"
#include "math/Vector2.h"
#include "Collider.h"

namespace nebula {

enum class BodyType {
    Static,
    Dynamic,
    Kinematic
};

struct BuoyancyParams {
    float fluidDensity;
    float fluidLinearDrag;
    float fluidAngularDrag;
    Vector2f fluidVelocity;
    float fluidHeight;

    BuoyancyParams()
        : fluidDensity(1000.0f)
        , fluidLinearDrag(5.0f)
        , fluidAngularDrag(1.0f)
        , fluidVelocity(0, 0)
        , fluidHeight(0.0f) {}
};

class RigidBody {
public:
    BodyType type;
    f32 mass;
    f32 inverseMass;
    f32 inertia;
    f32 inverseInertia;
    Vector2f position;
    f32 rotation;
    Vector2f linearVelocity;
    f32 angularVelocity;
    Vector2f force;
    f32 torque;
    f32 staticFriction;
    f32 dynamicFriction;
    f32 restitution;
    f32 linearDamping;
    f32 angularDamping;
    bool fixedRotation;
    Collider* collider;

    bool asleep;
    f32 sleepThreshold;
    f32 activationEnergy;
    bool freezePositionX;
    bool freezePositionY;
    bool freezeRotation;
    f32 gravityScale;
    BuoyancyParams buoyancy;

    RigidBody()
        : type(BodyType::Dynamic)
        , mass(1.0f), inverseMass(1.0f)
        , inertia(1.0f), inverseInertia(1.0f)
        , position(0.0f, 0.0f), rotation(0.0f)
        , linearVelocity(0.0f, 0.0f), angularVelocity(0.0f)
        , force(0.0f, 0.0f), torque(0.0f)
        , staticFriction(0.6f), dynamicFriction(0.4f)
        , restitution(0.5f)
        , linearDamping(0.0f), angularDamping(0.0f)
        , fixedRotation(false)
        , collider(nullptr)
        , asleep(false)
        , sleepThreshold(0.01f)
        , activationEnergy(0.05f)
        , freezePositionX(false)
        , freezePositionY(false)
        , freezeRotation(false)
        , gravityScale(1.0f) {}

    void applyForce(const Vector2f& f) { force += f; }
    void applyImpulse(const Vector2f& impulse, const Vector2f& contactPoint);
    void applyTorque(f32 t) { torque += t; }
    void applyForceAtPoint(const Vector2f& f, const Vector2f& point);
    void applyBuoyancyForce(const BuoyancyParams& params);

    void setMass(f32 m) {
        mass = m;
        inverseMass = (m > 0.0f) ? 1.0f / m : 0.0f;
        if (collider) updateInertia();
    }

    void setMassFromDensity(f32 density);

    Vector2f getVelocityAtPoint(const Vector2f& point) const {
        Vector2f r = point - position;
        return linearVelocity + Vector2f(-angularVelocity * r.y, angularVelocity * r.x);
    }

    void clearForces() { force = Vector2f(0, 0); torque = 0.0f; }

    void setLinearDamping(f32 d) { linearDamping = d; }
    void setAngularDamping(f32 d) { angularDamping = d; }

    bool isStatic() const { return type == BodyType::Static; }
    bool isKinematic() const { return type == BodyType::Kinematic; }
    bool isDynamic() const { return type == BodyType::Dynamic; }

    void setFixedRotation(bool fixed) { fixedRotation = fixed; }
    f32 getKineticEnergy() const {
        f32 linearKE = 0.5f * mass * linearVelocity.lengthSquared();
        f32 angularKE = 0.5f * inertia * angularVelocity * angularVelocity;
        return linearKE + angularKE;
    }

    bool canSleep() const;
    void trySleep();
    void wakeUp();

    void setFreezePositionX(bool freeze) { freezePositionX = freeze; }
    void setFreezePositionY(bool freeze) { freezePositionY = freeze; }
    void setFreezeRotation(bool freeze) { freezeRotation = freeze; }
    void setGravityScale(f32 scale) { gravityScale = scale; }

    void updateInertia();

    void integrateForces(f32 dt);
    void integrateVelocities(f32 dt);
};

} // namespace nebula
