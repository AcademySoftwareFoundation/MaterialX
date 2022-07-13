# MaterialX Overview {#mainpage}

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

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

### Building MaterialX

#### Building MaterialX C++

The MaterialX C++ libraries are automatically included when building MaterialX through CMake.

To enable OpenImageIO support in MaterialX builds, the following additional options may be used:

- `MATERIALX_BUILD_OIIO`: Requests that MaterialXRender be built with OpenImageIO in addition to stb_image, extending the set of supported image formats.
- `MATERIALX_OIIO_DIR`: Path to the root folder of an OpenImageIO installation.  If MATERIALX_BUILD_OIIO has been enabled, then this option may be used to select which installation is used.

To enable Open Shading Language compiler and render validation in MaterialX builds, the following additional options may be used:

- `MATERIALX_OSL_BINARY_OSLC`: Path to the OSL compiler binary (e.g. `oslc.exe`).
- `MATERIALX_OSL_BINARY_TESTRENDER`: Path to the OSL test render binary (e.g. `testrender.exe`).
- `MATERIALX_OSL_INCLUDE_PATH`: Path to the OSL shader include folder, which contains headers such as `stdosl.h`.

#### Building MaterialX Python

By default, the `MATERIALX_BUILD_PYTHON` option will use the active version of Python in the developer's path.  To select a specific version of Python, use one or more of the following advanced options:

- `MATERIALX_PYTHON_VERSION`: Python version to be used in building the MaterialX Python package (e.g. `2.7`)
- `MATERIALX_PYTHON_EXECUTABLE`: Python executable to be used in building the MaterialX Python package (e.g. `C:/Python27/python.exe`)

Additional options for the generation of MaterialX Python include the following:

- `MATERIALX_PYTHON_OCIO_DIR`: Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python. The recommended OpenColorIO configuration for MaterialX is [ACES 1.2](https://github.com/colour-science/OpenColorIO-Configs/tree/feature/aces-1.2-config/aces_1.2).
- `MATERIALX_PYTHON_PYBIND11_DIR`: Path to a folder containing the PyBind11 source to be used in building MaterialX Python. Defaults to the included PyBind11 source.

#### Building The MaterialX Viewer

Select the `MATERIALX_BUILD_VIEWER` option to build the MaterialX Viewer.  Installation will copy the **MaterialXView** executable to a `bin/` directory within the selected install folder.

#### Building API Documentation

To generate HTML documentation for the MaterialX C++ API, make sure a version of [Doxygen](https://www.doxygen.org/) is on your path, and select the advanced option `MATERIALX_BUILD_DOCS` in CMake.  This option will add a target named `MaterialXDocs` to your project, which can be built as an independent step from your development environment.

### Installing MaterialX

Building the `install` target of your project will install the MaterialX C++ and Python libraries to the folder specified by the `CMAKE_INSTALL_PREFIX` setting, and will install MaterialX Python as a third-party library in your Python environment.  Installation of MaterialX Python as a third-party library can be disabled by setting `MATERIALX_INSTALL_PYTHON` to `OFF`.

### MaterialX Versioning

The MaterialX codebase uses a modified semantic versioning system where the *major* and *minor* versions match that of the corresponding MaterialX [specification](https://www.materialx.org/Specification.html), and the *build* version represents engineering advances within that specification version.  MaterialX documents are similarly marked with the specification version they were authored in, and they are valid to load into any MaterialX codebase with an equal or higher specification version.

Upgrading of MaterialX documents from earlier versions is handled at import time by the Document::upgradeVersion method, which applies the syntax and node interface upgrades that have occurred in previous specification revisions.  This allows the syntax conventions of MaterialX and the names and interfaces of nodes to evolve over time, without invalidating documents from earlier versions.

### Additional Links

- The main [MaterialX website](http://www.materialx.org) provides background on the project's history, industry collaborations, and recent presentations.
- The [Python Scripts](https://github.com/materialx/MaterialX/tree/main/python/Scripts) folder contains standalone examples of MaterialX Python code.
- The [MaterialX Unit Tests](https://github.com/materialx/MaterialX/tree/main/source/MaterialXTest) folder contains examples of useful patterns for MaterialX C++.
- The [MaterialX Viewer](https://github.com/materialx/MaterialX/blob/main/documents/DeveloperGuide/Viewer.md) is a complete, cross-platform C++ application based upon [MaterialX Shader Generation](https://github.com/materialx/MaterialX/blob/main/documents/DeveloperGuide/ShaderGeneration.md)
