# Example Projects

## Platformer Example

A complete 2D platformer game demonstrating core engine features.

### Building

```bash
cd examples/platformer
cmake -B build
cmake --build build
./build/platformer
```

### Controls

| Key | Action |
|-----|--------|
| Arrow Keys / WASD | Movement |
| Space | Jump |
| E | Interact |
| Escape | Pause |

### Code Walkthrough

**Player Controller:**
```cpp
class PlayerController : public ecs::System {
public:
    void update(float dt) override {
        auto view = ecs::World::query<Transform, Velocity, PlayerTag>();
        for (auto [entity, transform, velocity, _] : view) {
            velocity.x = 0;
            if (Input::isKeyHeld(Key::Right)) velocity.x = 200;
            if (Input::isKeyHeld(Key::Left)) velocity.x = -200;
            if (Input::isKeyPressed(Key::Space) && isGrounded(entity)) {
                velocity.y = -400;
            }
        }
    }
};
```

**Collision Handling:**
```cpp
class CollisionSystem : public ecs::System {
    void update(float dt) override {
        auto view = ecs::World::query<Transform, BoxCollider>();
        // ... collision resolution
    }
};
```

## SHMUP Example

A side-scrolling shoot-em-up demonstrating particles, pooling, and enemy AI.

### Building

```bash
cd examples/shmup
cmake -B build
cmake --build build
./build/shmup
```

### Controls

| Key | Action |
|-----|--------|
| Arrow Keys | Move |
| Z | Shoot |
| X | Bomb |
| Shift | Slow mode |

### Code Walkthrough

**Bullet Pooling:**
```cpp
class BulletPool {
    static constexpr int POOL_SIZE = 500;
    std::vector<ecs::Entity> pool;

public:
    void init() {
        for (int i = 0; i < POOL_SIZE; i++) {
            auto bullet = ecs::World::createEntity();
            ecs::World::addComponent<Transform>(bullet);
            ecs::World::addComponent<Sprite>(bullet, "bullet.png");
            ecs::World::addComponent<Velocity>(bullet);
            ecs::World::setActive(bullet, false);
            pool.push_back(bullet);
        }
    }

    ecs::Entity spawn(vec2 position, vec2 velocity) {
        if (pool.empty()) return ecs::null;
        auto bullet = pool.back();
        pool.pop_back();
        ecs::World::setActive(bullet, true);
        ecs::World::getComponent<Transform>(bullet).position = position;
        ecs::World::getComponent<Velocity>(bullet).value = velocity;
        return bullet;
    }

    void despawn(ecs::Entity bullet) {
        ecs::World::setActive(bullet, false);
        pool.push_back(bullet);
    }
};
```

**Enemy Wave System:**
```cpp
class WaveManager : public ecs::System {
    int wave = 0;
    float spawnTimer = 0;

    void update(float dt) override {
        spawnTimer -= dt;
        if (spawnTimer <= 0) {
            spawnEnemyWave(wave);
            wave++;
            spawnTimer = std::max(1.0f, 5.0f - wave * 0.2f);
        }
    }
};
```

## Running All Examples

```bash
cmake -B build -DNEBULA_BUILD_EXAMPLES=ON
cmake --build build

# List available examples
./build/examples/list

# Run specific example
./build/examples/platformer/platformer
./build/examples/shmup/shmup
```
