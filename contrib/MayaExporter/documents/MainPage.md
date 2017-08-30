# MaterialX Maya Exporter {#mainpage}

The Maya Exporter is a plug-in for Maya which allows for export of 
various MaterialX elements related to looks as MaterialX data files.

## Plug-in Information

The current export options include:

- Specification of output file name (mtlx file)
- Export of the following node types
    - Displacement shaders  
    - Surface shaders
    - Lights
- Export of looks
    - light assignments
    - material assignment
- Export of selected objects as opposed to all objects

- Export of node definitions for exported node types
- Export of the MaterialX standard library

The plug-in distribution contains the following in a folder called 'MayaExporter' 

- MaterialXExport.mod: Maya module file
- data: MaterialX support data files. 
    - maya_translation.mtlx: Refer to the documentation found in this file
    to configure the translation of nodes. There are some configurations
    for some Arnold nodes which will be used if the mtoa plug-in is loaded.
    - mx_stdlib.mtlx: MaterialX standard library definition file
- documents : Support documents including plug-in code HTML documentation
- plug-ins : Maya plug-in libraries
- scripts: Maya support scripts including scripts for file export.

### Usage

To have the plug-in show up in Maya's plug-in path make sure to add the location of 'MayaExporter' to your 
'MAYA_MODULE_PATH'. 

Just loading in the plug-in will result in errors trying to find dependent files.

There are two interfaces which can be used to export:
    - Via the standard Maya export menus (Export All or Export Selected), or 
    - Directly via the 'mxExport' MEL command.

```shell
    Synopsis: mxExport [flags]

    Flags:
      -ds -displacementShaders  on|off
       -f -file                 String
      -li -lights               on|off
     -lia -lightAssignments     on|off
     -lis -lightShaders         on|off
      -ma -materialAssignments  on|off
      -nd -nodeDefinitions      on|off
     -sel -selection           
      -ss -surfaceShaders       on|off
     -std -includeStdLib        on|off
```
## Developer Information

- Download the latest version of the [CMake](https://cmake.org) build system.
- Point CMake to the root of the MaterialX library and generate C++ projects for your platform and compiler.
- The following CMake options are required:
    - MATERIALX_BUILD_MAYA_EXPORTER = 'True' to cause the plug-in to build 
    - MATERIALX_DIR = 'path to MaterialX SDK'
    - MAYA_RELEASE_DIR = 'path to installed version of Maya'. 

### Supported Configurations

- The MaterialX codebase requires a compiler with support for C++11, and can be built with any of the following:
    - Microsoft Visual Studio 2015 or newer
    - GCC 4.8 or newer

- The Maya version is assumed to be Maya 2018 or newer.
- The Arnold version is assumed to be version 5 or newer.

### Building API Documentation

To generate HTML documentation for the exporter classes, make sure 
a version of [Doxygen]'https://www.doxygen.org/' is on your path.

Documentation will then be generated during the normal build process for 
the plug-in.
