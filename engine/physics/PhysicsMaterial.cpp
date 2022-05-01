#include "PhysicsMaterial.h"

namespace nebula {

const PhysicsMaterial PhysicsMaterial::Default(1.0f, 0.5f, 0.5f, 0.6f, 0.4f);
const PhysicsMaterial PhysicsMaterial::Wood(0.8f, 0.3f, 0.6f, 0.7f, 0.5f);
const PhysicsMaterial PhysicsMaterial::Metal(7.8f, 0.2f, 0.4f, 0.5f, 0.3f);
const PhysicsMaterial PhysicsMaterial::Stone(2.5f, 0.1f, 0.8f, 0.9f, 0.7f);
const PhysicsMaterial PhysicsMaterial::Rubber(1.2f, 0.9f, 0.7f, 0.8f, 0.6f);
const PhysicsMaterial PhysicsMaterial::Ice(0.9f, 0.05f, 0.05f, 0.1f, 0.03f);
const PhysicsMaterial PhysicsMaterial::BouncyBall(1.5f, 0.95f, 0.3f, 0.4f, 0.2f);
const PhysicsMaterial PhysicsMaterial::Frictionless(1.0f, 0.5f, 0.0f, 0.0f, 0.0f);

} // namespace nebula
