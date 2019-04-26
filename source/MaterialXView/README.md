# MaterialXView Module

This module contains the source for a sample **MaterialX Viewer**, which leverages shader generation to generate GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.  Both the standard set of MaterialX nodes and the PBR node set are supported.

## User Interaction
To open MaterialXView, open the **MaterialXView** executable found in the `bin/` directory.

### Summary of Viewer Options

1.  **Load Mesh**: Load in geometry for viewing. There is currently support for OBJ file loading.
2.  **Load Material**: Load in a MaterialX document containing elements to render.
3.  **Property Editor**: View or edit the properties for the current  element selected.
4.  **Advanced Settings** : Load and fidelity options.

### 1. Geometry
Upon launching MaterialXView, a teapot model is automatically loaded.
To change this preview geometry, click `Load Mesh` and navigate to `/resources/Geometry` for a list of available models.

Currently, files using the  OBJ file format are supported. If a file contains more than one `group`, then a `Select Geometry` option is available which lists the available groups. One group is chosen as the current *active* group. To change the *active* group, click the `Select Geometry` list to choose from a list of available groups.

Under `Advanced Settings` it is possible to have a wireframe overlay displayed for the active group by choosing `Outline Selected Geometry`.

### 2. Materials
To experiment with different materials, click `Load Material`. Navigate to
`resources/Materials/TestSuite/pbrlib/material` for a selection of `.mtlx` files.

To adjust the attributes on the materials, click `Property Editor` to show or hide the material properties. Note that this changes the generated shader inputs and not the original MaterialX document.

Multiple materials can be loaded using the following options:

1.  Have more than one renderable item specified in the input MaterialX document (.mtlx).
2.  Loading in additional renderable item in an input MaterialX document (.mtlx) by
    1.  Clicking on `Advanced Setting`s and enable `Add Materials`.
    2.  Clicking on `Load Material` to select additional `.mtlx` files.

If more than one material has been loaded they will be listed in a pop-up menu under an `Assigned Material` label.

To assign a renderable item to either an existing group or the entire object, click on the pop-up menu to switch among (between) the available materials. Alternatively the `LEFT` and `RIGHT` arrows can be used to cycle through the list.

Note: Once you have loaded a material, it remains in the list even after you disable `Add Materials`.
To clear the entries, disable `Add Materials` and load in a new `.mtlx` file. The list will be refreshed to include only the renderable items from that document.

**Figure 1**: The following image shows the options for selecting an active group and assigning a material for a group. In this example, a group called `stand` has been assigned the material `M_jade`.
A wireframe overlay is applied to the `stand` geometry in the viewer.
<p><img src="/documents/Images/MaterialXView_Materials_And_Geomtery_Groups.png" width="320"></p>

### 3. Looks

The binding between materials and geometry can be performed by loading in a MaterialX document which contains a look if the 'Assign Looks' option is enabled. If the referenced material and assigned geometry exist then the material will be assigned.

**Figure 2**: The follow shows a example look which has been applied to a group of geometry.
<p><img src="/documents/Images/MaterialXView_AssignedLook.png" width="320"></p>

### 4. Lighting

**MaterialXView** currently provides built-in direct and indirect lighting (IBL). You can enable one or both by selecting `Advanced Settings > Lighting Options`.

To improve the indirect lighting highlights on your material, increase the `Advanced Settings > Environment Samples`. This may cause your system to slow down, so you may want to balance speed with quality.

**Figure 3**: Render with  indirect lighting enabled on the left, and with the addition of direct lighting on the right.
<p><img src="/documents/Images/MaterialXView_DirectLighting_Compare.png" width="480"></p>

### Keyboard Shortcuts

1.  The GLSL source code for the currently selected material can be saved to disk by pressing the `S` key while focus is in the viewer.
2.  This source code can be reloaded back in by pressing the `L` key while focus is in the viewer. This is useful to make quick adjustments on the source code for debugging purposes. The original MaterialX document is not affected.
3.  The current frame can be saved to disk by pressing the `F` key while focus is in the viewer.
4.  A DOT graph can be saved to disk by pressing the `D` key while focus is in the viewer.

### Command Line Options

Command line arguments can be specified by using the `--` prefix on startup:
1. `--library`: Additional library folder location(s).
2. `--path`: Additional file search path location(s).
3. `--mesh`: Mesh file name.
4. `--material`: Material file name.
5. `--envMethod`: Environment lighting method. 1 means to use prefiltered environment maps. The default is to use filtered importance sampling.
6. `--msaa`: Multi sampling count for anti-aliasing.

### Example Images

**Figure 4:** Standard Surface Shader with procedural and uniform materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024"></p>

**Figure 5:** Standard Surface Shader with textured, color-space-managed materials
<p><img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="480"></p>
