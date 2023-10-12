# Best Practices

## ECS Patterns

### Favor Composition Over Inheritance
```cpp
// Good: Use ECS components
struct Movable {};
struct Health { int hp; };

// Avoid: Deep inheritance trees
class Entity {};
class LivingEntity : public Entity {};
class Player : public LivingEntity {};
```

### Keep Components Small
Components should be plain data with no logic. One component per responsibility:
```cpp
// Good
struct Position { vec2 value; };
struct Velocity { vec2 value; };

// Avoid
struct PhysicsData {
    vec2 position;
    vec2 velocity;
    float mass;
    bool isGrounded;
    // ...
};
```

### Batch Component Operations
```cpp
// Good: Single query
auto view = ecs::World::query<Position, Velocity>();
for (auto [entity, pos, vel] : view) {
    pos.value += vel.value * dt;
}

// Avoid: Per-entity lookups in loops
for (auto entity : entities) {
    auto& pos = ecs::World::getComponent<Position>(entity);
    auto& vel = ecs::World::getComponent<Velocity>(entity);
    pos.value += vel.value * dt;
}
```

## Performance Tips

### Object Pooling
Reuse frequently created/destroyed objects:
```cpp
class BulletPool {
    std::vector<ecs::Entity> pool;
public:
    ecs::Entity acquire() {
        if (pool.empty()) {
            auto e = ecs::World::createEntity();
            ecs::World::addComponent<Transform>(e);
            ecs::World::addComponent<Sprite>(e, "bullet.png");
            return e;
        }
        auto e = pool.back();
        pool.pop_back();
        ecs::World::setActive(e, true);
        return e;
    }

    void release(ecs::Entity e) {
        ecs::World::setActive(e, false);
        pool.push_back(e);
    }
};
```

### Spatial Partitioning
Use spatial hashing or quadtrees for broad-phase collision:
```cpp
auto& grid = nebula::utils::SpatialGrid::create(64); // cell size
grid.insert(entity, boundingBox);
auto neighbors = grid.query(region);
```

### Minimize Draw Calls
- Use texture atlases
- Batch sprites by texture
- Use sprite sheets for animations

## Memory Management

### Pre-allocate Component Pools
```cpp
ecs::World::reserve<Transform>(5000);
ecs::World::reserve<Sprite>(3000);
```

### Use Stack Allocation for Temporary Data
```cpp
// Good
std::array<ecs::Entity, 64> tempEntities;
size_t count = 0;

// Avoid
std::vector<ecs::Entity> tempEntities;
tempEntities.reserve(64);
```

### Defragment Regularly
```cpp
// Run during loading screens
ecs::World::defragment();
```

## Code Organization

### Project Structure
```
src/
├── components/    # Component definitions
├── systems/       # System implementations
├── scenes/        # Scene definitions
├── ui/           # Custom widgets
└── main.cpp      # Entry point
```

### Naming Conventions
- Classes: `PascalCase`
- Methods: `PascalCase`
- Variables: `camelCase`
- Namespaces: `snake_case`
- Files: `PascalCase` matching class name
