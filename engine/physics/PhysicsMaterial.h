#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>
#include "core/Types.h"

namespace nebula {

enum class MaterialCombineMode {
    Multiply,
    Min,
    Max,
    Average
};

struct PhysicsMaterial {
    f32 density;
    f32 restitution;
    f32 friction;
    f32 staticFriction;
    f32 dynamicFriction;
    std::string name;
    u32 materialId;
    MaterialCombineMode frictionCombine;
    MaterialCombineMode restitutionCombine;

    PhysicsMaterial()
        : density(1.0f), restitution(0.5f), friction(0.5f)
        , staticFriction(0.6f), dynamicFriction(0.4f)
        , name("Default")
        , materialId(0)
        , frictionCombine(MaterialCombineMode::Multiply)
        , restitutionCombine(MaterialCombineMode::Multiply) {}

    PhysicsMaterial(f32 density, f32 restitution, f32 friction,
                    f32 staticFriction, f32 dynamicFriction,
                    const std::string& matName = "Default",
                    u32 id = 0,
                    MaterialCombineMode fricMode = MaterialCombineMode::Multiply,
                    MaterialCombineMode restMode = MaterialCombineMode::Multiply)
        : density(density), restitution(restitution), friction(friction)
        , staticFriction(staticFriction), dynamicFriction(dynamicFriction)
        , name(matName)
        , materialId(id)
        , frictionCombine(fricMode)
        , restitutionCombine(restMode) {}

    f32 getRestitution() const { return restitution; }
    f32 getFriction() const { return friction; }
    f32 getDensity() const { return density; }
    u32 getId() const { return materialId; }
    std::string getName() const { return name; }

    void setFrictionCombineMode(MaterialCombineMode mode) { frictionCombine = mode; }
    void setRestitutionCombineMode(MaterialCombineMode mode) { restitutionCombine = mode; }
    MaterialCombineMode getFrictionCombineMode() const { return frictionCombine; }
    MaterialCombineMode getRestitutionCombineMode() const { return restitutionCombine; }

    static const PhysicsMaterial Default;
    static const PhysicsMaterial Wood;
    static const PhysicsMaterial Metal;
    static const PhysicsMaterial Stone;
    static const PhysicsMaterial Rubber;
    static const PhysicsMaterial Ice;
    static const PhysicsMaterial BouncyBall;
    static const PhysicsMaterial Frictionless;

    static f32 combineFriction(f32 a, f32 b, MaterialCombineMode mode);
    static f32 combineRestitution(f32 a, f32 b, MaterialCombineMode mode);
    static f32 combineProperty(f32 a, f32 b, MaterialCombineMode mode);
};

struct MaterialPair {
    PhysicsMaterial materialA;
    PhysicsMaterial materialB;

    MaterialPair() = default;
    MaterialPair(const PhysicsMaterial& a, const PhysicsMaterial& b)
        : materialA(a), materialB(b) {}

    f32 getCombinedRestitution() const {
        return PhysicsMaterial::combineRestitution(
            materialA.restitution, materialB.restitution,
            materialA.restitutionCombine);
    }

    f32 getCombinedFriction() const {
        return PhysicsMaterial::combineFriction(
            materialA.friction, materialB.friction,
            materialA.frictionCombine);
    }

    f32 getCombinedStaticFriction() const {
        return PhysicsMaterial::combineFriction(
            materialA.staticFriction, materialB.staticFriction,
            materialA.frictionCombine);
    }

    f32 getCombinedDynamicFriction() const {
        return PhysicsMaterial::combineFriction(
            materialA.dynamicFriction, materialB.dynamicFriction,
            materialA.frictionCombine);
    }
};

struct MaterialOverride {
    bool overrideDensity;
    bool overrideRestitution;
    bool overrideFriction;
    bool overrideStaticFriction;
    bool overrideDynamicFriction;
    f32 density;
    f32 restitution;
    f32 friction;
    f32 staticFriction;
    f32 dynamicFriction;

    MaterialOverride()
        : overrideDensity(false)
        , overrideRestitution(false)
        , overrideFriction(false)
        , overrideStaticFriction(false)
        , overrideDynamicFriction(false)
        , density(1.0f)
        , restitution(0.5f)
        , friction(0.5f)
        , staticFriction(0.6f)
        , dynamicFriction(0.4f) {}

    void apply(PhysicsMaterial& material) const {
        if (overrideDensity) material.density = density;
        if (overrideRestitution) material.restitution = restitution;
        if (overrideFriction) material.friction = friction;
        if (overrideStaticFriction) material.staticFriction = staticFriction;
        if (overrideDynamicFriction) material.dynamicFriction = dynamicFriction;
    }
};

class MaterialRegistry {
public:
    MaterialRegistry() {
        registerMaterial("Default", PhysicsMaterial::Default);
        registerMaterial("Wood", PhysicsMaterial::Wood);
        registerMaterial("Metal", PhysicsMaterial::Metal);
        registerMaterial("Stone", PhysicsMaterial::Stone);
        registerMaterial("Rubber", PhysicsMaterial::Rubber);
        registerMaterial("Ice", PhysicsMaterial::Ice);
        registerMaterial("BouncyBall", PhysicsMaterial::BouncyBall);
        registerMaterial("Frictionless", PhysicsMaterial::Frictionless);
    }

    void registerMaterial(const std::string& name, const PhysicsMaterial& material) {
        m_materials[name] = material;
    }

    void unregisterMaterial(const std::string& name) {
        m_materials.erase(name);
    }

    PhysicsMaterial* getMaterial(const std::string& name) {
        auto it = m_materials.find(name);
        if (it != m_materials.end()) {
            return &it->second;
        }
        return nullptr;
    }

    bool hasMaterial(const std::string& name) const {
        return m_materials.find(name) != m_materials.end();
    }

    std::size_t getMaterialCount() const {
        return m_materials.size();
    }

    MaterialPair getMaterialPair(const std::string& nameA, const std::string& nameB) {
        PhysicsMaterial a = PhysicsMaterial::Default;
        PhysicsMaterial b = PhysicsMaterial::Default;
        auto itA = m_materials.find(nameA);
        if (itA != m_materials.end()) a = itA->second;
        auto itB = m_materials.find(nameB);
        if (itB != m_materials.end()) b = itB->second;
        return MaterialPair(a, b);
    }

private:
    std::unordered_map<std::string, PhysicsMaterial> m_materials;
};

inline f32 PhysicsMaterial::combineFriction(f32 a, f32 b, MaterialCombineMode mode) {
    return combineProperty(a, b, mode);
}

inline f32 PhysicsMaterial::combineRestitution(f32 a, f32 b, MaterialCombineMode mode) {
    return combineProperty(a, b, mode);
}

inline f32 PhysicsMaterial::combineProperty(f32 a, f32 b, MaterialCombineMode mode) {
    switch (mode) {
        case MaterialCombineMode::Multiply: return a * b;
        case MaterialCombineMode::Min:      return std::min(a, b);
        case MaterialCombineMode::Max:      return std::max(a, b);
        case MaterialCombineMode::Average:  return (a + b) * 0.5f;
    }
    return a * b;
}

} // namespace nebula
