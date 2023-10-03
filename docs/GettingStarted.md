# Getting Started with Nebula Engine

## Prerequisites

- C++17 compatible compiler
- CMake 3.14+
- SFML 2.6+

## Setting Up Your First Project

Create a new C++ file and include the engine header:

```cpp
#include <Nebula/Engine.h>
#include <Nebula/ECS.h>
#include <Nebula/Graphics.h>

using namespace nebula;

class MyGame : public Application {
public:
    MyGame() : Application("My First Game", 1280, 720) {}

    void onInit() override {
        // Create a camera
        camera = gfx::Camera::createOrthographic(1280, 720);

        // Create a player entity
        player = ecs::World::createEntity();
        ecs::World::addComponent<gfx::Sprite>(player, "player.png");
        ecs::World::addComponent<Transform>(player, {400, 300, 0});
    }

    void onUpdate(float dt) override {
        // Move player with arrow keys
        auto& transform = ecs::World::getComponent<Transform>(player);
        if (Input::isKeyPressed(Key::Left))  transform.position.x -= 200 * dt;
        if (Input::isKeyPressed(Key::Right)) transform.position.x += 200 * dt;
        if (Input::isKeyPressed(Key::Up))    transform.position.y -= 200 * dt;
        if (Input::isKeyPressed(Key::Down))  transform.position.y += 200 * dt;
    }

    void onRender() override {
        Renderer::begin(camera);
        Renderer::drawSprites();
        Renderer::end();
    }

private:
    ecs::Entity player;
    gfx::Camera camera;
};

int main() {
    MyGame game;
    game.run();
    return 0;
}
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Next Steps

- Read the [ECS Tutorial](ECS_Tutorial.md) for a deeper dive into the entity component system.
- Check the [Graphics Guide](Graphics_Guide.md) for advanced rendering techniques.
