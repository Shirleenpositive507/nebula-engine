# ECS Architecture Tutorial

## Overview

Nebula Engine uses an Entity Component System (ECS) architecture for optimal performance and flexibility. Entities are lightweight IDs, components are plain data, and systems contain the logic.

## Creating Components

Components are plain structs with no logic:

```cpp
struct Health : public ecs::Component {
    int current = 100;
    int max = 100;
};

struct Damage : public ecs::Component {
    int amount = 10;
    bool一次 = false;
};
```

## Creating Systems

Systems process entities with specific component combinations:

```cpp
class DamageSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = ecs::World::query<Health, Damage>();
        for (auto [entity, health, damage] : view) {
            health.current -= damage.amount;
            ecs::World::removeComponent<Damage>(entity);
            if (health.current <= 0) {
                ecs::World::destroyEntity(entity);
            }
        }
    }
};
```

## Entity Management

```cpp
// Create an entity with components
auto player = ecs::World::createEntity();
ecs::World::addComponent<Transform>(player, {0, 0, 0});
ecs::World::addComponent<Sprite>(player, "hero.png");
ecs::World::addComponent<Health>(player);

// Query entities
auto enemies = ecs::World::query<Transform, EnemyTag>();
for (auto [entity, transform, _] : enemies) {
    // Process each enemy
}

// Destroy entity
ecs::World::destroyEntity(player);
```

## Prefabs

Prefabs allow reusable entity templates:

```cpp
auto bulletPrefab = ecs::Prefab::create();
bulletPrefab->addComponent<Transform>();
bulletPrefab->addComponent<Sprite>("bullet.png");
bulletPrefab->addComponent<Velocity>({0, -500});

// Instantiate
auto bullet = bulletPrefab->instantiate();
ecs::World::getComponent<Transform>(bullet).position = {400, 300};
```

## System Ordering

Control system execution order when registering:

```cpp
ecs::World::addSystem<InputSystem>(SystemOrder::First);
ecs::World::addSystem<PhysicsSystem>(SystemOrder::Early);
ecs::World::addSystem<DamageSystem>(SystemOrder::Normal);
ecs::World::addSystem<RenderSystem>(SystemOrder::Last);
```
