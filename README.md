# FESceneGraphUI

A reusable scene graph UI component built on Dear ImGui. It provides a hierarchical tree view (`SceneGraphUI::TreeView`) for visualizing and interacting with scene graph nodes. The widget is decoupled from any specific engine through a `SceneGraphUI::BackendInterface`, with a ready-made backend for the [Focal Engine](https://github.com/Azzinoth/FocalEngine).

## Architecture

- `SceneGraphUI::BackendInterface` - abstract interface your data source implements (root, children, parent, IDs, names, tags, move/rename, liveness).
- `SceneGraphUI::NodeHandle` - opaque handle the widget passes around; wraps a backend pointer + ID.
- `SceneGraphUI::TreeView` - the ImGui widget. Construct with a `BackendInterface*` and call `Render()` per frame.
- `FESceneGraphBackend` (optional) - default backend for Focal Engine.

To use FESceneGraphUI with a non Focal-Engine project, implement `BackendInterface` against your own scene representation; no Focal Engine dependency is needed.

## Integration

FESceneGraphUI is designed to be used as a Git submodule. Dear ImGui is required; Focal Engine is only required if you opt into the bundled backend.

### Setup

Add as a submodule:
```bash
git submodule add https://github.com/Azzinoth/FESceneGraphUI SubSystems/FESceneGraphUI
```

Minimal CMake (custom backend, no Focal Engine):
```cmake
set(DEAR_IMGUI_INCLUDE_DIR "path/to/imgui" CACHE PATH "" FORCE)
add_subdirectory(path/to/FESceneGraphUI)

target_link_libraries(YourProject PRIVATE FESceneGraphUI)
```

With the Focal Engine backend (add after FocalEngine):
```cmake
set(DEAR_IMGUI_INCLUDE_DIR "path/to/imgui" CACHE PATH "" FORCE)
set(FE_SCENE_GRAPH_UI_WITH_FOCAL_BACKEND ON CACHE BOOL "" FORCE)

add_subdirectory(path/to/FocalEngine)
add_subdirectory(path/to/FESceneGraphUI)

target_link_libraries(YourProject PRIVATE FocalEngine FESceneGraphUI FESceneGraphUI_FocalBackend)

```

## Focal Engine Ecosystem

The Focal Engine project consists of four modular components that work together to provide a complete development environment:

[Basic Application Module](https://github.com/Azzinoth/FEBasicApplication) - A foundation layer for OpenGL and ImGui applications that provides essential utilities including time measurement, thread pooling, logging, TCP networking, and profiling capabilities.

[Visual Node System](https://github.com/Azzinoth/VisualNodeSystem) - A framework for creating visual node-based interfaces with features like zoom, reroute nodes, group comments, and JSON serialization, ideal for material editors and visual scripting.

[Focal Engine](https://github.com/Azzinoth/FocalEngine) - The engine with all core functionality.

Focal Engine Scene Graph UI (this repository) - A reusable scene graph UI component, that provides a hierarchical tree view for visualizing and interacting with scene graph nodes.

[Focal Engine Editor](https://github.com/Azzinoth/FocalEngineEditor) - A comprehensive editor for the engine.

This modularity makes it easier to include just the engine in applications that don't need the editor's complexity. It also simplifies the implementation of export functionality in the editor, allowing users to compile their projects into standalone executable applications with all necessary resources.

## Projects Using FESceneGraphUI

[Focal Engine Editor](https://github.com/Azzinoth/FocalEngineEditor) - A comprehensive editor for the engine.

[HabiCAT 3D](https://github.com/Azzinoth/HabiCAT3D) - An open-source software that implements novel algorithms for generating multi-scale complexity metrics maps(like rugosity, fractal dimension, vector dispersion and others) for complex 3D habitat models.