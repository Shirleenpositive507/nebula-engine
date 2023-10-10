# Editor Usage Guide

## Overview

The Nebula Editor is a built-in tool for live game editing, debugging, and prototyping.

## Viewport Navigation

| Input | Action |
|-------|--------|
| Middle mouse drag | Pan viewport |
| Scroll wheel | Zoom in/out |
| Right-click drag | Orbit camera (3D) |
| F | Focus on selected entity |

## Entity Editing

### Scene Hierarchy
The entity tree shows all entities in the current scene. Drag to re-parent, right-click for context menu.

### Inspector Panel
Select an entity to view and edit its components:
- Add/remove components via the "Add Component" button
- Edit component properties in real-time
- Transform gizmos for position, rotation, scale

### Creating Entities
```cpp
// Editor context menu actions are generated automatically
// from registered component types
```

## Project Management

### Project Structure
```
MyProject/
├── assets/       # Textures, audio, fonts
├── scenes/       # .scene files
├── prefabs/      # .prefab files
├── scripts/      # Game scripts
└── project.json  # Project configuration
```

### Project Settings
- Resolution and aspect ratio
- Default scene
- Asset paths
- Build configuration

## Debugging Tools

### Console
Open with `~` key. Supports:
- Lua/script execution
- Variable inspection
- Command history

### Profiler
Real-time performance metrics:
- FPS counter
- Draw call count
- Memory usage
- System update times

### Debug Draw
Toggle visualizations:
- Physics colliders
- Spatial partition boundaries
- Light influence areas
- Pathfinding nodes

### Log Viewer
Filterable log output with severity levels:
- Info (white)
- Warning (yellow)
- Error (red)

## Hot Reloading

The editor supports hot-reloading for:
- Textures and sprites
- Audio files
- Shader programs
- Script files

Simply modify a file in the assets folder and the editor will detect the change.
