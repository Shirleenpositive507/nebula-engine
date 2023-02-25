#pragma once
#include <cmath>
#include "core/Types.h"
#include "math/Vector2.h"
#include "RigidBody.h"

namespace nebula {

enum class JointType {
    Distance,
    Spring,
    Hinge,
    Fixed,
    Pulley,
    Gear,
    Wheel,
    Weld
};

struct Joint {
    JointType type;
    RigidBody* bodyA;
    RigidBody* bodyB;

    Joint() : type(JointType::Distance), bodyA(nullptr), bodyB(nullptr) {}
    virtual ~Joint() = default;

    virtual Vector2f getReactionForce() const = 0;
    virtual f32 getReactionTorque() const = 0;
    virtual void solve(f32 dt) = 0;
};

struct DistanceJoint : public Joint {
    f32 length;
    f32 damping;
    f32 stiffness;

    DistanceJoint()
        : length(1.0f), damping(0.0f), stiffness(0.0f) {
        type = JointType::Distance;
    }

    DistanceJoint(RigidBody* a, RigidBody* b, f32 len, f32 damp, f32 stiff)
        : length(len), damping(damp), stiffness(stiff) {
        type = JointType::Distance;
        bodyA = a;
        bodyB = b;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB) return Vector2f(0, 0);
        Vector2f diff = bodyB->position - bodyA->position;
        f32 dist = diff.length();
        if (dist < 1e-8f) return Vector2f(0, 0);
        f32 displacement = dist - length;
        return diff.normalized() * displacement * stiffness;
    }

    f32 getReactionTorque() const override { return 0.0f; }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;
        Vector2f diff = bodyB->position - bodyA->position;
        f32 dist = diff.length();
        if (dist < 1e-8f) return;

        Vector2f dir = diff / dist;
        f32 displacement = dist - length;
        Vector2f force = dir * (-displacement * stiffness);

        Vector2f relVel = bodyB->linearVelocity - bodyA->linearVelocity;
        f32 dampingForce = relVel.dot(dir) * damping;
        force -= dir * dampingForce;

        bodyA->applyForce(-force);
        bodyB->applyForce(force);
    }

    void setLength(f32 len) { length = len; }
};

struct SpringJoint : public Joint {
    f32 restLength;
    f32 stiffness;
    f32 damping;

    SpringJoint()
        : restLength(1.0f), stiffness(10.0f), damping(1.0f) {
        type = JointType::Spring;
    }

    SpringJoint(RigidBody* a, RigidBody* b, f32 rest, f32 stiff, f32 damp)
        : restLength(rest), stiffness(stiff), damping(damp) {
        type = JointType::Spring;
        bodyA = a;
        bodyB = b;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB) return Vector2f(0, 0);
        Vector2f diff = bodyB->position - bodyA->position;
        f32 dist = diff.length();
        if (dist < 1e-8f) return Vector2f(0, 0);
        f32 displacement = dist - restLength;
        return diff.normalized() * displacement * stiffness;
    }

    f32 getReactionTorque() const override { return 0.0f; }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;
        Vector2f diff = bodyB->position - bodyA->position;
        f32 dist = diff.length();
        if (dist < 1e-8f) return;

        Vector2f dir = diff / dist;
        f32 displacement = dist - restLength;
        Vector2f springForce = dir * (-displacement * stiffness);

        Vector2f relVel = bodyB->linearVelocity - bodyA->linearVelocity;
        f32 dampingForce = relVel.dot(dir) * damping;
        springForce -= dir * dampingForce;

        bodyA->applyForce(-springForce);
        bodyB->applyForce(springForce);
    }

    void setStiffness(f32 s) { stiffness = s; }
    void setDamping(f32 d) { damping = d; }
};

struct HingeJoint : public Joint {
    Vector2f anchor;
    f32 minAngle;
    f32 maxAngle;
    f32 stiffness;
    f32 damping;

    HingeJoint()
        : anchor(0, 0), minAngle(-3.14159f), maxAngle(3.14159f)
        , stiffness(10.0f), damping(1.0f) {
        type = JointType::Hinge;
    }

