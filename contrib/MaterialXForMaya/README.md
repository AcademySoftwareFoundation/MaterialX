# MaterialXForMaya

Sample MaterialX exporter for Maya

The project distribution contains the following:
- **data** (support data files)
- **source** (source code)
- **tools** (various tools and scripts)

# Building

The project has succesfully been build on Windows. Cmake is used to generate build files. 
Follow the steps below to generate the build files. "yourpath" should be replaced with the path to the
root directory of the project.

1. Download or build the MaterialX SDK. 
	- If building locally, build against at least the 3.5.2 tag in the MaterialX repo.

2. Install or download a cut of Maya. Maya 2018 is needed for Arnold 5.
	- If building Arnold locally use the 'develop' branch for now.

3. Run Cmake GUI and set the following options:
	- CMAKE_INSTALL_PREFIX = yourpath/deploy
	- MATERIALX_DIR = path to MaterialX SDK
	- MAYA_DEBUG_DIR = path to debug build of Maya (as needed)
	- MAYA_RELEASE_DIR = path to release build of Maya (as needed)

4. Press Configure and then Generate. Build files should be generated and placed in <yourpath>/build

# Running

To have the plug-in show up in Maya's plug-in path make sure to add 'CMAKE_INSTALL_PREFIX'/contrib/MaterialXForMaya to your MAYA_MODULE_PATH.
Just loading in the plug-in will result in errors trying to find dependent files.

There are two interfaces which can be used to export. Via the standard Maya export menus (Export All or Export Selected), or directly via the 'mxExport' MEL command.
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
