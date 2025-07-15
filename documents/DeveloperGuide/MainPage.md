# MaterialX Overview

MaterialX is an open standard for representing rich material and look-development content in computer graphics, enabling its platform-independent description and exchange across applications and renderers.  Launched at [Industrial Light & Magic](https://www.ilm.com/) in 2012, MaterialX has been a key technology in their feature films and real-time experiences since _Star Wars: The Force Awakens_ and _Millennium Falcon: Smugglers Run_.  The project was released as open source in 2017, with companies including Sony Pictures Imageworks, Pixar, Autodesk, Adobe, and SideFX contributing to its ongoing development.  In 2021, MaterialX became the seventh hosted project of the [Academy Software Foundation](https://www.aswf.io/).

## Quick Start for Developers

- Download the latest version of the [CMake](https://cmake.org/) build system.
- Obtain the MaterialX source code either by downloading it from the [Releases](https://github.com/AcademySoftwareFoundation/MaterialX/releases) page or by cloning the repository, as outlined in our [Development Workflow](../../CONTRIBUTING.md#development-workflow).
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- Select the `MATERIALX_BUILD_PYTHON` option to build Python bindings.
- Select the `MATERIALX_BUILD_VIEWER` option to build the [MaterialX Viewer](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/DeveloperGuide/Viewer.md).
- Select the `MATERIALX_BUILD_GRAPH_EDITOR` option to build the [MaterialX Graph Editor](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/DeveloperGuide/GraphEditor.md). 

## Supported Platforms

The MaterialX codebase requires a compiler with support for C++17, and can be built with any of the following:

- Microsoft Visual Studio 2017 or newer
- GCC 8 or newer
- Clang 5 or newer

Make sure to use the appropriate generator for your chosen compiler. For example:

- The `MinGW Makefiles` generator is typically used with **GCC**.
- The `Ninja` generator is commonly used with **Clang**.
- On **Windows**, Visual Studio includes the MSVC toolchain, which serves as both generator and compiler, eliminating the need to to install GCC, Clang, or Ninja separately.

The Python bindings for MaterialX are based on [PyBind11](https://github.com/pybind/pybind11), and support Python versions 3.9 and greater. For details, see [Building MaterialX Python](#building-materialx-python).

On macOS, you'll need to [install Xcode](https://developer.apple.com/xcode/resources/), in order to get access to the Metal Tools as well as compiler toolchains.

## Build Methods

You can build MaterialX using any of the following methods:

1. [CMake GUI](#cmake-gui)
2. [CMake Command-Line Interface (CLI)](#cmake-cli)
3. [Using an IDE](#use-an-ide)

There is no recommended method; it’s purely based on personal preference.

### CMake GUI
You can use the CMake GUI to configure and generate project files for MaterialX. Note that CMake GUI only generates build files—the actual build must be performed in an external tool such as an IDE or the terminal.

To get started, open CMake GUI and go through these steps:

1. **Browse Source**: Select the root of the cloned MaterialX repository.
2. **Browse Build**: Choose a build directory (e.g. `MaterialX/build`).
3. *If needed,* modify the environment variables displayed in the UI based on your convenience.
4. Click **Configure**: Click Configure: This saves your current configuration. You'll need to click it again anytime you change any options. The first time you click it, CMake will prompt you to choose a generator (e.g., Visual Studio, Ninja).
5. Click **Generate**: This step creates the build system files (e.g., a Visual Studio solution or Makefiles).

    You’ll see progress messages in the output window confirming each step. For example:
    ```
    Configuring done (3.9s)
    Generating done (1.0s)
    ```

6. Build the project:
   - Click **Open Project** to launch the generated project in your default IDE or the configured environment to open your project, then build from there.
   - **Or,** build directly from the terminal:
        ```bash
        cd <your-build-directory>
        cmake --build .
        ```

### CMake CLI

The CMake Command-Line Interface (CLI) offers flexible ways to configure and build the project.
You can either pass build options directly using the `-D` flag or use [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) (available in CMake 3.19+) for a more structured setup.

Basic usage examples are provided below. For more advanced configurations, see the [YAML build actions](../../.github/workflows/main.yml) in the repository.

**Basic CLI Examples**
- Using **MinGW Makefiles** with the **GCC** compiler:
    ```bash
    cd MaterialX
    cmake -S . -B build -DMATERIALX_BUILD_VIEWER=ON -DMATERIALX_BUILD_GRAPH_EDITOR=ON
    cmake --build ./build
    ```

- Using **Ninja** with the **Clang** compiler:
    ```bash
    cmake -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B build -DMATERIALX_BUILD_VIEWER=ON -DMATERIALX_BUILD_GRAPH_EDITOR=ON
    cmake --build build
    ```

**Basic CMake Presets Example**
- Define build options in a simple JSON file (`CMakePresets.json`) at the project root.
 
    <details><summary>Example: <code>CMakePresets.json</code></summary>

    ```json
    {
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 23,
        "patch": 0
    },
    "configurePresets": [
        {
        "name": "default",
        "generator": "Visual Studio 17 2022",
        "description": "Default build configuration",
        "hidden": false,
        "binaryDir": "build",
        "cacheVariables": {
            "MATERIALX_BUILD_VIEWER": "ON",
            "MATERIALX_BUILD_GRAPH_EDITOR": "ON"
        }
        }
    ]
    }
    ```
    </details>

    To configure and build using the preset:

    ```bash
    cd MaterialX
    cmake --preset default
    cmake --build build
    ```


### Use an IDE

MaterialX is compatible with any IDE that supports CMake. Below are some common IDEs for MaterialX development:

#### CLion

[CLion](https://www.jetbrains.com/clion/) is a cross-platform IDE that fully supports CMake. It is free for non-commercial use and provides granular settings to manage CMake builds effectively.

To get started, simply open the MaterialX repository in CLion, and it will auto-load the CMake project.

You can configure CMake settings and create custom build profiles as follows:

1. Open **CMake settings** via
   **⚙️ Gear Icon (top right) → `Settings...` → `Build, Execution, Deployment` → `CMake`**
2. Under the **Profiles** section, select an existing profile or create a new one.
3. In the **CMake Options / Cache Variables** field, define your build options.
    For example: Enable `MATERIALX_BUILD_VIEWER` and `MATERIALX_BUILD_GRAPH_EDITOR`.

To build the project:

- Use `Build → Build Project`.
- To install, use `Build → Install`.


## Build Options

| **Option**                      | **Description**                                                              |
| ------------------------------- | ---------------------------------------------------------------------------- |
| `MATERIALX_BUILD_PYTHON`        | Builds Python bindings. Output is located in `/bin` within the build folder. |
| `MATERIALX_BUILD_VIEWER`        | Builds the MaterialX Viewer. Output is located in `/bin`.                    |
| `MATERIALX_BUILD_GRAPH_EDITOR`  | Builds the MaterialX Graph Editor. Output is located in `/bin`.              |
| `MATERIALX_BUILD_OIIO`          | Builds MaterialXRender with OpenImageIO support.                             |
| `MATERIALX_BUILD_OCIO`          | Builds MaterialXGenShader with OpenColorIO color spaces and transforms.      |
| `MATERIALX_PYTHON_VERSION`      | Specifies the Python version to use for building the Python package.         |
| `MATERIALX_PYTHON_EXECUTABLE`   | Defines the Python executable for building the MaterialX Python package.     |
| `MATERIALX_PYTHON_PYBIND11_DIR` | Path to the PyBind11 source for custom Python builds.                        |
| `MATERIALX_BUILD_DOCS`          | Builds the API documentation.                                                |
| `CMAKE_INSTALL_PREFIX`          | Specifies the install directory for MaterialX C++ and Python libraries.      |
| `MATERIALX_INSTALL_PYTHON`      | Determines whether to install MaterialX Python as a third-party library.     |



## Building MaterialX

### Building MaterialX C++

The MaterialX C++ libraries are automatically included when building MaterialX through CMake.

To enable OpenImageIO and OpenColorIO support in MaterialX builds, the following additional options may be used:

- `MATERIALX_BUILD_OIIO`: Requests that MaterialXRender be built with OpenImageIO in addition to stb_image, extending the set of supported image formats.  The minimum supported version of OpenImageIO is 2.2.
- `MATERIALX_BUILD_OCIO`: Requests that MaterialXGenShader be built with support for custom OpenColorIO color spaces and transforms.  The minimum supported version of OpenColorIO is 2.4.

See the [MaterialX Unit Tests](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/source/MaterialXTest) page for documentation on shader generation and render testing in GLSL, OSL, and MDL.

### Building MaterialX Python

By default, the `MATERIALX_BUILD_PYTHON` option will use the active version of Python in the developer's path.  To select a specific version of Python, use one or more of the following advanced options:

- `MATERIALX_PYTHON_VERSION`: Python version to be used in building the MaterialX Python package (e.g. `3.9`)
- `MATERIALX_PYTHON_EXECUTABLE`: Python executable to be used in building the MaterialX Python package (e.g. `C:/Python39/python.exe`)

Additional options for the generation of MaterialX Python include the following:

- `MATERIALX_PYTHON_PYBIND11_DIR`: Path to a folder containing the PyBind11 source to be used in building MaterialX Python. Defaults to the included PyBind11 source.

### Building The MaterialX Viewer

Select the `MATERIALX_BUILD_VIEWER` option to build the MaterialX Viewer.  Installation will copy the `MaterialXView` executable to a `bin/` directory within the selected install folder.

### Building API Documentation

To generate HTML documentation for the MaterialX C++ API, make sure a version of [Doxygen](https://www.doxygen.org/) is on your path, and select the advanced option `MATERIALX_BUILD_DOCS` in CMake.  This option will add a target named `MaterialXDocs` to your project, which can be built as an independent step from your development environment.

## Installing MaterialX

Building the `install` target of your project will install the MaterialX C++ and Python libraries to the folder specified by the `CMAKE_INSTALL_PREFIX` setting, and will install MaterialX Python as a third-party library in your Python environment.  Installation of MaterialX Python as a third-party library can be disabled by setting `MATERIALX_INSTALL_PYTHON` to `OFF`.

## MaterialX Versioning

The MaterialX codebase uses a modified semantic versioning system where the *major* and *minor* versions match that of the corresponding [MaterialX Specification](https://materialx.org/Specification.html), and the *build* version represents engineering advances within that specification version.  MaterialX documents are similarly marked with the specification version they were authored in, and they are valid to load into any MaterialX codebase with an equal or higher specification version.

Upgrading of MaterialX documents from earlier versions is handled at import time by the `Document::upgradeVersion()` method, which applies the syntax and node interface upgrades that have occurred in previous specification revisions.  This allows the syntax conventions of MaterialX and the names and interfaces of nodes to evolve over time, without invalidating documents from earlier versions.

### MaterialX API Changes

The following rules describe the categories of changes to the [MaterialX API](https://materialx.org/docs/api/classes.html) that are allowed in version upgrades:

- In *build* version upgrades, only non-breaking changes to the MaterialX API are allowed.  For any API call that is modified in a build version upgrade, backwards compatibility should be maintained using deprecated C++ and Python wrappers for the original API call.
- In *minor* and *major* version upgrades, breaking changes to the MaterialX API are allowed, though their benefit should be carefully weighed against their cost.  Any breaking changes to API calls should be highlighted in the release notes for the new version.

### MaterialX Data Library Changes

The following rules describe the categories of changes to the [MaterialX Data Libraries](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries) that are allowed in version upgrades:

- In *build* version upgrades, only additive changes and fixes to the MaterialX data libraries are allowed.  Additive changes are allowed to introduce new nodes, node versions, and node inputs with backwards-compatible default values.  Data library fixes are allowed to update a node implementation to improve its alignment with the specification, without making any changes to its name or interface.
- In *minor* version upgrades, changes to the names and interfaces of MaterialX nodes are allowed, with the requirement that version upgrade logic be used to maintain the validity and visual interpretation of documents from earlier versions.
- In *major* version upgrades, changes to the syntax rules of MaterialX documents are allowed, with the requirement that version upgrade logic be used to maintain the validity and visual interpretation of documents from earlier versions.  These changes usually require synchronized updates to both the MaterialX API and data libraries.

## Additional Links

- The main [MaterialX website](http://www.materialx.org) provides background on the project's history, industry collaborations, and recent presentations.
- The [Python Scripts](https://github.com/materialx/MaterialX/tree/main/python/Scripts) folder contains standalone examples of MaterialX Python code.
- The [MaterialX Unit Tests](https://github.com/materialx/MaterialX/tree/main/source/MaterialXTest) folder contains examples of useful patterns for MaterialX C++.
- The [MaterialX Viewer](https://github.com/materialx/MaterialX/blob/main/documents/DeveloperGuide/Viewer.md) is a complete, cross-platform C++ application based upon [MaterialX Shader Generation](https://github.com/materialx/MaterialX/blob/main/documents/DeveloperGuide/ShaderGeneration.md)