    HingeJoint(RigidBody* a, RigidBody* b, const Vector2f& anc,
               f32 minAng, f32 maxAng)
        : anchor(anc), minAngle(minAng), maxAngle(maxAng)
        , stiffness(10.0f), damping(1.0f) {
        type = JointType::Hinge;
        bodyA = a;
        bodyB = b;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB) return Vector2f(0, 0);
        Vector2f worldAnchorA = bodyA->position + anchor;
        Vector2f worldAnchorB = bodyB->position + anchor;
        return (worldAnchorB - worldAnchorA) * stiffness;
    }

    f32 getReactionTorque() const override {
        if (!bodyA || !bodyB) return 0.0f;
        f32 angleDiff = bodyB->rotation - bodyA->rotation;
        if (angleDiff > maxAngle) return (angleDiff - maxAngle) * stiffness;
        if (angleDiff < minAngle) return (angleDiff - minAngle) * stiffness;
        return 0.0f;
    }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;
        f32 angleDiff = bodyB->rotation - bodyA->rotation;
        f32 correction = 0.0f;

        if (angleDiff > maxAngle) correction = angleDiff - maxAngle;
        else if (angleDiff < minAngle) correction = angleDiff - minAngle;

        if (std::abs(correction) > 1e-6f) {
            f32 torque = -correction * stiffness;
            f32 angVelDiff = bodyB->angularVelocity - bodyA->angularVelocity;
            torque -= angVelDiff * damping;
            bodyA->applyTorque(-torque);
            bodyB->applyTorque(torque);
        }
    }

    void setLimits(f32 min, f32 max) { minAngle = min; maxAngle = max; }
    void setStiffness(f32 s) { stiffness = s; }
    void setDamping(f32 d) { damping = d; }
};

struct FixedJoint : public Joint {
    Vector2f localAnchorA;
    Vector2f localAnchorB;
    f32 referenceAngle;

    FixedJoint()
        : localAnchorA(0, 0), localAnchorB(0, 0), referenceAngle(0) {
        type = JointType::Fixed;
    }

    FixedJoint(RigidBody* a, RigidBody* b,
               const Vector2f& anchorA, const Vector2f& anchorB, f32 refAngle)
        : localAnchorA(anchorA), localAnchorB(anchorB), referenceAngle(refAngle) {
        type = JointType::Fixed;
        bodyA = a;
        bodyB = b;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB) return Vector2f(0, 0);
        Vector2f worldA = bodyA->position + localAnchorA;
        Vector2f worldB = bodyB->position + localAnchorB;
        return (worldB - worldA) * 100.0f;
    }

    f32 getReactionTorque() const override {
        if (!bodyA || !bodyB) return 0.0f;
        return (bodyB->rotation - bodyA->rotation - referenceAngle) * 100.0f;
    }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;

        Vector2f worldA = bodyA->position + localAnchorA;
        Vector2f worldB = bodyB->position + localAnchorB;
        Vector2f posError = worldB - worldA;
        f32 angError = bodyB->rotation - bodyA->rotation - referenceAngle;

        Vector2f correction = -posError * 100.0f;
        f32 angCorrection = -angError * 100.0f;

        bodyA->applyForce(-correction);
        bodyB->applyForce(correction);
        bodyA->applyTorque(-angCorrection);
        bodyB->applyTorque(angCorrection);
    }
};

struct PulleyJoint : public Joint {
    Vector2f groundAnchorA;
    Vector2f groundAnchorB;
    Vector2f localAnchorA;
    Vector2f localAnchorB;
    f32 ratio;
    f32 maxLengthA;
    f32 maxLengthB;

    PulleyJoint()
        : groundAnchorA(0, 0), groundAnchorB(0, 0)
        , localAnchorA(0, 0), localAnchorB(0, 0)
        , ratio(1.0f), maxLengthA(0), maxLengthB(0) {
        type = JointType::Pulley;
    }

    PulleyJoint(RigidBody* a, RigidBody* b,
                const Vector2f& groundA, const Vector2f& groundB,
                const Vector2f& anchorA, const Vector2f& anchorB, f32 r)
        : groundAnchorA(groundA), groundAnchorB(groundB)
        , localAnchorA(anchorA), localAnchorB(anchorB)
        , ratio(r) {
        type = JointType::Pulley;
        bodyA = a;
        bodyB = b;
        Vector2f diffA = bodyA->position + localAnchorA - groundAnchorA;
        Vector2f diffB = bodyB->position + localAnchorB - groundAnchorB;
        maxLengthA = diffA.length();
        maxLengthB = diffB.length() / ratio;
    }

    Vector2f getReactionForce() const override {
        return Vector2f(0, 0);
    }

    f32 getReactionTorque() const override {
        return 0.0f;
    }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;

