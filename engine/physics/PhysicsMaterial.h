#pragma once
#include "core/Types.h"

namespace nebula {

struct PhysicsMaterial {
    f32 density;
    f32 restitution;
    f32 friction;
    f32 staticFriction;
    f32 dynamicFriction;

    PhysicsMaterial()
        : density(1.0f), restitution(0.5f), friction(0.5f)
        , staticFriction(0.6f), dynamicFriction(0.4f) {}

    PhysicsMaterial(f32 density, f32 restitution, f32 friction,
                    f32 staticFriction, f32 dynamicFriction)
        : density(density), restitution(restitution), friction(friction)
        , staticFriction(staticFriction), dynamicFriction(dynamicFriction) {}

    f32 getRestitution() const { return restitution; }
    f32 getFriction() const { return friction; }
    f32 getDensity() const { return density; }

    static const PhysicsMaterial Default;
    static const PhysicsMaterial Wood;
    static const PhysicsMaterial Metal;
    static const PhysicsMaterial Stone;
    static const PhysicsMaterial Rubber;
    static const PhysicsMaterial Ice;
    static const PhysicsMaterial BouncyBall;
    static const PhysicsMaterial Frictionless;
};

struct MaterialPair {
    PhysicsMaterial materialA;
    PhysicsMaterial materialB;

    MaterialPair() = default;
    MaterialPair(const PhysicsMaterial& a, const PhysicsMaterial& b)
        : materialA(a), materialB(b) {}

    f32 getCombinedRestitution() const {
        return materialA.restitution * materialB.restitution;
    }

    f32 getCombinedFriction() const {
        return std::sqrt(materialA.friction * materialB.friction);
    }

    f32 getCombinedStaticFriction() const {
        return std::sqrt(materialA.staticFriction * materialB.staticFriction);
    }

    f32 getCombinedDynamicFriction() const {
        return std::sqrt(materialA.dynamicFriction * materialB.dynamicFriction);
    }
};

} // namespace nebula
