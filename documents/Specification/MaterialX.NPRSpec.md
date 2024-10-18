<!-----
MaterialX NPR Shading Nodes v1.39
----->


# MaterialX NPR Shading Nodes

**Version 1.39**  
Doug Smythe - Industrial Light & Magic  
Jonathan Stone - Lucasfilm Advanced Development Group  
July 1, 2024

# Introduction

The MaterialX Specification and MaterialX Physically Based Shading Nodes documents describe a number of standard pattern and shading nodes that may be used to construct nodegraph-based shaders for physically based rendering in a variety of applications.  However, there are certain operations that are desirable in non-photorealistic shading styles but which cannot be implemented within certain rendering constructs.  It is also helpful conceptually to separate nodes primarily useful for photorealistic and non-photorealistic shading styles into separate libraries.

This document describes a number of MaterialX nodes primarily applicable to non-photorealistic, or NPR, rendering.  Rendering applications whose architecture cannot support these operations are not required to support these nodes.


## Table of Contents

**[MaterialX NPR Library](#materialx-npr-library)**  
 [NPR Application Nodes](#npr-application-nodes)  
 [NPR Utility Nodes](#npr-utility-nodes)  
 [NPR Shading Nodes](#npr-shading-nodes)  

**[References](#references)**


# MaterialX NPR Library


## NPR Application Nodes

<a id="node-viewdirection"> </a>

* **`viewdirection`**: the current scene view direction (e.g. from the viewing/camera position to the current shading position).  If `viewdirection` is used in a PBR shading context, it should be noted that this would be the same as the incident ray direction for primary ("camera") rays but **not** for secondary/reflection rays.  This node must be of type vector3.

    * `space` (uniform string):  the space in which to return the view vector direction, defaults to `world`. 



## NPR Utility Nodes

<a id="node-facingratio"> </a>

* **`facingratio`**: returns the geometric facing ratio, computed as the dot product between the view direction and geometric normal.  Output is a float between 0.0 and 1.0.

    * `viewdirection` (vector3): the viewing direction, defaults to the value of the "Vworld" (world space view direction) geometric property.
    * `normal` (vector3): the surface normal vector, defaults to the value of the "Nworld" (world space view direction) geometric property.  This vector is expected to be prenormalized to length 1.0.
    * `faceforward` (boolean): description needed; default is false.
    * `invert` (boolean): description needed; default is false.



## NPR Shading Nodes

<a id="node-gooch-shade"> </a>

* **`gooch_shade`**: Computes the single-pass shading portion of the Gooch[^Gooch1998] lighting model.  Output type `surfaceshader`.
    * `warm_color` (color3): the "warm" color for shading, defaults to (0.8, 0.8, 0.7) in the `lin_rec709` colorspace.
    * `cool_color` (color3): the "cool" color for shading, defaults to (0.3, 0.3, 0.8) in the `lin_rec709` colorspace.
    * `specular_intensity` (float): the intensity of the specular component. Defaults to 1.0.
    * `shininess` (float): the specular power typically ranging from 1 to 256, defaults to 64.
    * `light_direction` (vector3): the incoming predominant lighting direction in world space, defaults to (1.0, -0.5, -0.5).



# References

[^Gooch1998]: Gooch et al., **A Non-Photorealistic Lighting Model For Automatic Technical Illustration**, <https://users.cs.northwestern.edu/~ago820/SIG98/gooch98.pdf>, 1998.
