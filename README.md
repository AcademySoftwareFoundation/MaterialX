# MaterialX

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/materialx/MaterialX/blob/master/LICENSE.txt)
[![Version](https://badge.fury.io/gh/materialx%2FMaterialX.svg)](https://github.com/materialx/MaterialX/releases/latest)
[![Build Status](https://github.com/MaterialX/MaterialX/workflows/build/badge.svg?branch=master)](https://github.com/MaterialX/MaterialX/actions?query=branch%3Amaster)

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

### Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org/) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.
- Select the `MATERIALX_BUILD_JS` option to build JavaScript bindings.
- Select the `MATERIALX_BUILD_VIEWER` option to build the MaterialX viewer.

### Supported Platforms

The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:

- Microsoft Visual Studio 2015 or newer
- GCC 4.8 or newer
- Clang 3.3 or newer

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and support Python versions 2.7 and 3.x.

The JavaScript bindings for MaterialX are based on [emscripten](https://emscripten.org/).

### Repository

The MaterialX repository consists of the following folders:

- [documents](documents) - MaterialX documentation, including the specification and developer guide.
- [libraries](libraries) - The standard data libraries for MaterialX, including definitions for the standard and physically-based-shading nodes.
- [python](python) - Support modules and example scripts for MaterialX Python.
- [javascript](source/JsMaterialX) - Support modules and tests for MaterialX JavaScript.
- [resources](resources) - Resources for rendering MaterialX content, including example materials, images, and geometry.
- [source](source) - Cross-platform C++ libraries for MaterialX with Python bindings.

### MaterialX Viewer

The [MaterialX Viewer](documents/DeveloperGuide/Viewer.md) leverages shader generation to build GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.

**Figure 1:** Procedural and uniform materials in the MaterialX viewer
<p float="left">
  <img src="/documents/Images/MaterialXView_Marble.png" width="206" />
  <img src="/documents/Images/MaterialXView_Copper.png" width="206" /> 
  <img src="/documents/Images/MaterialXView_Plastic.png" width="206" /> 
  <img src="/documents/Images/MaterialXView_Carpaint.png" width="206" /> 
</p>

**Figure 2:** Textured, color-space-managed materials in the MaterialX viewer
<p float="left">
  <img src="/documents/Images/MaterialXView_TiledBrass.png" width="416" />
  <img src="/documents/Images/MaterialXView_TiledWood.png" width="416" /> 
</p>

**Figure 3:** Droid character materials in the MaterialX viewer. © & TM Lucasfilm Ltd. Used with permission.
<p float="left">
  <img src="/documents/Images/MaterialXView_BB8.png" width="416" />
  <img src="/documents/Images/MaterialXView_R2D2.png" width="416" /> 
</p>

### Pre-Built Binaries

The following packages contain pre-built binaries for the latest release, including the MaterialX viewer, Python libraries, and example assets:

- [Microsoft Windows (Visual Studio 2017, 64-bit, Python 3.7)](https://github.com/materialx/MaterialX/releases/latest/download/MaterialX_Windows_VS2017_x64_Python37.zip)
- [Microsoft Windows (Visual Studio 2019, 64-bit, Python 3.8)](https://github.com/materialx/MaterialX/releases/latest/download/MaterialX_Windows_VS2019_x64_Python38.zip)

### Additional Resources

- The [Developer Guide](http://www.materialx.org/docs/api/index.html) contains a developer-oriented overview of MaterialX with API documentation.
- The [Python Scripts](python/Scripts/README.md) folder contains standalone examples of MaterialX Python code.
- The [JavaScript](source/JsMaterialX/README.md) source folder contains more information about the setup.
 Presentations at the [Physically Based Shading Course](https://blog.selfshadow.com/publications/s2020-shading-course/#materialx) and [MaterialX Birds of a Feather](http://www.materialx.org/assets/MaterialX_Sig2020_BOF_slides.pdf) provide details on the roadmap for MaterialX development.
