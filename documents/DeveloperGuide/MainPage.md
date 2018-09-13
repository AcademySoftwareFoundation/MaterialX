# MaterialX Overview {#mainpage}

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

### Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.

### Supported Platforms

The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:

- Microsoft Visual Studio 2015 or newer
- GCC 4.8 or newer
- Clang 3.3 or newer

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and currently support Python versions 2.6, 2.7, and 3.x.

### Building MaterialX Python

By default, the `MATERIALX_BUILD_PYTHON` option will use the active version of Python in the developer's path.  To select a specific version of Python, use one or more of the following advanced options:

- `MATERIALX_PYTHON_VERSION`: Python version to be used in building the MaterialX Python package (e.g. `2.7`)
- `MATERIALX_PYTHON_EXECUTABLE`: Python executable to be used in building the MaterialX Python package (e.g. `C:/Python27/python.exe`)

Additional options for the generation of MaterialX Python include the following:

- `MATERIALX_PYTHON_OCIO_DIR`: Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python. The recommended OpenColorIO configuration for MaterialX is [ACES 1.0.3](https://github.com/imageworks/OpenColorIO-Configs/tree/master/aces_1.0.3).
- `MATERIALX_PYTHON_PYBIND11_DIR`: Path to a folder containing the PyBind11 source. Defaults to the included PyBind11 source, which has been extended to support Python 2.6.

### Building API Documentation

To generate HTML documentation for the MaterialX C++ API, make sure a version of [Doxygen](https://www.doxygen.org/) is on your path, and select the advanced option `MATERIALX_BUILD_DOCS` in CMake.  This option will add a target named `MaterialXDocs` to your project, which can be built as an independent step from your development environment.

### Installing MaterialX

Building the `install` target of your project will install the MaterialX C++ and Python libraries to the folder specified by the `CMAKE_INSTALL_PREFIX` setting, and will install MaterialX Python as a third-party library in your Python environment.  Installation of MaterialX Python as a third-party library can be disabled by setting `MATERIALX_INSTALL_PYTHON` to `OFF`.

### Additional Links

- The main [MaterialX website](http://www.materialx.org)
- [Code Examples](@ref codeexamples) in C++ and Python
