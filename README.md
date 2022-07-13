<p align="center">
  <img src="documents/Images/MaterialXLogo.png" height="170" />
</p>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/LICENSE)
[![Version](https://img.shields.io/github/v/release/AcademySoftwareFoundation/MaterialX)](https://github.com/AcademySoftwareFoundation/MaterialX/releases/latest)
[![Build Status](https://github.com/AcademySoftwareFoundation/MaterialX/workflows/main/badge.svg?branch=main)](https://github.com/AcademySoftwareFoundation/MaterialX/actions?query=branch%3Amain)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/6025/badge)](https://bestpractices.coreinfrastructure.org/projects/6025)

### Introduction

MaterialX is an open standard for representing rich material and look-development content in computer graphics, enabling its platform-independent description and exchange across applications and renderers.  Launched at [Industrial Light & Magic](https://www.ilm.com/) in 2012, MaterialX has been a key technology in their feature films and real-time experiences since _Star Wars: The Force Awakens_ and _Millennium Falcon: Smugglers Run_.  The project was released as open source in 2017, with companies including Sony Pictures Imageworks, Pixar, Autodesk, Adobe, and SideFX contributing to its ongoing development.  In 2021, MaterialX became the seventh hosted project of the [Academy Software Foundation](https://www.aswf.io/).

### Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org/) build system.
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
- [javascript](javascript) - Support modules and example scripts for MaterialX JavaScript.
- [libraries](libraries) - The standard data libraries for MaterialX, including definitions for the pattern and shading nodes.
- [python](python) - Support modules and example scripts for MaterialX Python.
- [resources](resources) - Resources for rendering MaterialX content, including example materials, images, and geometry.
- [source](source) - Cross-platform C++ libraries for MaterialX with Python bindings.

### MaterialX Viewer

The [MaterialX Viewer](documents/DeveloperGuide/Viewer.md) leverages shader generation to build GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.

**Figure 1:** Procedural and uniform materials in the MaterialX viewer
<p float="left">
  <img src="/documents/Images/MaterialXView_Marble.png" width="204" />
  <img src="/documents/Images/MaterialXView_Copper.png" width="204" /> 
  <img src="/documents/Images/MaterialXView_Plastic.png" width="204" /> 
  <img src="/documents/Images/MaterialXView_Carpaint.png" width="204" /> 
</p>

**Figure 2:** Textured, color-space-managed materials in the MaterialX viewer
<p float="left">
  <img src="/documents/Images/MaterialXView_TiledBrass.png" width="412" />
  <img src="/documents/Images/MaterialXView_TiledWood.png" width="412" /> 
</p>

### Pre-Built Binaries

The following packages contain pre-built binaries for the latest release, including the MaterialX viewer, Python libraries, and example assets:

- [Microsoft Windows (Visual Studio 2022, Python 3.9)](https://github.com/AcademySoftwareFoundation/MaterialX/releases/latest/download/MaterialX_Windows_VS2022_x64_Python39.zip)
- [MacOS (Xcode 13, Python 3.9)](https://github.com/AcademySoftwareFoundation/MaterialX/releases/latest/download/MaterialX_MacOS_Xcode_13_Python39.zip)
- [Linux (GCC 11, Python 3.9)](https://github.com/AcademySoftwareFoundation/MaterialX/releases/latest/download/MaterialX_Linux_GCC_11_Python39.zip)

### Additional Resources

- The [Developer Guide](http://www.materialx.org/docs/api/index.html) contains a developer-oriented overview of MaterialX with API documentation.
- The [Python Scripts](python/Scripts) folder contains standalone examples of MaterialX Python code.
- The [JavaScript](javascript) folder contains details on building JavaScript bindings for MaterialX.
- Presentations at the [Physically Based Shading Course](https://blog.selfshadow.com/publications/s2020-shading-course/#materialx) and [ASWF Open Source Days](http://www.materialx.org/assets/ASWF_OSD2021_MaterialX_slides_final.pdf) provide details on the roadmap for MaterialX development.
