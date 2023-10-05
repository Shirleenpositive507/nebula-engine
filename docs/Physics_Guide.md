# Physics System Guide

## Setting Up the Physics World

```cpp
#include <Nebula/Physics.h>

auto& physicsWorld = nebula::phys::World::create();
physicsWorld.setGravity({0, 980}); // pixels/s²
```

## Body Types

| Type | Description |
|------|-------------|
| `Static` | Immovable, infinite mass |
| `Dynamic` | Affected by forces and collisions |
| `Kinematic` | Moves under user control, pushes dynamic bodies |

```cpp
auto body = physicsWorld.createBody(phys::BodyType::Dynamic);
body->setPosition({400, 300});
body->setMass(1.0f);
```

## Colliders

```cpp
// AABB collider
auto box = body->addCollider<phys::BoxCollider>({32, 32});

// Circle collider
auto circle = body->addCollider<phys::CircleCollider>(16);

// Polygon collider
std::vector<vec2> vertices = {{0,0}, {32,0}, {16,32}};
auto poly = body->addCollider<phys::PolygonCollider>(vertices);
```

## Joints

```cpp
// Pin joint (fixed point)
auto pin = physicsWorld.createJoint<phys::PinJoint>(bodyA, {400, 300});

// Spring joint
auto spring = physicsWorld.createJoint<phys::SpringJoint>(bodyA, bodyB);
spring->setStiffness(50.0f);
spring->setDamping(0.5f);
```

## Raycasting

```cpp
auto result = physicsWorld.raycast({0, 0}, {800, 600});
if (result.hit) {
    // result.point, result.normal, result.body
}
```

## Collision Callbacks

```cpp
body->onCollision = [](phys::CollisionInfo& info) {
    // info.otherBody, info.contactPoint, info.relativeVelocity
    if (info.otherBody->hasTag("enemy")) {
        // Handle enemy collision
    }
};
```

## Physics Layers

Use layers to control which bodies interact:

```cpp
body->setLayer(1);    // Player layer
body->setMask(2 | 4); // Collide with layers 2 and 4 only
```
