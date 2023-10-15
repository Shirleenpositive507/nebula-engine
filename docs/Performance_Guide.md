# Performance Optimization Guide

## Draw Call Batching

### Texture Atlases
Combine multiple textures into a single atlas to minimize state changes:

```cpp
// Generate a texture atlas
auto atlas = nebula::gfx::TextureAtlas::create(1024, 1024);
atlas->addRegion("player", "player.png");
atlas->addRegion("enemy", "enemy.png");
atlas->addRegion("bullet", "bullet.png");
atlas->pack();

// Use atlas regions
auto& sprite = ecs::World::addComponent<gfx::Sprite>(entity);
sprite.setTexture(atlas->getTexture());
sprite.setTextureRect(atlas->getRegion("player"));
```

### Sprite Batching
The renderer automatically batches sprites by texture. To maximize batching:
- Sort sprites by texture
- Use the same shader for similar sprites
- Minimize individual state changes

### Instanced Rendering
For大量 identical sprites (particles, bullets):

```cpp
auto instancedSprite = gfx::InstancedSprite::create("particle.png", 10000);
instancedSprite->setInstanceData(positions, colors, sizes);
renderer.drawInstanced(instancedSprite);
```

## Object Pooling

Avoid frequent allocation/deallocation:

```cpp
template<typename T>
class ObjectPool {
    std::vector<T> objects;
    std::vector<size_t> freeList;
public:
    size_t acquire() {
        if (freeList.empty()) {
            objects.emplace_back();
            return objects.size() - 1;
        }
        size_t index = freeList.back();
        freeList.pop_back();
        return index;
    }

    void release(size_t index) {
        freeList.push_back(index);
    }

    T& get(size_t index) { return objects[index]; }
};
```

## Spatial Partitioning

### Grid-Based Partitioning
```cpp
nebula::utils::SpatialGrid grid(32); // 32px cell size

// Insert entities
grid.insert(entity1, boundingBox1);
grid.insert(entity2, boundingBox2);

// Query nearby entities
auto nearby = grid.query(region, QueryType::All);

// Remove when entity moves
grid.update(entity, oldBounds, newBounds);
```

### Quadtree
For scenes with sparse entity distribution:

```cpp
auto quadtree = nebula::utils::Quadtree::create(sceneBounds, 8, 16);
// maxDepth=8, maxObjects=16
```

## Profiling

Use the built-in profiler to identify bottlenecks:

```cpp
#include <Nebula/Profiler.h>

void update() {
    NEBULA_PROFILE_FUNCTION();

    {
        NEBULA_PROFILE_SCOPE("Physics Update");
        physicsWorld.update(dt);
    }

    {
        NEBULA_PROFILE_SCOPE("AI Update");
        aiSystem.update(dt);
    }
}

// Dump profile data
nebula::utils::Profiler::dumpToFile("profile_results.json");
```

### Profile Viewing
Use Chrome's `chrome://tracing` to visualize the JSON output.

## Memory Optimization

### Component Packing
Group frequently accessed components together in memory:

```cpp
// Access pattern: Position and Velocity are always read together
ecs::World::pack<Position, Velocity>();
```

### Custom Allocators
```cpp
// Linear allocator for frame-scoped data
nebula::utils::LinearAllocator frameAllocator(1024 * 1024); // 1 MB
frameAllocator.reset(); // Call at end of frame
```

### Cache-Friendly Iteration
```cpp
// Good: Sequential memory access
auto view = ecs::World::query<Position>();
for (auto [entity, pos] : view) {
    // positions stored contiguously
}

// Avoid: Random access
for (auto entity : someRandomOrder) {
    auto& pos = ecs::World::getComponent<Position>(entity);
}
```
