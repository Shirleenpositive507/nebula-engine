#include "RigidBody.h"

namespace nebula {

void RigidBody::applyImpulse(const Vector2f& impulse, const Vector2f& contactPoint) {
    linearVelocity += impulse * inverseMass;
    Vector2f r = contactPoint - position;
    angularVelocity += inverseInertia * r.cross(impulse);
    wakeUp();
}

void RigidBody::applyForceAtPoint(const Vector2f& f, const Vector2f& point) {
    force += f;
    Vector2f r = point - position;
    torque += r.cross(f);
    wakeUp();
}

void RigidBody::applyBuoyancyForce(const BuoyancyParams& params) {
    if (!collider) return;

    Rectf bounds = collider->getBounds();
    Vector2f worldPos = position + bounds.getCenter();
    f32 bodyBottom = position.y + bounds.getMaxY();
    f32 bodyTop = position.y + bounds.getMinY();

    if (bodyBottom <= params.fluidHeight) return;

    f32 submergedHeight = bodyBottom - params.fluidHeight;
    if (submergedHeight > bodyBottom - bodyTop) {
        submergedHeight = bodyBottom - bodyTop;
    }

    f32 submergedFraction = submergedHeight / (bodyBottom - bodyTop);
    if (submergedFraction > 1.0f) submergedFraction = 1.0f;
    if (submergedFraction < 0.0f) submergedFraction = 0.0f;

    f32 displacedVolume = mass / params.fluidDensity * submergedFraction;
    Vector2f buoyancyForce(0, params.fluidDensity * displacedVolume * 9.81f);

    force += buoyancyForce;

    Vector2f dragForce = (params.fluidVelocity - linearVelocity) * params.fluidLinearDrag * submergedFraction;
    force += dragForce;

    torque += -angularVelocity * params.fluidAngularDrag * submergedFraction;
}

void RigidBody::setMassFromDensity(f32 density) {
    if (!collider) return;
    f32 area = 0.0f;
    switch (collider->getType()) {
        case ColliderType::Box: {
            auto* box = static_cast<BoxCollider*>(collider);
            area = box->width * box->height;
            break;
        }
        case ColliderType::Circle: {
            auto* circle = static_cast<CircleCollider*>(collider);
            area = 3.14159265f * circle->radius * circle->radius;
            break;
        }
        case ColliderType::Polygon: {
            auto* poly = static_cast<PolygonCollider*>(collider);
            for (size_t i = 0; i < poly->vertices.size(); ++i) {
                const Vector2f& a = poly->vertices[i];
                const Vector2f& b = poly->vertices[(i + 1) % poly->vertices.size()];
                area += a.cross(b);
            }
            area = std::abs(area) / 2.0f;
            break;
        }
    }
    setMass(area * density);
}

void RigidBody::updateInertia() {
    if (fixedRotation || type == BodyType::Static) {
        inertia = 0.0f;
        inverseInertia = 0.0f;
        return;
    }
    if (!collider) {
        inertia = 1.0f;
        inverseInertia = 1.0f;
        return;
    }
    switch (collider->getType()) {
        case ColliderType::Box: {
            auto* box = static_cast<BoxCollider*>(collider);
            inertia = mass * (box->width * box->width + box->height * box->height) / 12.0f;
            break;
        }
        case ColliderType::Circle: {
            auto* circle = static_cast<CircleCollider*>(collider);
            inertia = mass * circle->radius * circle->radius / 2.0f;
            break;
        }
        case ColliderType::Polygon: {
            auto* poly = static_cast<PolygonCollider*>(collider);
            f32 num = 0.0f, den = 0.0f;
            for (size_t i = 0; i < poly->vertices.size(); ++i) {
                const Vector2f& a = poly->vertices[i];
                const Vector2f& b = poly->vertices[(i + 1) % poly->vertices.size()];
                f32 cross = std::abs(a.cross(b));
                num += cross * (a.dot(a) + b.dot(b) + a.dot(b));
                den += cross;
            }
            inertia = mass * num / (6.0f * den);
            break;
        }
    }
    inverseInertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f;
}

bool RigidBody::canSleep() const {
    if (type != BodyType::Dynamic) return true;
    f32 speedSq = linearVelocity.lengthSquared();
    f32 angSpeed = std::abs(angularVelocity);
    return (speedSq < sleepThreshold * sleepThreshold) && (angSpeed < sleepThreshold);
}

void RigidBody::trySleep() {
    if (type == BodyType::Static || type == BodyType::Kinematic) {
        asleep = true;
        return;
    }
    if (canSleep()) {
        asleep = true;
        linearVelocity = Vector2f(0, 0);
        angularVelocity = 0.0f;
    }
}

void RigidBody::wakeUp() {
    if (asleep) {
        asleep = false;
    }
}

void RigidBody::integrateForces(f32 dt) {
    if (type != BodyType::Dynamic) return;

    if (asleep) {
        linearVelocity = Vector2f(0, 0);
        angularVelocity = 0.0f;
        return;
    }

    linearVelocity += force * inverseMass * dt;
    angularVelocity += torque * inverseInertia * dt;

    if (linearDamping > 0.0f) linearVelocity *= std::max(1.0f - linearDamping * dt, 0.0f);
    if (angularDamping > 0.0f) angularVelocity *= std::max(1.0f - angularDamping * dt, 0.0f);
}

void RigidBody::integrateVelocities(f32 dt) {
    if (type != BodyType::Dynamic) return;
    if (asleep) return;

    Vector2f displacement = linearVelocity * dt;
    if (freezePositionX) displacement.x = 0;
    if (freezePositionY) displacement.y = 0;
    position += displacement;

    if (!freezeRotation) {
        rotation += angularVelocity * dt;
    }

    if (getKineticEnergy() < activationEnergy) {
        trySleep();
    }
}

} // namespace nebula
