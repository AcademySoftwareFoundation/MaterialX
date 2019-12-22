# MaterialX Viewer

The MaterialX Viewer leverages shader generation to build GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.  The standard set of pattern and physically-based shading nodes is supported, and libraries of custom nodes can be included as additional library paths.

### Example Images

**Figure 1:** Standard Surface Shader with procedural and uniform materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024"></p>

**Figure 2:** Standard Surface Shader with textured, color-space-managed materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="640"></p>

## Building The MaterialX Viewer
Select the `MATERIALX_BUILD_VIEWER` option in CMake to build the MaterialX Viewer.  Installation will copy the **MaterialXView** executable to a `/bin` directory within the selected install folder.

### Summary of Viewer Options

1.  **Load Mesh**: Load a new geometry in the OBJ format.
2.  **Load Material**: Load a material document in the MTLX format.
3.  **Load Environment**: Load a lat-long environment light in the HDR format.
4.  **Property Editor**: View or edit properties of the current material.
5.  **Advanced Settings** : Asset and rendering options.

### Geometry

The default display geometry for the MaterialX viewer is the Arnold Shader Ball, which was contributed to the MaterialX project by the Solid Angle team at Autodesk.  To change the display geometry, click `Load Mesh` and navigate to the [Geometry](../../resources/Geometry) folder for additional models in the OBJ format.

If a loaded geometry contains more than one geometric group, then a `Select Geometry` drop-down box will appear, allowing the user to select which group is active.  The active geometric group will be used for subsequent actions such as material assignment and rendering property changes.

### Materials

To change the displayed material, click `Load Material` and navigate to the [Materials/Examples/StandardSurface](../../resources/Materials/Examples/StandardSurface) or [Materials/Examples/UsdPreviewSurface](../../resources/Materials/Examples/UsdPreviewSurface) folders, which contain a selection of example materials in the MTLX format.

Once a material is loaded into the viewer, its parameters may be inspected and adjusted by clicking the `Property Editor` and scrolling through the list of parameters.  An edited material may be saved to the file system by clicking `Save Material`.  Clicking on `Advanced Settings` and enabling `Show Advanced Properties` will extend the set of material properties that are displayed in the editor.

Multiple material documents can be combined in a single session by navigating to `Advanced Settings` and enabling `Merge Materials`.  Loading new materials with this setting enabled will add them to the current material list, where they can be assigned to geometry via the `Assigned Material` drop-down box.  Alternatively the `LEFT` and `RIGHT` arrows can be used to cycle through the list of available materials.

If a material document containing `look` elements is loaded into the viewer, then any material assignments within the look will be applied to geometric groups that match the specified geometry strings.  See [standard_surface_look_brass_tiled.mtlx](../../resources/Materials/Examples/StandardSurface/standard_surface_look_brass_tiled.mtlx) for an example of a material document containing look elements.

### Lighting

The default lighting environment for the viewer is the San Giuseppe Bridge environment from HDRI Haven.  To load another environment into the viewer, click `Load Environment` and navigate to the [Environments](../../resources/Images/Environments) folder, or load any HDR environment in the latitude-longitude format.  If the HDR file on disk has a companion image with the `_diffuse` suffix, then this file will be loaded as the diffuse convolution of the environment; otherwise, a diffuse convolution will be generated at load-time using spherical harmonics.

The fidelity of environment lighting can be improved by increasing the `Environment Samples` option under `Advanced Settings`, though this improved quality requires additional GPU resources and can affect the interactivity of the viewer.

Additional direct lights may be enabled through the `Lighting Options` section of `Advanced Settings`, though these lights are not yet editable in the viewer at run-time.

### Images

By default, the MaterialX viewer loads and saves image files using `stb_image`, which supports commmon 8-bit formats such as JPEG, PNG, TGA, and BMP, as well as the HDR format for high-dynamic-range images.  If you need access to additional image formats such as EXR and TIFF, then the MaterialX viewer can be built with support for `OpenImageIO`.  To build MaterialX with OpenImageIO, check the `MATERIALX_BUILD_OIIO` option in CMake, and specify the location of your OpenImageIO installation with the `MATERIALX_OIIO_DIR` option.

### Keyboard Shortcuts

- `R`: Reload the current material from file.  Hold `SHIFT` to reload all standard libraries as well.
- `S`: Save the current shader source to file.
- `L`: Load shader source from file.  Editing the source files before loading provides a way to debug and experiment with shader source code.
- `D`: Save each node graph in the current material as a DOT file.  See www.graphviz.org for more details on this format.
- `F`: Capture the current frame and save to file.

### Command-Line Options

The following are common command-line options for MaterialXView, and a complete list can be displayed with the `--help` option.
- `--material [FILENAME]`: Specify the displayed material
- `--mesh [FILENAME]`: Specify the displayed geometry
- `--library [FILEPATH]`: Specify an additional library folder
- `--path [FILEPATH]`: Specify an additional search-path folder
- `--help`: Display the complete list of command-line options
