# MaterialX

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/materialx/MaterialX/blob/master/LICENSE.txt)
[![Travis Build Status](https://travis-ci.org/materialx/MaterialX.svg?branch=master)](https://travis-ci.org/materialx/MaterialX)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/pmlxnp5m1fve11k0?svg=true)](https://ci.appveyor.com/project/jstone-lucasfilm/materialx)

MaterialX is an open standard for transfer of rich material and look-development content between applications and renderers.  Originated at Lucasfilm in 2012, MaterialX has been used by Industrial Light & Magic (ILM) in feature films such as _Star Wars: The Force Awakens_ and real-time experiences such as _Trials on Tatooine_, and it remains the central material format for new ILM productions.

### Quick Start for Developers

-   Download the latest version of the [CMake](https://cmake.org/) build system.
-   Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
-   Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.

### Supported Platforms

The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:

-   Microsoft Visual Studio 2015 or newer
-   GCC 4.8 or newer
-   Clang 3.3 or newer

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and currently support Python versions 2.6, 2.7, and 3.x.

### Repository

The MaterialX repository consists of the following folders:

-   [source](source) - Cross-platform C++ libraries for MaterialX with Python bindings.
-   [python](python) - Support modules for MaterialX Python.
-   [documents](documents) - MaterialX documentation, including its specification and developer guides.
-   [libraries](libraries) - The standard data libraries for MaterialX, including the definitions of its pattern and shader nodes.
-   [resources](resources) - Resources for rendering MaterialX content, including example materials, images, and geometry.

### Additional Resources

-   The [Developer Guide](http://www.materialx.org/docs/api/index.html) contains more detailed documentation and code examples in C++ and Python.
