# MaterialXView Module

This module contains the source for a sample **MaterialX Viewer**, which leverages shader generation to generate GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.  Both the standard set of MaterialX nodes and the PBR node set are supported.

## User Interaction
To open MaterialXView, open the **MaterialXView** executable found in the `bin/` directory.

### Summary of Viewer Options

 1. **Load Mesh**: Load in geometry for viewing. There is currently support for OBJ file loading.
 2. **Load Material**: Load in a MaterialX document containing elements to render.
 3. **Property Editor**: View or edit the properties for the current  element selected.
 4. **Advanced Settings** : Load and fidelity options.

### 1. Geometry
Upon launching MaterialXView, a teapot model is automatically loaded.
To change this preview geometry, click `Load Mesh` and navigate to `/resources/Geometry` for a list of available models.

Currently files using the  OBJ file format are  supported. If a file contains more than `group`, then a `Select Geometry` option is available which lists the available groups. One group is chosen as the current *active* group. To change the *active* group click on the associated geometry group label to choose from a list of available groups.

Under `Advanced Settings` it is possible to have a wireframe overlay displayed for the active group by choosing `Outline Selected Geometry`.

### 2. Materials
To experiment with different materials, click `Load Material`. Navigate to
`resources/Materials/TestSuite/pbrlib/material` for a selection of `.mtlx` files.

To adjust the attributes on the materials, click `Property Editor` to show or hide the material properties. Note that this changes the generated shader inputs and not the original MaterialX document.

To load in more than one material:

 1. Either have more than one renderable item specified in the input MaterialX document (.mtlx) or
 2. Incrementally load in more materials by:
    1. Clicking on `Advanced Setting`s and enable `Merge Materials`.
    2. Clicking on `Load Material` to select additional `.mtlx` files. The materials are listed under a `Assigned Material` option.

To assign a renderable item to either an existing group or the entire object, click the `Assigned Material` list to switch among (between) the currently loaded materials.
Alternatively the `LEFT` and `RIGHT` arrows can be used to cycle through the list.

Note: Once you have loaded a material, it remains in the list even after you disable `Merge Materials`. After disabling this option the next load will reset the list to the
renderable elements found in that document.

**Figure 1**. Snapshot showing the options for selecting *active* group and for that group the assigned material. In this case a group called `stand` has been assigned the material `M_jade`.
A wireframe overlay is applied to the `stand` geometry in the viewer.

<img src="/documents/Images/MaterialXView_Materials_And_Geomtery_Groups.png" width="480">

### 3. Lighting

**MaterialXView** currently provides built-in direct and indirect lighting (IBL). You can enable one or both by selecting `Advanced Settings > Lighting Options`.

To improve the highlights on your material, increase the `Advanced Settings > Environment Samples`. This may cause your system to slow down, so you may want to balance speed with quality.

### Utilites

 1. The GLSL source code for the currently selected material can be saved to disk by pressing the `S` key while focus is in the viewer.
 2. This source code can be reloaded back in by pressing the `L` key while focus is in the viewer. This is useful to make quick adjustments on the source code for debugging purposes. The original MaterialX document is not affected.
 3. The current frame can be saved to disk by pressing the `F` key while focus is in the viewer.

### Example Images

**Standard Surface Shader with procedural and uniform materials**
<img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024">

**Standard Surface Shader with textured, color-space-managed materials**
<img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="480">
