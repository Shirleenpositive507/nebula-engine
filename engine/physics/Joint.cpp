#include "Joint.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace nebula {

static f32 clampAngle(f32 angle) {
    while (angle > 3.14159265f) angle -= 2.0f * 3.14159265f;
    while (angle < -3.14159265f) angle += 2.0f * 3.14159265f;
    return angle;
}

void DistanceJoint::solve(f32 dt) {
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

void SpringJoint::solve(f32 dt) {
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

void HingeJoint::solve(f32 dt) {
    if (!bodyA || !bodyB) return;
    f32 angleDiff = clampAngle(bodyB->rotation - bodyA->rotation);
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

void FixedJoint::solve(f32 dt) {
    if (!bodyA || !bodyB) return;

    Vector2f worldA = bodyA->position + localAnchorA;
    Vector2f worldB = bodyB->position + localAnchorB;
    Vector2f posError = worldB - worldA;
    f32 angError = clampAngle(bodyB->rotation - bodyA->rotation - referenceAngle);

    Vector2f correction = -posError * 100.0f;
    f32 angCorrection = -angError * 100.0f;

    bodyA->applyForce(-correction);
    bodyB->applyForce(correction);
    bodyA->applyTorque(-angCorrection);
    bodyB->applyTorque(angCorrection);
}

void PulleyJoint::solve(f32 dt) {
    if (!bodyA || !bodyB) return;

    Vector2f worldAnchorA = bodyA->position + localAnchorA;
    Vector2f worldAnchorB = bodyB->position + localAnchorB;

    Vector2f diffA = worldAnchorA - groundAnchorA;
    Vector2f diffB = worldAnchorB - groundAnchorB;
    f32 lenA = diffA.length();
    f32 lenB = diffB.length();

    if (lenA < 1e-8f || lenB < 1e-8f) return;

    Vector2f dirA = diffA / lenA;
    Vector2f dirB = diffB / lenB;

    f32 constraintC = lenA + ratio * lenB - maxLengthA - ratio * maxLengthB;

    f32 effectiveMass = bodyA->inverseMass + (ratio * ratio) * bodyB->inverseMass;
    if (effectiveMass < 1e-10f) return;

    f32 impulse = -constraintC / effectiveMass;
    impulse = std::max(impulse, 0.0f);

    Vector2f impulseA = dirA * impulse;
    Vector2f impulseB = dirB * (impulse * ratio);

    bodyA->applyImpulse(impulseA, worldAnchorA);
    bodyB->applyImpulse(-impulseB, worldAnchorB);

    if (lenA > maxLengthA) {
        Vector2f limitForce = dirA * (maxLengthA - lenA) * stiffness;
        bodyA->applyForce(-limitForce);
    }

    if ((lenB / ratio) > maxLengthB) {
        Vector2f limitForce = dirB * (maxLengthB * ratio - lenB) * stiffness;
        bodyB->applyForce(-limitForce);
    }
}

void GearJoint::solve(f32 dt) {
    if (!jointA || !jointB) return;

    RigidBody* a1 = jointA->bodyA;
    RigidBody* a2 = jointA->bodyB;
    RigidBody* b1 = jointB->bodyA;
    RigidBody* b2 = jointB->bodyB;

    if (!a1 || !a2 || !b1 || !b2) return;

    f32 angleA = clampAngle(a2->rotation - a1->rotation);
    f32 angleB = clampAngle(b2->rotation - b1->rotation);
    f32 constraint = angleA - ratio * angleB;

    f32 invMassA = a1->inverseInertia + a2->inverseInertia;
    f32 invMassB = (b1->inverseInertia + b2->inverseInertia) * ratio * ratio;
    f32 effectiveMass = invMassA + invMassB;
    if (effectiveMass < 1e-10f) return;

    f32 correction = -constraint / effectiveMass;
    f32 angularVelError = (a2->angularVelocity - a1->angularVelocity) -
                          ratio * (b2->angularVelocity - b1->angularVelocity);
    f32 dampingForce = angularVelError * damping * 0.1f;
    correction -= dampingForce / effectiveMass;

    a1->applyTorque(-correction);
    a2->applyTorque(correction);
    b1->applyTorque(correction * ratio);
    b2->applyTorque(-correction * ratio);
}

void WheelJoint::solve(f32 dt) {
    if (!bodyA || !bodyB) return;

    Vector2f worldAnchorA = bodyA->position + localAnchorA;
    Vector2f worldAnchorB = bodyB->position + localAnchorB;

    Vector2f suspensionDir = localAxisA;
    Vector2f diff = worldAnchorB - worldAnchorA;
    f32 projection = diff.dot(suspensionDir);

    if (std::abs(projection) > maxSuspensionTravel) {
        f32 correction = (maxSuspensionTravel - std::abs(projection)) *
                         ((projection > 0) ? 1.0f : -1.0f);
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
    Vector2f lateralVel = bodyB->linearVelocity - bodyA->linearVelocity;
    f32 lateralImpulse = lateralVel.dot(lateralDir) * 0.5f;
    bodyA->applyForce(lateralDir * lateralImpulse);
    bodyB->applyForce(-lateralDir * lateralImpulse);
}

void WeldJoint::solve(f32 dt) {
    if (!bodyA || !bodyB || broken) return;

    Vector2f worldA = bodyA->position + localAnchorA;
    Vector2f worldB = bodyB->position + localAnchorB;
    Vector2f posError = worldB - worldA;
    f32 angError = clampAngle(bodyB->rotation - bodyA->rotation - referenceAngle);

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

} // namespace nebula
