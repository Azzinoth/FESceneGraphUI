# FESceneGraphUI

A reusable scene graph UI component for the [Focal Engine](https://github.com/Azzinoth/FocalEngine). It provides a hierarchical tree view for visualizing and interacting with scene graph nodes using ImGui.

## Integration

FESceneGraphUI is designed to be used as a Git submodule. It depends on [Focal Engine](https://github.com/Azzinoth/FocalEngine) but does not include it, parent project is responsible for providing the FocalEngine target.

### Setup

Add as a submodule:
```bash
git submodule add https://github.com/Azzinoth/FESceneGraphUI SubSystems/FESceneGraphUI
```

In your CMakeLists.txt, add FESceneGraphUI **after** FocalEngine:
```cmake
add_subdirectory(path/to/FocalEngine)
add_subdirectory(path/to/FESceneGraphUI)

target_link_libraries(YourProject PRIVATE FocalEngine FESceneGraphUI)
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