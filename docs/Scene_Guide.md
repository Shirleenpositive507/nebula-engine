# Scene Management Guide

## Creating Scenes

Scenes encapsulate a complete game state with entities, systems, and logic:

```cpp
#include <Nebula/Scene.h>

class GameScene : public nebula::scene::Scene {
public:
    void onEnter() override {
        // Initialize entities
        auto player = createEntity();
        addComponent<Transform>(player, {400, 300});
        addComponent<Sprite>(player, "player.png");

        // Register systems
        addSystem<MovementSystem>();
        addSystem<RenderSystem>();
    }

    void onUpdate(float dt) override {
        // Scene-specific logic
        if (nebula::Input::isKeyPressed(Key::Escape)) {
            sceneManager->push<PauseScene>();
        }
    }

    void onExit() override {
        // Cleanup
    }
};
```

## Scene Transitions

```cpp
// Push scene (pause current)
sceneManager->push<GameScene>();

// Replace current scene
sceneManager->replace<GameScene>();

// Pop back to previous scene
sceneManager->pop();

// With transition effect
sceneManager->push<GameScene>(nebula::scene::FadeTransition::create(0.5f));
sceneManager->replace<MenuScene>(nebula::scene::SlideTransition::create(
    Direction::Left, 0.3f
));
```

Available transition types:
- `FadeTransition` - Cross-fade between scenes
- `SlideTransition` - Slide in direction
- `ZoomTransition` - Zoom in/out
- `PixelateTransition` - Pixelation dissolve

## Serialization

Scenes can be serialized to JSON:

```cpp
// Save scene
scene->saveToFile("savegame.json");

// Load scene
auto scene = Scene::loadFromFile("savegame.json");

// Serialize individual entities
json entityData = scene->serializeEntity(playerEntity);
```

## Prefabs

Reusable entity templates:

```cpp
auto enemyPrefab = nebula::scene::Prefab::create();
enemyPrefab->addComponent<Transform>();
enemyPrefab->addComponent<Sprite>("enemy.png");
enemyPrefab->addComponent<Health>(100);
enemyPrefab->addComponent<AIComponent>();

// Instantiate multiple enemies
for (int i = 0; i < 10; i++) {
    auto enemy = enemyPrefab->instantiate();
    auto& transform = getComponent<Transform>(enemy);
    transform.position = {100.0f + i * 80, 200.0f};
}

// Save/load prefabs
enemyPrefab->saveToFile("prefabs/enemy.prefab");
auto loaded = nebula::scene::Prefab::loadFromFile("prefabs/enemy.prefab");
```
