# Graphics Pipeline Guide

## Renderer

The renderer provides a hardware-accelerated 2D rendering pipeline with automatic sprite batching.

```cpp
nebula::gfx::Renderer renderer;

// Begin frame
renderer.begin(camera);

// Draw sprites
renderer.draw(sprite);

// End frame and present
renderer.end();
```

## Sprite Batching

Sprites with the same texture are automatically batched for optimal draw call performance:

```cpp
auto& sprite = ecs::World::addComponent<gfx::Sprite>(entity);
sprite.setTexture("tileset.png");
sprite.setTextureRect({0, 0, 32, 32});
sprite.setColor({255, 255, 255, 255});
```

## Cameras

```cpp
// Orthographic camera (default)
auto camera = gfx::Camera::createOrthographic(1280, 720);

// Follow target
camera.setFollowTarget(playerEntity, {0, 0}); // offset

// Shake effect
camera.shake(5.0f, 0.3f); // intensity, duration
```

## Shaders

```cpp
auto shader = gfx::Shader::load("shaders/vertex.glsl", "shaders/fragment.glsl");
shader->setUniform("uTime", time);
shader->setUniform("uColor", vec4{1, 0, 0, 1});

renderer.setShader(shader);
```

## Post-Processing

```cpp
auto bloom = gfx::PostProcessor::create(gfx::Effect::Bloom);
bloom->setParameter("intensity", 0.8f);
bloom->setParameter("threshold", 0.6f);

auto crt = gfx::PostProcessor::create(gfx::Effect::CRT);
crt->setParameter("scanlineIntensity", 0.3f);

renderer.addPostProcessor(bloom);
renderer.addPostProcessor(crt);
```

## Particle Systems

```cpp
auto particleSystem = gfx::ParticleSystem::create(1000);
particleSystem->setEmitter({
    .position = {400, 300},
    .spawnRate = 100,
    .lifetime = {0.5f, 2.0f},
    .speed = {50, 200},
    .color = {1, 0.5, 0, 1},
    .fadeOut = true
});

particleSystem->update(dt);
particleSystem->render(renderer);
```

## Lighting

```cpp
auto light = gfx::Light::create(gfx::LightType::Point);
light->setPosition({400, 300});
light->setRadius(200);
light->setColor({1, 0.8, 0.6});
light->setIntensity(1.5f);

renderer.addLight(light);
```