        Vector2f worldAnchorA = bodyA->position + localAnchorA;
        Vector2f worldAnchorB = bodyB->position + localAnchorB;

        Vector2f diffA = worldAnchorA - groundAnchorA;
        Vector2f diffB = worldAnchorB - groundAnchorB;
        f32 lenA = diffA.length();
        f32 lenB = diffB.length();

        if (lenA < 1e-8f || lenB < 1e-8f) return;

        f32 constraintB = lenB - maxLengthB * ratio;
        if (constraintB > 0) {
            Vector2f correction = diffB.normalized() * (-constraintB * stiffness);
            bodyB->applyForce(-correction);
            bodyA->applyForce(correction * ratio);
        }

        if (lenA > maxLengthA) {
            Vector2f correction = diffA.normalized() * (maxLengthA - lenA) * stiffness;
            bodyA->applyForce(-correction);
        }
    }

    f32 stiffness = 100.0f;
    f32 damping = 1.0f;
};

struct GearJoint : public Joint {
    Joint* jointA;
    Joint* jointB;
    f32 ratio;

    GearJoint()
        : jointA(nullptr), jointB(nullptr), ratio(1.0f) {
        type = JointType::Gear;
    }

    GearJoint(Joint* ja, Joint* jb, f32 r)
        : jointA(ja), jointB(jb), ratio(r) {
        type = JointType::Gear;
        if (ja) bodyA = ja->bodyA;
        if (jb) bodyB = jb->bodyA;
    }

    Vector2f getReactionForce() const override {
        return Vector2f(0, 0);
    }

    f32 getReactionTorque() const override {
        if (!jointA || !jointB) return 0.0f;
        return (jointA->getReactionTorque() - ratio * jointB->getReactionTorque()) * 10.0f;
    }

    void solve(f32 dt) override {
        if (!jointA || !jointB) return;

        RigidBody* a1 = jointA->bodyA;
        RigidBody* a2 = jointA->bodyB;
        RigidBody* b1 = jointB->bodyA;
        RigidBody* b2 = jointB->bodyB;

        if (!a1 || !a2 || !b1 || !b2) return;

        f32 angleA = a2->rotation - a1->rotation;
        f32 angleB = b2->rotation - b1->rotation;
        f32 constraint = angleA - ratio * angleB;

        f32 correction = -constraint * 100.0f;
        f32 dampingForce = (a2->angularVelocity - a1->angularVelocity -
                           ratio * (b2->angularVelocity - b1->angularVelocity)) * 1.0f;
        correction -= dampingForce;

        a1->applyTorque(-correction);
        a2->applyTorque(correction);
        b1->applyTorque(correction * ratio);
        b2->applyTorque(-correction * ratio);
    }
};

struct WheelJoint : public Joint {
    Vector2f localAnchorA;
    Vector2f localAnchorB;
    Vector2f localAxisA;
    f32 suspensionStiffness;
    f32 suspensionDamping;
    f32 maxSuspensionTravel;
    f32 motorSpeed;
    f32 maxMotorTorque;
    f32 wheelRadius;

    WheelJoint()
        : localAnchorA(0, 0), localAnchorB(0, 0)
        , localAxisA(0, -1)
        , suspensionStiffness(20.0f), suspensionDamping(5.0f)
        , maxSuspensionTravel(0.5f)
        , motorSpeed(0.0f), maxMotorTorque(0.0f)
        , wheelRadius(0.5f) {
        type = JointType::Wheel;
    }

