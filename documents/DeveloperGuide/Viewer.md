# MaterialX Viewer

The MaterialX Viewer leverages shader generation to build GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.  Both the standard set of MaterialX nodes and the PBR node set are supported.

### Example Images

**Figure 1:** Standard Surface Shader with procedural and uniform materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024"></p>

**Figure 2:** Standard Surface Shader with textured, color-space-managed materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="480"></p>

## Building The MaterialX Viewer
Select the `MATERIALX_BUILD_VIEWER` option to build the MaterialX Viewer.  Installation will copy the **MaterialXView** executable to a `bin/` directory within the selected install folder.

### Summary of Viewer Options

1.  **Load Mesh**: Load in geometry for viewing.  There is currently support for OBJ file loading.
2.  **Load Material**: Load in a MaterialX document containing elements to render.
3.  **Property Editor**: View or edit the properties for the current element selected.
4.  **Advanced Settings** : Loading and fidelity options.

### Geometry

Upon launching MaterialXView, a teapot model is automatically loaded.
To change this preview geometry, click `Load Mesh` and navigate to `/resources/Geometry` for a list of available models.

Currently, files using the  OBJ file format are supported. If a file contains more than one `group`, then a `Select Geometry` option is available which lists the available groups.  One group is chosen as the current *active* group. To change the *active* group, click the `Select Geometry` list to choose from a list of available groups.

Under `Advanced Settings` it is possible to have a wireframe overlay displayed for the active group by choosing `Outline Selected Geometry`.

### Materials

To experiment with different materials, click `Load Material`.  Navigate to
`resources/Materials/Examples/StandardSurface` for a selection of `.mtlx` files.

To adjust the attributes on the materials, click `Property Editor` to show or hide the material properties.  Note that this changes the generated shader inputs and not the original MaterialX document.

Multiple materials can be loaded using the following options:

1.  Load a single MaterialX document containing multiple materials, or
2.  Merge additional materials into the current session by
    1.  Clicking on `Advanced Settings` and enabling `Merge Materials`.
    2.  Clicking on `Load Material` to select additional `.mtlx` files.

If more than one material has been loaded they will be listed in a pop-up menu under an `Assigned Material` label.

To assign a renderable item to either an existing group or the entire object, click on the pop-up menu to switch among (between) the available materials.  Alternatively the `LEFT` and `RIGHT` arrows can be used to cycle through the list.

Note: Once you have loaded a material, it remains in the list even after you disable `Add Materials`.
To clear the entries, disable `Add Materials` and load in a new `.mtlx` file.  The list will be refreshed to include only the renderable items from that document.

### Looks

Assignment of materials to geometry can be performed by loading a MaterialX document which contains one or more looks (e.g. `resources\Materials\Examples\StandardSurface\test_look.mtlx`).  If the geometry string referenced by a look matches the name of a group in the current mesh, then the given material assignment will be applied to that mesh group.

### Lighting

**MaterialXView** currently provides built-in direct and indirect lighting (IBL).  You can enable one or both by selecting `Advanced Settings > Lighting Options`.

To improve the indirect lighting highlights on your material, increase the `Advanced Settings > Environment Samples`.  This may cause your system to slow down, so you may want to balance speed with quality.

### Keyboard Shortcuts

- `R`: Reload the current document from file.
- `S`: Save the current shader source to file.
- `L`: Load shader source from file.  Editing the source files before loading provides a way to debug and experiment with shader source code.
- `D`: Save each node graph in the current material as a DOT file.  See www.graphviz.org for more details on this format.
- `F`: Capture the current frame and save to file.

### Command-Line Options

The following are common command-line options for MaterialXView, and a complete list can be displayed with the `--help` option.
- `--library [FILEPATH]`: Additional library folder location
- `--path [FILEPATH]`: Additional file search path location
- `--mesh [FILENAME]`: Mesh filename
- `--material [FILENAME]`: Material filename
- `--help`: Display the complete list of command-line options
