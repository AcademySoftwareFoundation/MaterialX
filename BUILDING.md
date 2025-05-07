# Building MaterialX

This document includes information on getting setup with building MaterialX

## Quick Start for Developers

If you're developing directly with CMake:
- 

- Download the latest version of the [CMake](https://cmake.org/) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.
- Select the `MATERIALX_BUILD_VIEWER` option to build
  the [MaterialX Viewer](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/DeveloperGuide/Viewer.md).
- Select the `MATERIALX_BUILD_GRAPH_EDITOR` option to build
  the [MaterialX Graph Editor](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/DeveloperGuide/GraphEditor.md).

## Platform Specific Build instructions

MaterialX operates on a range of platforms. Some of them require some extra configuration and are listed here.

### macOS

macOS requires you to [install Xcode](https://developer.apple.com/xcode/resources/) to get access to the Metal Tools.

## Editor Specific Instructions

MaterialX should work in any editor that supports CMake, or that CMake can generate a project for.
Some common Editors are listed here to help developers get started.

### CLion

[CLion](https://www.jetbrains.com/clion/) is a cross-platform IDE that can be used to develop MaterialX.
Additionally, it includes CMake and is free for Non-Commercial Use.

To get started with CLion, open the MaterialX repository directly, and it will load the CMake project for you.
If you want to enable features like Python, go to `Settings > Build, Execution and Deployment > CMake` and configure
the CMake Options, for example

```
-DMATERIALX_BUILD_PYTHON=ON
-DMATERIALX_BUILD_VIEWER=ON
-DMATERIALX_BUILD_GRAPH_EDITOR=ON
```

To build, either select `Build>Build Project` or select a specific configuration to build.
To install, select `Build>Install` 