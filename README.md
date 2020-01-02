# MaterialX

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/materialx/MaterialX/blob/master/LICENSE.txt)
[![Travis Build Status](https://travis-ci.com/materialx/MaterialX.svg?branch=master)](https://travis-ci.com/materialx/MaterialX)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/pmlxnp5m1fve11k0?svg=true)](https://ci.appveyor.com/project/jstone-lucasfilm/materialx)

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

### Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org/) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.
- Select the `MATERIALX_BUILD_VIEWER` option to build the MaterialX viewer.

### Supported Platforms

The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:

- Microsoft Visual Studio 2015 or newer
- GCC 4.8 or newer
- Clang 3.3 or newer

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and support Python versions 2.7 and 3.x.

### Repository

The MaterialX repository consists of the following folders:

- [documents](documents) - MaterialX documentation, including the specification and developer guide.
- [libraries](libraries) - The standard data libraries for MaterialX, including definitions for the standard and physically-based-shading nodes.
- [python](python) - Support modules and example scripts for MaterialX Python.
- [resources](resources) - Resources for rendering MaterialX content, including example materials, images, and geometry.
- [source](source) - Cross-platform C++ libraries for MaterialX with Python bindings.

### MaterialX Viewer

The [MaterialX Viewer](documents/DeveloperGuide/Viewer.md) leverages shader generation to build GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.

**Figure 1:** Standard Surface Shader with procedural and uniform materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024"></p>

**Figure 2:** Standard Surface Shader with textured, color-space-managed materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="640"></p>

### Additional Resources

- The [Developer Guide](http://www.materialx.org/docs/api/index.html) contains a developer-oriented overview of MaterialX with API documentation.
- The [Python Scripts](python/Scripts/README.md) folder contains standalone examples of MaterialX Python code.
- Presentations at the [Autodesk Vision Series](https://area.autodesk.com/blogs/thebuzz/open-source-at-autodesk-materialx/) provide details on the roadmap for MaterialX development.