    WheelJoint(RigidBody* chassis, RigidBody* wheel,
               const Vector2f& anchor, const Vector2f& axis)
        : localAnchorA(anchor), localAnchorB(0, 0)
        , localAxisA(axis)
        , suspensionStiffness(20.0f), suspensionDamping(5.0f)
        , maxSuspensionTravel(0.5f)
        , motorSpeed(0.0f), maxMotorTorque(0.0f)
        , wheelRadius(0.5f) {
        type = JointType::Wheel;
        bodyA = chassis;
        bodyB = wheel;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB) return Vector2f(0, 0);
        return (bodyB->position - bodyA->position - localAnchorA) * suspensionStiffness;
    }

    f32 getReactionTorque() const override {
        return 0.0f;
    }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB) return;

        Vector2f worldAnchorA = bodyA->position + localAnchorA;
        Vector2f worldAnchorB = bodyB->position + localAnchorB;

        Vector2f suspensionDir = localAxisA;
        Vector2f diff = worldAnchorB - worldAnchorA;
        f32 projection = diff.dot(suspensionDir);

        if (std::abs(projection) > maxSuspensionTravel) {
            f32 correction = (maxSuspensionTravel - std::abs(projection)) * std::sign(projection);
            Vector2f suspensionForce = suspensionDir * correction * suspensionStiffness;

            Vector2f relVel = bodyB->linearVelocity - bodyA->linearVelocity;
            f32 dampingForce = relVel.dot(suspensionDir) * suspensionDamping;
            suspensionForce -= suspensionDir * dampingForce;

            bodyA->applyForce(-suspensionForce);
            bodyB->applyForce(suspensionForce);
        }

        if (std::abs(maxMotorTorque) > 0.0f) {
            f32 currentAngVel = bodyB->angularVelocity;
            f32 motorTorque = (motorSpeed - currentAngVel) * maxMotorTorque;
            motorTorque = std::clamp(motorTorque, -maxMotorTorque, maxMotorTorque);
            bodyB->applyTorque(motorTorque);
            bodyA->applyTorque(-motorTorque * 0.1f);
        }

        Vector2f lateralDir(-suspensionDir.y, suspensionDir.x);
        Vector2f lateralVel = (bodyB->linearVelocity - bodyA->linearVelocity);
        f32 lateralImpulse = lateralVel.dot(lateralDir) * 0.5f;
        bodyA->applyForce(lateralDir * lateralImpulse);
        bodyB->applyForce(-lateralDir * lateralImpulse);
    }

    void setMotorSpeed(f32 speed) { motorSpeed = speed; }
    void setMaxMotorTorque(f32 torque) { maxMotorTorque = torque; }
    void setSuspension(f32 stiffness, f32 damping) {
        suspensionStiffness = stiffness;
        suspensionDamping = damping;
    }
};

struct WeldJoint : public Joint {
    Vector2f localAnchorA;
    Vector2f localAnchorB;
    f32 referenceAngle;
    f32 breakForce;
    f32 breakTorque;
    bool broken;

    WeldJoint()
        : localAnchorA(0, 0), localAnchorB(0, 0)
        , referenceAngle(0)
        , breakForce(std::numeric_limits<f32>::max())
        , breakTorque(std::numeric_limits<f32>::max())
        , broken(false) {
        type = JointType::Weld;
    }

    WeldJoint(RigidBody* a, RigidBody* b,
              const Vector2f& anchorA, const Vector2f& anchorB,
              f32 refAngle, f32 maxForce = std::numeric_limits<f32>::max(),
              f32 maxTorque = std::numeric_limits<f32>::max())
        : localAnchorA(anchorA), localAnchorB(anchorB)
        , referenceAngle(refAngle)
        , breakForce(maxForce), breakTorque(maxTorque)
        , broken(false) {
        type = JointType::Weld;
        bodyA = a;
        bodyB = b;
    }

    Vector2f getReactionForce() const override {
        if (!bodyA || !bodyB || broken) return Vector2f(0, 0);
        Vector2f worldA = bodyA->position + localAnchorA;
        Vector2f worldB = bodyB->position + localAnchorB;
        return (worldB - worldA) * 100.0f;
    }

    f32 getReactionTorque() const override {
        if (!bodyA || !bodyB || broken) return 0.0f;
        return (bodyB->rotation - bodyA->rotation - referenceAngle) * 100.0f;
    }

    void solve(f32 dt) override {
        if (!bodyA || !bodyB || broken) return;

        Vector2f worldA = bodyA->position + localAnchorA;
        Vector2f worldB = bodyB->position + localAnchorB;
        Vector2f posError = worldB - worldA;
        f32 angError = bodyB->rotation - bodyA->rotation - referenceAngle;

        Vector2f correction = -posError * 100.0f;
        f32 angCorrection = -angError * 100.0f;

        f32 forceMag = correction.length();
        f32 torqueMag = std::abs(angCorrection);

        if (forceMag > breakForce || torqueMag > breakTorque) {
            broken = true;
            return;
        }

        bodyA->applyForce(-correction);
        bodyB->applyForce(correction);
        bodyA->applyTorque(-angCorrection);
        bodyB->applyTorque(angCorrection);
    }

    void setBreakForce(f32 force) { breakForce = force; }
    void setBreakTorque(f32 torque) { breakTorque = torque; }
    bool isBroken() const { return broken; }
};

} // namespace nebula
