<!-----
MaterialX NPR Shading Nodes v1.39
----->


# MaterialX NPR Shading Nodes

**Version 1.39**  
Doug Smythe - Industrial Light & Magic  
Jonathan Stone - Lucasfilm Advanced Development Group  
July 1, 2024

# Introduction

The [MaterialX Standard Nodes](./MaterialX.StandardNodes.md) and [MaterialX Physically Based Shading Nodes](./MaterialX.PBRSpec.md) documents describe a number of standard pattern and shading nodes that may be used to construct nodegraph-based shaders for physically based rendering in a variety of applications.  However, there are certain operations that are desirable in non-photorealistic shading styles but which cannot be implemented within certain rendering constructs.  It is also helpful conceptually to separate nodes primarily useful for photorealistic and non-photorealistic shading styles into separate libraries.

This document describes a number of MaterialX nodes primarily applicable to non-photorealistic, or NPR, rendering.  Rendering applications whose architecture cannot support these operations are not required to support these nodes.


## Table of Contents

**[MaterialX NPR Library](#materialx-npr-library)**  
 [NPR Application Nodes](#npr-application-nodes)  
 [NPR Utility Nodes](#npr-utility-nodes)  
 [NPR Shading Nodes](#npr-shading-nodes)  

**[References](#references)**

<br>


# MaterialX NPR Library


## NPR Application Nodes

<a id="node-viewdirection"> </a>

### `viewdirection`
The current scene view direction, as defined by the shading environment.

The view direction is a normalized vector from the viewer position to the current shading position. In a PBR shading context, it represents the incident direction for primary camera rays, independent of any secondary or reflection rays.

|Port   |Description                           |Type   |Default|Accepted Values     |
|-------|--------------------------------------|-------|-------|--------------------|
|`space`|The space of the view direction vector|string |world  |model, object, world|
|`out`  |Output: view direction                |vector3|       |                    |



## NPR Utility Nodes

<a id="node-facingratio"> </a>

### `facingratio`
The geometric facing ratio of the view direction and surface normal.

Facing ratio is computed as the dot product between the view direction and surface normal.

|Port           |Description                                                               |Type   |Default |
|---------------|--------------------------------------------------------------------------|-------|--------|
|`viewdirection`|The input view direction vector                                           |vector3|_Vworld_|
|`normal`       |The input surface normal vector                                           |vector3|_Nworld_|
|`faceforward`  |Makes the output always positive, facing towards the view direction       |boolean|true    |
|`invert`       |Inverts the output values by multiplying them by -1                       |boolean|false   |
|`out`          |Output: the float representing the ratio between view direction and normal|float  |        |




## NPR Shading Nodes

<a id="node-gooch-shade"> </a>

### `gooch_shade`
Computes the color from single-pass shading portion of the Gooch[^Gooch1998] lighting model.

Gooch shade provides an illustrative shading effect by blending colors based on the angle between the surface normal and the light direction. It also provides a simple Phong specular highlight, on top of the warm and cool colors.

|Port                |Description                            |Type   |Default      |
|--------------------|---------------------------------------|-------|-------------|
|`warm_color`        |The color facing toward the light      |color3 |0.8, 0.8, 0.7|
|`cool_color`        |The color facing away from the light   |color3 |0.3, 0.3, 0.8|
|`specular_intensity`|The intensity of the highlight         |float  |1            |
|`shininess`         |The size of the highlight              |float  |64           |
|`light_direction`   |The world-space direction of the light |vector3|1, -0.5, -0.5|
|`out`               |Output: the Gooch lighting model result|color3 |             |

<br>


# References

[^Gooch1998]: Gooch et al., **A Non-Photorealistic Lighting Model For Automatic Technical Illustration**, <https://users.cs.northwestern.edu/~ago820/SIG98/gooch98.pdf>, 1998.
