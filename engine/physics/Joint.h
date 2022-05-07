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
    Fixed
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

} // namespace nebula
