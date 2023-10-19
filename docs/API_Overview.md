# Nebula Engine API Overview

## Namespaces

| Namespace | Description |
|-----------|-------------|
| `nebula` | Root namespace for all engine types |
| `nebula::ecs` | Entity Component System |
| `nebula::gfx` | Graphics and rendering |
| `nebula::phys` | Physics simulation |
| `nebula::audio` | Audio playback and spatialization |
| `nebula::ui` | User interface widgets and layouts |
| `nebula::scene` | Scene management and transitions |
| `nebula::net` | Networking (TCP/UDP) |
| `nebula::math` | Math primitives (Vector2/3/4, Matrix3/4, Quaternion) |
| `nebula::utils` | Utility functions and helpers |
| `nebula::editor` | Editor and debug tools |

## Main Classes

### Core
- `Application` - Main application entry point, manages the game loop
- `Window` - Window and input management
- `Time` - Frame timing and delta time utilities

### ECS
- `World` - Entity and component container
- `Entity` - Lightweight entity handle
- `Component` - Base component type
- `System` - Base system type for processing entities
- `Query` - Entity query builder for filtered iteration

### Graphics
- `Renderer` - 2D hardware-accelerated renderer
- `Sprite` - Renderable sprite with texture and transform
- `Camera` - Viewport and projection camera
- `Shader` - GLSL shader program wrapper
- `Texture` - Texture resource management
- `ParticleSystem` - Particle emitter and updater
- `Light` - 2D light source
- `PostProcessor` - Post-processing effects pipeline
- `TextureAtlas` - Texture atlas generation and region management
- `InstancedSprite` - Instanced rendering for large sprite counts
- `ShaderLibrary` - Built-in shader collection

### Physics
- `World` - Physics simulation world
- `Body` - Rigid body (static, dynamic, kinematic)
- `Collider` - Collision shape (AABB, circle, polygon)
- `Joint` - Constraint between bodies
- `RaycastResult` - Ray intersection query result
- `PhysicsLayer` - Collision layer and mask system

### Audio
- `Sound` - Short sound effect playback
- `Music` - Streamed music playback
- `AudioSource` - 3D positional audio source
- `AudioListener` - Listener position and orientation
- `Effect` - Audio effect (reverb, echo, etc.)
- `AudioStream` - Procedural/network audio streaming
- `AudioGroup` - Grouped volume and mute control

### UI
- `Widget` - Base UI widget
- `Button` - Clickable button
- `Label` - Text label
- `Panel` - Container panel
- `Layout` - Layout manager (horizontal, vertical, grid)
- `Style` - Widget styling properties
- `Slider` - Value slider
- `TextInput` - Text input field

### Scene
- `Scene` - Scene with entities and systems
- `SceneManager` - Scene stack and transitions
- `Prefab` - Reusable entity template
- `FadeTransition` - Cross-fade transition effect
- `SlideTransition` - Slide-direction transition
- `ZoomTransition` - Zoom in/out transition

### Networking
- `Server` - TCP/UDP server
- `Client` - TCP/UDP client
- `Packet` - Serialized network message
- `Connection` - Active connection handle
- `Reliability` - Packet reliability channels

### Editor
- `Editor` - Main editor controller
- `Viewport` - Scene viewport widget
- `EntityInspector` - Entity property editor
- `Console` - In-game debug console
- `ProfilerPanel` - Real-time performance profiling UI
- `LogViewer` - Filterable log output viewer
