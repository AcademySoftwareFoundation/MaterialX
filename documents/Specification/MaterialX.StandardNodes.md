<!-----
MaterialX Standard Nodes v1.39
----->


# MaterialX Standard Nodes

**Version 1.39**  
Doug Smythe - Industrial Light & Magic  
Jonathan Stone - Lucasfilm Advanced Development Group  
March 15, 2025


# Introduction

The MaterialX Specification defines a content schema to describe materials, image processing and shading networks and how the nodes in those networks access textural and geometric information, in a platform- and shading-language-independent manner.

This document describes a specific set of **Standard Nodes** that can be used to read and process image and geometric attribute data, as well as create new image data procedurally.  These "stdlib" nodes are an essential core part of all MaterialX implementations.  Additional nodes are described in companion documents [**MaterialX Physically Based Shading Nodes**](./MaterialX.PBRSpec.md) and [**MaterialX NPR Shading Nodes**](./MaterialX.NPRSpec.md).

In the descriptions below, a node with an "(NG)" annotation indicates a node that is implemented using a nodegraph in the MaterialX distribution, while unannotated nodes are implemented natively in the various renderer shading languages.


## Table of Contents

**[Introduction](#introduction)**  

**[Standard Source Nodes](#standard-source-nodes)**  
 [Texture Nodes](#texture-nodes)  
 [Procedural Nodes](#procedural-nodes)  
 [Noise Nodes](#noise-nodes)  
 [Shape Nodes](#shape-nodes)  
 [Geometric Nodes](#geometric-nodes)  
 [Application Nodes](#application-nodes)  

**[Standard Operator Nodes](#standard-operator-nodes)**  
 [Math Nodes](#math-nodes)  
 [Logical Operator Nodes](#logical-operator-nodes)  
 [Adjustment Nodes](#adjustment-nodes)  
 [Compositing Nodes](#compositing-nodes)  
 [Conditional Nodes](#conditional-nodes)  
 [Channel Nodes](#channel-nodes)  
 [Convolution Nodes](#convolution-nodes)  

**[Standard Shader Nodes](#standard-shader-nodes)**

<br>


# Standard Source Nodes

Source nodes use external data and/or procedural functions to form an output; they do not have any required inputs.  Each source node must define its output type.

This section defines the Source Nodes that all MaterialX implementations are expected to support.  Standard Source Nodes are grouped into the following classifications: [Texture Nodes](#texture-nodes), [Procedural Nodes](#procedural-nodes), [Noise Nodes](#noise-nodes), [Shape Nodes](#shape-nodes), [Geometric Nodes](#geometric-nodes) and [Application Nodes](#application-nodes).


## Texture Nodes

Texture nodes are used to read filtered image data from image or texture map files for processing within a node graph.

```xml
  <image name="in1" type="color4">
    <input name="file" type="filename" value="layer1.tif"/>
    <input name="default" type="color4" value="0.5,0.5,0.5,1"/>
  </image>
  <image name="in2" type="color3">
    <input name="file" type="filename" value="<albedomap>"/>
    <input name="default" type="color3" value="0.18,0.18,0.18"/>
  </image>
```

Standard Texture nodes:

<a id="node-image"> </a>

* **`image`**: samples data from a single image, or from a layer within a multi-layer image.  When used in the context of rendering a geometry, the image is mapped onto the geometry based on geometry UV coordinates, with the lower-left corner of an image mapping to the (0,0) UV coordinate (or to the fractional (0,0) UV coordinate for tiled images).
The type of the &lt;image> node determines the number of channels output, which may be less than the number of channels in the image file, outputting the first N channels from the image file.  So a `float` &lt;image> would return the Red channel of an RGB image, and a `color3` &lt;image> would return the RGB channels of an RGBA image.  If the type of the &lt;image> node has more channels than the referenced image file, then the output will contain zero values in all channels beyond the N channels of the image file.
    * `file` (uniform filename): the URI of an image file.  The filename can include one or more substitutions to change the file name (including frame number) that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.
    * `layer` (uniform string): the name of the layer to extract from a multi-layer input file.  If no value for `layer` is provided and the input file has multiple layers, then the "default" layer will be used, or "rgba" if there is no "default" layer.  Note: the number of channels defined by the `type` of the `<image>` must match the number of channels in the named layer.
    * `default` (float or color<em>N</em> or vector<em>N</em>): a default value to use if the `file` reference can not be resolved (e.g. if a &lt;_geometry token_>, [_interface token_] or {_hostattr_} is included in the filename but no substitution value or default is defined, or if the resolved `file` URI cannot be read), or if the specified `layer` does not exist in the file.  The `default` value must be the same type as the `<image>` element itself.  If `default` is not defined, the default color value will be 0.0 in all channels.
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the image data is read.  Default is to use the current u,v coordinate.
    * `uaddressmode` (uniform string): determines how U coordinates outside the 0-1 range are processed before sampling the image; see below.  Default is "periodic".
    * `vaddressmode` (uniform string): determines how V coordinates outside the 0-1 range are processed before sampling the image; see below.  Default is "periodic".
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.

<a id="node-tiledimage"> </a>

* **`tiledimage`** (NG): samples data from a single image, with provisions for tiling and offsetting the image across uv space.
    * `file` (uniform filename): the URI of an image file.  The filename can include one or more substitutions to change the file name (including frame number) that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.
    * `default` (float or color<em>N</em> or vector<em>N</em>): a default value to use if the `file` reference can not be resolved (e.g. if a &lt;geomtoken>, [interfacetoken] or {hostattr} is included in the filename but no substitution value or default is defined, or if the resolved file URI cannot be read), or if the specified `layer` does not exist in the file.  The `default` value must be the same type as the `<image>` element itself.  If `default` is not defined, the default color value will be 0.0 in all channels.
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the image data is read.  Default is to use the current u,v coordinate.
    * `uvtiling` (vector2): the tiling rate for the given image along the U and V axes. Mathematically equivalent to multiplying the incoming texture coordinates by the given vector value. Default value is (1.0, 1.0).
    * `uvoffset` (vector2): the offset for the given image along the U and V axes. Mathematically equivalent to subtracting the given vector value from the incoming texture coordinates. Default value is (0.0, 0.0).
    * `realworldimagesize` (vector2): the real-world size represented by the `file` image, with unittype "distance".  A `unit` attribute may be provided to indicate the units that `realworldimagesize` is expressed in.
    * `realworldtilesize` (vector2): the real-world size of a single square 0-1 UV tile, with unittype "distance".  A `unit` attribute may be provided to indicate the units that `realworldtilesize` is expressed in.
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.

<a id="node-triplanarprojection"> </a>

* **`triplanarprojection`** (NG): samples data from three images (or layers within multi-layer images), and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.
    * `filex` (uniform filename): the URI of an image file to be projected in the direction from the +X axis back toward the origin.
    * `filey` (uniform filename): the URI of an image file to be projected in the direction from the +Y axis back toward the origin with the +X axis to the right.
    * `filez` (uniform filename): the URI of an image file to be projected in the direction from the +Z axis back toward the origin.
    * `layerx` (uniform string): the name of the layer to extract from a multi-layer input file for the x-axis projection.  If no value for `layerx` is provided and the input file has multiple layers, then the "default" layer will be used, or "rgba" if there is no "default" layer.  Note: the number of channels defined by the `type` of the `<image>` must match the number of channels in the named layer.
    * `layery` (uniform string): the name of the layer to extract from a multi-layer input file for the y-axis projection.
    * `layerz` (uniform string): the name of the layer to extract from a multi-layer input file for the z-axis projection.
    * `default` (float or color<em>N</em> or vector<em>N</em>): a default value to use if any `file<em>X</em>` reference can not be resolved (e.g. if a &lt;geomtoken>, [interfacetoken] or {hostattr} is included in the filename but no substitution value or default is defined, or if the resolved file URI cannot be read)  The `default` value must be the same type as the `<triplanarprojection>` element itself.  If `default` is not defined, the default color value will be 0.0 in all channels.
    * `position` (vector3): a spatially-varying input specifying the 3D position at which the projection is evaluated.  Default is to use the current 3D object-space coordinate.
    * `normal` (vector3): a spatially-varying input specifying the 3D normal vector used for blending.  Default is to use the current object-space surface normal.
    * `upaxis` (integer enum): which axis is considered to be "up", either 0 for X, 1 for Y, or 2 for Z.  Default is Y (1).
    * `blend` (float): a 0-1 weighting factor for blending the three axis samples using the geometric normal, with higher values giving softer blending.  Default is 1.0.
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.


<a id="addressmode-values"> </a>

The following values are supported by `uaddressmode` and `vaddressmode` inputs of [image](#node-image) nodes:

* “constant”: Texture coordinates outside the 0-1 range return the value of the node's `default` input.
* “clamp”: Texture coordinates are clamped to the 0-1 range before sampling the image.
* “periodic”: Texture coordinates outside the 0-1 range "wrap around", effectively being processed by a modulo 1 operation before sampling the image.
* "mirror": Texture coordinates outside the 0-1 range will be mirrored back into the 0-1 range, e.g. u=-0.01 will return the u=0.01 texture coordinate value, and u=1.01 will return the u=0.99 texture coordinate value.


Texture nodes using `file*` inputs also support the following inputs to handle boundary conditions for image file frame ranges for all `file*` inputs:

* `framerange` (uniform string): a string "_minframe_-_maxframe_", e.g. "10-99", to specify the range of frames that the image file is allowed to have, usually the range of image files on disk.  Default is unbounded.
* `frameoffset` (integer): a number that is added to the current frame number to get the image file frame number.  E.g. if `frameoffset` is 25, then processing frame 100 will result in reading frame 125 from the imagefile sequence.  Default is no frame offset.
* `frameendaction` (uniform string): what to do when the resolved image frame number is outside the `framerange` range:
    * "constant": Return the value of the node's `default` input (default action)
    * "clamp": Hold the minframe image for all frames before _minframe_ and hold the maxframe image for all frames after _maxframe_
    * "periodic": Frame numbers "wrap around", so after the _maxframe_ it will start again at _minframe_ (and similar before _minframe_ wrapping back around to _maxframe_)
    * "mirror": Frame numbers "mirror" or "ping-pong" at the endpoints of framerange, so a read of the frame after _maxframe_ will return the image from frame _maxframe_-1, and a read of the frame before _minframe_ will return the image from frame _minframe_+1.

Arbitrary frame number expressions and speed changes are not supported.



## Procedural Nodes

Procedural nodes are used to generate value data programmatically.

```xml
  <constant name="n8" type="color3">
    <input name="value" type="color3" value="0.8,1.0,1.3"/>
  </constant>
  <ramptb name="n9" type="float">
    <input name="valuet" type="float" value="0.9"/>
    <input name="valueb" type="float" value="0.2"/>
  </ramptb>
```

Standard Procedural nodes:

<a id="node-constant"> </a>

* **`constant`**: a constant value.
    * `value` (any non-shader-semantic type): the value to output

<a id="node-ramplr"> </a>

* **`ramplr`**: a left-to-right linear value ramp.
    * `valuel` (float or color<em>N</em> or vector<em>N</em>): the value at the left (U=0) edge
    * `valuer` (float or color<em>N</em> or vector<em>N</em>): the value at the right (U=1) edge
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the ramp interpolation is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-ramptb"> </a>

* **`ramptb`**: a top-to-bottom linear value ramp.
    * `valuet` (float or color<em>N</em> or vector<em>N</em>): the value at the top (V=1) edge
    * `valueb` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom (V=0) edge
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the ramp interpolation is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-ramp4"> </a>

* **`ramp4`** (NG): a 4-corner bilinear value ramp.
    * `valuetl` (float or color<em>N</em> or vector<em>N</em>): the value at the top-left (U0V1) corner
    * `valuetr` (float or color<em>N</em> or vector<em>N</em>): the value at the top-right (U1V1) corner
    * `valuebl` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom-left (U0V0) corner
    * `valuebr` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom-right (U1V0) corner
    * `texcoord` (vector2, optional): the name of a vector2-type node specifying the 2D texture coordinate at which the ramp interpolation is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-splitlr"> </a>

* **`splitlr`**: a left-right split matte, split at a specified U value.
    * `valuel` (float or color<em>N</em> or vector<em>N</em>): the value at the left (U=0) edge
    * `valuer` (float or color<em>N</em> or vector<em>N</em>): the value at the right (U=1) edge
    * `center` (float): a value representing the U-coordinate of the split; all pixels to the left of "center" will be `valuel`, all pixels to the right of "center" will be `valuer`.  Default is 0.5.
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the split position is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-splittb"> </a>

* **`splittb`**: a top-bottom split matte, split at a specified V value.
    * `valuet` (float or color<em>N</em> or vector<em>N</em>): the value at the top (V=1) edge
    * `valueb` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom (V=0) edge
    * `center` (float): a value representing the V-coordinate of the split; all pixels above "center" will be `valuet`, all pixels below "center" will be `valueb`.  Default is 0.5.
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the split position is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-randomfloat"> </a>

* **`randomfloat`**: Produces a stable randomized float value between 'min' and 'max', based on an 'input' signal and 'seed' value.  Uses a 2d cellnoise function to produce the output.
    * `in` (float or integer): Initial randomization seed, default is 0.
    * `min` (float): The minimum output value, default is 0.0.
    * `max` (float): The maximum output value, default is 1.0.
    * `seed` (integer): Additional randomization seed, default is 0.

<a id="node-randomcolor"> </a>

* **`randomcolor`**: Produces a randomized RGB color within a randomized hue, saturation and brightness range, based on an 'input' signal and 'seed' value.  Output type color3.
    * `in` (float or integer): Initial randomization seed, default is 0.
    * `huelow` (float): The minimum hue value, default is 0.0.
    * `huehigh` (float): The maximum hue value, default is 1.0.
    * `saturationlow` (float): The minimum saturation value, default is 0.0.
    * `saturationhigh` (float): The maximum saturation value, default is 1.0.
    * `brightnesslow` (float): The minimum brightness value, default is 0.0.
    * `brightnesshigh` (float): The maximum brightness value, default is 1.0.
    * `seed` (integer): Additional randomization seed, default is 0.


To scale or offset `rampX` or `splitX` input coordinates, use a &lt;texcoord> or similar Geometric node processed by vector2 &lt;multiply>, &lt;rotate> and/or &lt;add> nodes, and connect to the node's `texcoord` input.



## Noise Nodes

Noise nodes are used to generate value data using one of several procedural noise functions.

```xml
  <noise2d name="n9" type="float">
    <input name="pivot" type="float" value="0.5"/>
    <input name="amplitude" type="float" value="0.05"/>
  </noise2d>
```

Standard Noise nodes:

<a id="node-noise2d"> </a>

* **`noise2d`**: 2D Perlin noise in 1, 2, 3 or 4 channels.
    * `amplitude` (float or vector<em>N</em>): the center-to-peak amplitude of the noise (peak-to-peak amplitude is 2x this value).  Default is 1.0.
    * `pivot` (float): the center value of the output noise; effectively, this value is added to the result after the Perlin noise is multiplied by `amplitude`.  Default is 0.0.
    * `texcoord` (vector2): the 2D texture coordinate at which the noise is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-noise3d"> </a>

* **`noise3d`**: 3D Perlin noise in 1, 2, 3 or 4 channels.
    * `amplitude` (float or vector<em>N</em>): the center-to-peak amplitude of the noise (peak-to-peak amplitude is 2x this value).  Default is 1.0.
    * `pivot` (float): the center value of the output noise; effectively, this value is added to the result after the Perlin noise is multiplied by `amplitude`.  Default is 0.0.
    * `position` (vector3): the 3D position at which the noise is evaluated.  Default is to use the current 3D object-space coordinate.

<a id="node-fractal2d"> </a>

* **`fractal2d`**: Zero-centered 2D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 2D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.
  * `amplitude` (float or vector<em>N</em>): the center-to-peak amplitude of the noise (peak-to-peak amplitude is 2x this value).  Default is 1.0.
  * `octaves` (integer): the number of octaves of noise to be summed.  Default is 3.
  * `lacunarity` (float or vector<em>N</em>): the exponential scale between successive octaves of noise; must be an integer value if period is non-zero so the result is properly tileable.  Default is 2.0.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
  * `diminish` (float or vector<em>N</em>): the rate at which noise amplitude is diminished for each octave.  Should be between 0.0 and 1.0; default is 0.5.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
  * `texcoord` (vector2): the 2D texture coordinate at which the noise is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-fractal3d"> </a>

* **`fractal3d`**: Zero-centered 3D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 3D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.
    * `amplitude` (float or vector<em>N</em>): the center-to-peak amplitude of the noise (peak-to-peak amplitude is 2x this value).  Default is 1.0.
    * `octaves` (integer): the number of octaves of noise to be summed.  Default is 3.
    * `lacunarity` (float or vector<em>N</em>): the exponential scale between successive octaves of noise; must be an integer value if period is non-zero so the result is properly tileable.  Default is 2.0.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
    * `diminish` (float or vector<em>N</em>): the rate at which noise amplitude is diminished for each octave.  Should be between 0.0 and 1.0; default is 0.5.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
    * `position` (vector3): the 3D position at which the noise is evaluated.  Default is to use the current 3D object-space coordinate.

<a id="node-cellnoise2d"> </a>

* **`cellnoise2d`**: 2D cellular noise, 1 or 3 channels (type float or vector3).
    * `texcoord` (vector2): the 2D position at which the noise is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-cellnoise3d"> </a>

* **`cellnoise3d`**: 3D cellular noise, 1 or 3 channels (type float or vector3).
    * `position` (vector3): the 3D position at which the noise is evaluated.  Default is to use the current 3D object-space coordinate.

<a id="node-worleynoise2d"> </a>

* **`worleynoise2d`**: 2D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).
    * `jitter` (float): amount to jitter the cell center position, with smaller values creating a more regular pattern.  Default is 1.0.
    * `style` (integer): the output style, one of "distance" (distance to the cell center), or "solid" (constant value for each cell).
    * `texcoord` (vector2): the 2D position at which the noise is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-worleynoise3d"> </a>

* **`worleynoise3d`**: 3D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).
    * `jitter` (float): amount to jitter the cell center position, with smaller values creating a more regular pattern.  Default is 1.0.
    * `style` (integer): the output style, one of "distance" (distance to the cell center), or "solid" (constant value for each cell). Default is "distance".
    * `position` (vector3): the 3D position at which the noise is evaluated.  Default is to use the current 3D object-space coordinate.

<a id="node-unifiednoise2d"> </a>

* **`unifiednoise2d`** (NG): a single node supporting 2D Perlin, Cell, Worley or Fractal noise in a unified interface.
    * `type` (integer): The type of noise function to use.  One of 0 (Perlin), 1 (Cell), 2 (Worley), or 3 (Fractal); default is Perlin.
    * `texcoord` (vector2): the input 2d space. Default is the first texture coordinates.
    * `freq` (vector2): Adjusts the noise frequency, with higher values producing smaller noise shapes. Default is (1,1).
    * `offset` (vector2): Shift the noise in 2d space. Default is (0,0).
    * `jitter` (float): Adjust uniformity of Worley noise; for other noise types jitters the results.
    * `outmin` (float): The lowest values fit to the noise. Default is 0.0.
    * `outmax` (float): The highest values fit to the noise. Default is 1.0.
    * `clampoutput` (boolean): Clamp the output to the min and max output values.
    * `octaves` (integer): The number of octaves of Fractal noise to be generated. Default is 3.
    * `lacunarity` (float): The exponential scale between successive octaves of Fractal noise. Default is 2.0.
    * `diminish` (float): The rate at which noise amplitude is diminished for each octave of Fractal noise. Default is 0.5.

<a id="node-unifiednoise3d"> </a>

* **`unifiednoise3d`** (NG): a single node supporting 3D Perlin, Cell, Worley or Fractal noise in a unified interface.
    * `type` (integer): The type of noise function to use.  One of 0 (Perlin), 1 (Cell), 2 (Worley), or 3 (Fractal); default is Perlin.
    * `position` (vector3): the input 3d space. Default is position in object-space.
    * `freq` (vector3): Adjusts the noise frequency, with higher values producing smaller noise shapes. Default is (1,1,1).
    * `offset` (vector3): Shift the noise in 3d space. Default is (0,0,0).
    * `jitter` (float): Adjust uniformity of Worley noise; for other noise types jitters the results.
    * `outmin` (float): The lowest values fit to the noise. Default is 0.0.
    * `outmax` (float): The highest values fit to the noise. Default is 1.0.
    * `clampoutput` (boolean): Clamp the output to the min and max output values.
    * `octaves` (integer): The number of octaves of Fractal noise to be generated. Default is 3.
    * `lacunarity` (float): The exponential scale between successive octaves of Fractal noise. Default is 2.0.
    * `diminish` (float): The rate at which noise amplitude is diminished for each octave of Fractal noise. Default is 0.5.


To scale or offset the noise pattern generated by a 3D noise node such as `noise3d`, `fractal3d` or `cellnoise3d`, use a &lt;position> or other [Geometric Node](#geometric-nodes) (see below) connected to vector3 &lt;multiply> and/or &lt;add> nodes, in turn connected to the noise node's `position` input.  To scale or offset the noise pattern generated by a 2D noise node such as `noise2d` or `cellnoise2d`, use a &lt;texcoord> or similar Geometric node processed by vector2 &lt;multiply>, &lt;rotate> and/or &lt;add> nodes, and connect to the node's `texcoord` input.



## Shape Nodes

Shape nodes are used to generate shapes or patterns in UV space.

```xml
  <checkerboard name="n10" type="color3">
    <input name="color1" type="color3" value="1.0,0.0,0.0"/>
    <input name="color2" type="color3" value="0.0,0.0,1.0"/>
    <input name="uvtiling" type="vector2" value="8, 8"/>
  </checkerboard>
```

Standard Shape nodes:

<a id="node-checkerboard"> </a>

* **`checkerboard`** (NG): a 2D checkerboard pattern.  Output type color3.
    * `color1` (color3): The first color used in the checkerboard pattern.
    * `color2` (color3): The second color used in the checkerboard pattern.
    * `uvtiling` (vector2): The tiling of the checkerboard pattern along each axis, with higher values producing smaller squares. Default is (8, 8).
    * `uvoffset` (vector2): The offset of the checkerboard pattern along each axis. Default is (0, 0).
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.

<a id="node-line"> </a>

* **`line`** (NG): Returns 1 if texcoord is at less than radius distance from a line segment defined by point1 and point2; otherwise returns 0.  Output type float.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `center` (vector2): An offset value added to both the point1 and point2 coordinates, default is (0, 0).
    * `radius` (float): The radius or "half thickness" of the line, default is 0.1.
    * `point1` (vector2): The UV coordinate of the first endpoint, default is (0.25, 0.25).
    * `point2` (vector2): The UV coordinate of the second endpoint, default is (0.75, 0.75).

<a id="node-circle"> </a>

* **`circle`** (NG): Returns 1 if texcoord is inside a circle defined by center and radius; otherwise returns 0.  Output type float.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `center` (vector2): The center coordinate of the circle, default is (0, 0).
    * `radius` (float): The radius of the circle, default is 0.5.

<a id="node-cloverleaf"> </a>

* **`cloverleaf`** (NG): Returns 1 if texcoord is inside a cloverleaf shape described by four semicircles on the edges of a square defined by center and radius; otherwise returns 0.  Output type float.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `center` (vector2): 2x the coordinate of the center of the cloverleaf pattern, default is (0, 0); a value of (1,1) will center the cloverleaf in the 0-1 UV space.
    * `radius` (float): The radius of the complete cloverleaf pattern, default is 0.5 resulting in a cloverleaf pattern filling the 0-1 UV boundary.

<a id="node-hexagon"> </a>

* **`hexagon`** (NG): Returns 1 if texcoord is inside a hexagon shape inscribed by a circle defined by center and radius; otherwise returns 0.  Output type float.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `center` (vector2): The center coordinate of the hexagon, default is (0, 0).
    * `radius` (float): The inner (edge center to opposite edge center) radius of the hexagon, default is 0.5.

<a id="node-grid"> </a>

* **`grid`** (NG): Creates a grid pattern of (1, 1, 1) white lines on a (0, 0, 0) black background with the given tiling, offset, and line thickness.  Pattern can be regular or staggered.  Output type color3.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `uvtiling` (vector2): Tiling factor, with higher values producing a denser grid.  Default is (1, 1).
    * `uvoffset` (vector2): UV Offset, default is (0, 0).
    * `thickness` (float): The thickness of the grid lines, default is 0.05.
    * `staggered` (boolean): If true, every other row will be offset 50% to produce a "brick wall" pattern.  Default is false.

<a id="node-crosshatch"> </a>

* **`crosshatch`** (NG): Creates a crosshatch pattern with the given tiling, offset, and line thickness.  Pattern can be regular or staggered.  Output type color3.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `uvtiling` (vector2): Tiling factor, with higher values producing a denser grid.  Default is (1, 1).
    * `uvoffset` (vector2): UV Offset, default is (0, 0).
    * `thickness` (float): The thickness of the grid lines, default is 0.05.
    * `staggered` (boolean): If true, every other row will be offset 50% to produce an "alternating diamond" pattern.  Default is false.

<a id="node-tiledcircles"> </a>

* **`tiledcircles`** (NG): Creates a black and white pattern of circles with a defined tiling and size (diameter).  Pattern can be regular or staggered.  Output type color3.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `uvtiling` (vector2): Tiling factor, with higher values producing a denser grid.  Default is (1, 1).
    * `uvoffset` (vector2): UV Offset, default is (0, 0).
    * `size` (float): The diameter of the circles in the tiled pattern, default is 0.5; if `size` is 1.0, the edges of adjacent circles in the tiling will exactly touch.
    * `staggered` (boolean): If true, every other row will be offset 50%, and the spacing of the tiling will be adjusted in the V direction to center the circles on the vertices of an equilateral triangle grid.  Default is false.

<a id="node-tiledcloverleafs"> </a>

* **`tiledcloverleafs`** (NG): Creates a black and white pattern of cloverleafs with a defined tiling and size (diameter of the circles circumscribing the shape).  Pattern can be regular or staggered.  Output type color3.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `uvtiling` (vector2): Tiling factor, with higher values producing a denser grid.  Default is (1, 1).
    * `uvoffset` (vector2): UV Offset, default is (0, 0).
    * `size` (float): The outer diameter of the cloverleafs in the tiled pattern, default is 0.5; if `size` is 1.0, the edges of adjacent cloverleafs in the tiling will exactly touch.
    * `staggered` (boolean): If true, an additional pattern of cloverleafs will be generated in between the originals offset by 50% in both U and V.  Default is false.

<a id="node-tiledhexagons"> </a>

* **`tiledhexagons`** (NG): Creates a black and white pattern of hexagons with a defined tiling and size (diameter of the circles circumscribing the shape).  Pattern can be regular or staggered.  Output type color3.
    * `texcoord` (vector2): The input 2d space. Default is the first texture coordinates.
    * `uvtiling` (vector2): Tiling factor, with higher values producing a denser grid.  Default is (1, 1).
    * `uvoffset` (vector2): UV Offset, default is (0, 0).
    * `size` (float): The inner diameter of the hexagons in the tiled pattern, default is 0.5; if `size` is 1.0, the edges of adjacent hexagons in the U-direcction tiling will exactly touch.
    * `staggered` (boolean): If true, every other row will be offset 50%, and the spacing of the tiling will be adjusted in the V direction to center the hexagons on the vertices of an equilateral triangle grid.  Default is false.



## Geometric Nodes

Geometric nodes are used to reference local geometric properties from within a node graph:

```xml
  <position name="wp1" type="vector3" space="world"/>
  <texcoord name="c1" type="vector2">
    <input name="index" type="integer" value="1"/>
  </texcoord>
```

Standard Geometric nodes:

<a id="node-position"> </a>

* **`position`**: the coordinates associated with the currently-processed data, as defined in a specific coordinate space.  This node must be of type vector3.
    * `space` (uniform string): the name of the coordinate space in which the position is defined.  Default is "object", see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.

<a id="node-normal"> </a>

* **`normal`**: the geometric normal associated with the currently-processed data, as defined in a specific coordinate space.  This node must be of type vector3.
    * `space` (uniform string): the name of the coordinate space in which the normal vector is defined.  Default is "object", see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.

<a id="node-tangent"> </a>

* **`tangent`**: the geometric tangent vector associated with the currently-processed data, as defined in a specific coordinate space.  This node must be of type vector3.
    * `space` (uniform string): the name of the coordinate space in which the tangent vector is defined.  Default is "object", see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.
    * `index` (uniform integer): the index of the texture coordinates against which the tangent is computed.  The default index is 0.

<a id="node-bitangent"> </a>

* **`bitangent`**: the geometric bitangent vector associated with the currently-processed data, as defined in a specific coordinate space.  This node must be of type vector3.
    * `space` (uniform string): the name of the coordinate space in which the bitangent vector is defined.  Default is "object", see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.
    * `index` (uniform integer): the index of the texture coordinates against which the tangent is computed.  The default index is 0.

<a id="node-bump"> </a>

* **`bump`**: offset the surface normal by a scalar value.  This node must be of type type vector3, and is generally connected to a shader node's "normal" input.
    * `height` (float): Amount to offset the surface normal.
    * `scale` (float): Scalar to adjust the height amount.
    * `normal` (vector3): Surface normal; defaults to the current world-space normal.
    * `tangent` (vector3): Surface tangent vector, defaults to the current world-space tangent vector.

<a id="node-texcoord"> </a>

* **`texcoord`**: the 2D or 3D texture coordinates associated with the currently-processed data.  This node must be of type vector2 or vector3.
    * `index` (uniform integer): the index of the texture coordinates to be referenced.  The default index is 0.

<a id="node-geomcolor"> </a>

* **`geomcolor`**: the color associated with the current geometry at the current `position`, generally bound via per-vertex color values.  Can be of type float, color3 or color4, and must match the type of the "color" bound to the geometry.
    * `index` (uniform integer): the index of the color to be referenced, default is 0.

<a id="node-geompropvalue"> </a>

* **`geompropvalue`**: the value of the specified varying geometric property (defined using &lt;geompropdef>) of the currently-bound geometry.  This node's type must match that of the referenced geomprop.
    * `geomprop` (uniform string): the geometric property to be referenced.
    * `default` (same type as the geomprop's value): a value to return if the specified `geomprop` is not defined on the current geometry.

<a id="node-geompropvalueuniform"> </a>

* **`geompropvalueuniform`**: the value of the specified uniform geometric property (defined using &lt;geompropdef>) of the currently-bound geometry.  This node's type must match that of the referenced geomprop.
    * `geomprop` (uniform string): the geometric property to be referenced.
    * `default` (same type as the geomprop's value): a value to return if the specified `geomprop` is not defined on the current geometry.

Additionally, the `geomcolor` and `geompropvalue` nodes for color3/color4-type properties can take a `colorspace` attribute to declare what colorspace the color property value is in; the default is "none" for no colorspace declaration (and hence no colorspace conversion).



## Application Nodes

Application nodes are used to reference application-defined properties within a node graph, and have no inputs:

```xml
  <frame name="f1" type="float"/>
  <time name="t1" type="float"/>
```

Standard Application nodes:

<a id="node-frame"> </a>

* **`frame`**: the current frame number as defined by the host environment.  This node must be of type float.  Applications may use whatever method is appropriate to communicate the current frame number to the &lt;frame> node's implementation, whether via an internal state variable, a custom input, or other method.

<a id="node-time"> </a>

* **`time`**: the current time in seconds, as defined by the host environment.  This node must be of type float.  Applications may use whatever method is appropriate to communicate the current time to the &lt;time> node's implementation, whether via an internal state variable, a custom input, dividing the current frame number by a local "frames per second" value, or other method; real-time applications may return some variation of wall-clock time.

<br>


# Standard Operator Nodes

Operator nodes process one or more required input streams to form an output.  Like other nodes, each operator must define its output type, which in most cases also determines the type(s) of the required input streams.

```xml
  <multiply name="n7" type="color3">
    <input name="in1" type="color3" nodename="n5"/>
    <input name="in2" type="float" value="2.0"/>
  </multiply>
  <over name="n11" type="color4">
    <input name="fg" type="color4" nodename="n8"/>
    <input name="bg" type="color4" nodename="inbg"/>
  </over>
  <add name="n2" type="color3">
    <input name="in1" type="color3" nodename="n12"/>
    <input name="in2" type="color3" nodename="img4"/>
  </add>
```

The inputs of compositing operators are called "fg" and "bg" (plus "alpha" for float and color3 variants, and "mix" for all variants of the `mix` operator), while the inputs of other operators are called "in" if there is exactly one input, or "in1", "in2" etc. if there are more than one input.  If an implementation does not support a particular operator, it should pass through the "bg", "in" or "in1" input unchanged.

This section defines the Operator Nodes that all MaterialX implementations are expected to support.  Standard Operator Nodes are grouped into the following classifications: [Math Nodes](#math-nodes), [Adjustment Nodes](#adjustment-nodes), [Compositing Nodes](#compositing-nodes), [Conditional Nodes](#conditional-nodes), [Channel Nodes](#channel-nodes) and [Convolution Nodes](#convolution-nodes).



## Math Nodes

Math nodes have one or two spatially-varying inputs, and are used to perform a math operation on values in one spatially-varying input stream, or to combine two spatially-varying input streams using a specified math operation.  The given math operation is performed for each channel of the input stream(s), and the data type of each input must either match that of the input stream(s), or be a float value that will be applied to each channel separately.


<a id="node-add"> </a>

* **`add`**: add a value to the incoming float/color/vector/matrix, or, add one integer value to another.
    * `in1` (float or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>, or integer): the value or nodename for the primary input; for matrix types, the default is the zero matrix.
    * `in2` (same type as `in1` or float, or integer): the value or nodename to add; for matrix types, the default is the zero matrix.

<a id="node-subtract"> </a>

* **`subtract`**: subtract a value from the incoming float/color/vector/matrix, or subtract one integer value from another; in either case, outputting "in1-in2".
    * `in1` (float or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>, or integer): the value or nodename for the primary input; for matrix types, the default is the zero matrix.
    * `in2` (same type as `in1` or float, or integer): the value or nodename to subtract; for matrix types, the default is the zero matrix

<a id="node-multiply"> </a>

* **`multiply`**: multiply an incoming float/color/vector/matrix by a value.  Multiplication of two vectors is interpreted as a component-wise vector multiplication, while multiplication of two matrices is interpreted as a standard matrix product.  To multiply a vector and a matrix, use one of the `transform*` nodes.
    * `in1` (float or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>): the value or nodename for the primary input
    * `in2` (same type as `in1` or float): the value or nodename to multiply by; default is 1.0 in all channels for float/color/vector types, or the identity matrix for matrix types.

<a id="node-divide"> </a>

* **`divide`**: divide an incoming float/color/vector/matrix by a value; dividing a channel value by 0 results in floating-point "NaN".  Division of two vectors is interpreted as a component-wise division of the first vector by the second, while division of two matrices is interpreted as a standard matrix product of the `in1` matrix and the inverse of the `in2` matrix.
    * `in1` (float or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>): the value or nodename for the primary input
    * `in2` (same type as `in1` or float): the value or nodename to divide by; default is 1.0 in all channels for float/color/vector types, or the identity matrix for matrix types.

<a id="node-modulo"> </a>

* **`modulo`**: the remaining fraction after dividing an incoming float/color/vector by a value and subtracting the integer portion.  Modulo always returns a non-negative result, matching the interpretation of the GLSL and OSL `mod()` function (not `fmod()`).
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the value or nodename for the primary input
    * `in2` (same type as `in1` or float): the modulo value or nodename to divide by, cannot be 0 in any channel; default is 1.0 in all channels, which effectively returns the fractional part of a float value

<a id="node-fract"> </a>

* **`fract`**: the fractional part of a float-based value.
    * `in` (float or vector<em>N</em>): the value or nodename for the primary input

<a id="node-invert"> </a>

* **`invert`**: subtract the incoming float/color/vector from "amount" in all channels, outputting: `amount - in`.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the value or nodename for the primary input
    * `amount` (same type as `in` or float): the value or nodename to subtract from; default is 1.0 in all channels

<a id="node-absval"> </a>

* **`absval`**: the per-channel absolute value of the incoming float/color/vector.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-sign"> </a>

* **`sign`**: the per-channel sign of the incoming float/color/vector value: -1 for negative, +1 for positive, or 0 for zero.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-floor"> </a>

* **`floor`**: the per-channel nearest integer value less than or equal to the incoming float/color/vector.  The output remains in floating point per-channel, i.e. the same type as the input, except that the floor(float) also has a variant outputting an integer type.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-ceil"> </a>

* **`ceil`**: the per-channel nearest integer value greater than or equal to the incoming float/color/vector.  The output remains in floating point per-channel, i.e. the same type as the input, except that the ceil(float) also has a variant outputting an integer type.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-round"> </a>

* **`round`**: round each channel of the incoming float/color/vector values to the nearest integer value, e.g "floor(in+0.5)"; the round(float) also has a variant outputting an integer type.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-power"> </a>

* **`power`**: raise incoming float/color values to the specified exponent, commonly used for "gamma" adjustment.
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the value or nodename for the primary input
    * `in2` (same type as `in1` or float): exponent value or nodename; output = pow(in1, in2); default is 1.0 in all channels

<a id="node-safepower"> </a>

* **`safepower`** (NG): raise incoming float/color values to the specified exponent.  Unlike the standard [&lt;power>](#node-power) node, negative `in1` values for &lt;safepower> will result in negative output values, e.g. `out = sign(in1)*pow(abs(in1),in2)`.
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the value or nodename for the primary input
    * `in2` (same type as `in1` or float): exponent value or nodename; default is 1.0 in all channels

<a id="node-sin"> </a>

* **`sin`**: the sine of the incoming value, which is expected to be expressed in radians.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-cos"> </a>

* **`cos`**: the cosine of the incoming value, which is expected to be expressed in radians.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-tan"> </a>

* **`tan`**: the tangent of the incoming value, which is expected to be expressed in radians.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-asin"> </a>

* **`asin`**: the arcsine of the incoming value; the output will be expressed in radians.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-acos"> </a>

* **`acos`**: the arccosine of the incoming value; the output will be expressed in radians.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-atan2"> </a>

* **`atan2`**: the arctangent of the expression (iny/inx); the output will be expressed in radians.  If both `iny` and `inx` are provided, they must be the same type.
    * `iny` (float or vector<em>N</em>): the value or nodename for the "y" input; default is 0.0.
    * `inx` (float or vector<em>N</em>): the value or nodename for the "x" input; default is 1.0.

<a id="node-sqrt"> </a>

* **`sqrt`**: the square root of the incoming value.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-ln"> </a>

* **`ln`**: the natural log of the incoming value.
    * `in` (float or vector<em>N</em>): the input value or nodename; default is 1.0.

<a id="node-exp"> </a>

* **`exp`**: "e" to the power of the incoming value.
    * `in` (float or vector<em>N</em>): the input value or nodename

<a id="node-clamp"> </a>

* **`clamp`**: clamp incoming values per-channel to a specified range of float/color/vector values.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `low` (same type as `in` or float): clamp low value; any value lower than this will be set to "low".  Default is 0 in all channels.
    * `high` (same type as `in` or float): clamp high value; any value higher than this will be set to "high".  Default is 1 in all channels.

<a id="node-trianglewave"> </a>

* **`trianglewave`**: Generate a triangle wave from the given scalar input.  The generated wave ranges from zero to one and repeats on integer boundaries.
    * `in` (float): the scalar value or nodename

<a id="node-min"> </a>

* **`min`**: select the minimum of the two incoming values
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the first value or nodename
    * `in2` (same type as `in1` or float): the second value or nodename

<a id="node-max"> </a>

* **`max`**: select the maximum of the two incoming values
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the first value or nodename
    * `in2` (same type as `in1` or float): the second value or nodename

<a id="node-normalize"> </a>

* **`normalize`**: output the normalized vector<em>N</em> from the incoming vector<em>N</em> stream; cannot be used on float or color<em>N</em> streams.  Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.
    * `in` (vector<em>N</em>): the input value or nodename

<a id="node-magnitude"> </a>

* **`magnitude`**: output the float magnitude (vector length) of the incoming vector<em>N</em> stream; cannot be used on float or color<em>N</em> streams.  Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.
    * `in` (vector<em>N</em>): the input value or nodename


<a id="node-distance"> </a>

* **`distance`**: Measures the distance between two points in 2D, 3D, or 4D.
    * `in1` (vector<em>N</em>): the first input value or nodename
    * `in2` (same type as `in1`): the second input value or nodename

<a id="node-dotproduct"> </a>

* **`dotproduct`**: output the (float) dot product of two incoming vector<em>N</em> streams; cannot be used on float or color<em>N</em> streams.
    * `in1` (vector<em>N</em>): the input value or nodename for the primary input.
    * `in2` (same type as `in1`): the secondary value or nodename

<a id="node-crossproduct"> </a>

* **`crossproduct`**: output the (vector3) cross product of two incoming vector3 streams; cannot be used on any other stream type.  A disabled `crossproduct` node passes through the value of `in1` unchanged.
    * `in1` (vector3): the input value or nodename for the primary input.
    * `in2` (vector3): the secondary value or nodename

<a id="node-transformpoint"> </a>

* **`transformpoint`**: transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.
    * `in` (vector3): the input coordinate vector.
    * `fromspace` (uniform string): the name of a vector space understood by the rendering target to transform the `in` point from; see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.
    * `tospace` (uniform string): the name of a vector space understood by the rendering target for the space to transform the `in` point to.

<a id="node-transformvector"> </a>

* **`transformvector`**: transform the incoming vector3 vector from one specified space to another; cannot be used on any other stream type.
    * `in` (vector3): the input vector.
    * `fromspace` (uniform string): the name of a vector space understood by the rendering target to transform the `in` point from; see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.
    * `tospace` (uniform string): the name of a vector space understood by the rendering target for the space to transform the `in` point to.

<a id="node-transformnormal"> </a>

* **`transformnormal`**: transform the incoming vector3 normal from one specified space to another; cannot be used on any other stream type.
    * `in` (vector3): the input normal vector; default is (0,0,1).
    * `fromspace` (uniform string): the name of a vector space understood by the rendering target to transform the `in` point from; see [Geometric Spaces](./MaterialX.Specification.md#geometric-spaces) in the main Specification document for supported options.
    * `tospace` (uniform string): the name of a vector space understood by the rendering target for the space to transform the `in` point to.

<a id="node-transformmatrix"> </a>

* **`transformmatrix`**: transform the incoming vector<em>N</em> coordinate by the specified matrix.
    * `in` (vector<em>N</em>): the input vector.  If needed, an additional 1.0 component will be temporarily appended to the `in` vector to make it match the dimension of the transforming `mat` matrix, then removed after transformation.
    * `mat` matrix33/44): the matrix used to transform the vector; a vector2 `in` can be transformed by a matrix33, a vector3 by a matrix33 or a matrix44, and a vector4 by a matrix44.  Default is the identity matrix.

<a id="node-normalmap"> </a>

* **`normalmap`**: transform a normal vector from encoded tangent space to world space.  The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.
    * `in` (vector3): the input vector; default is (0.5, 0.5, 1.0).
    * `scale` (float or vector2): a scalar multiplier for the (x,y) components of the incoming vector; defaults to 1.0
    * `normal` (vector3): surface normal; defaults to the current world-space normal.
    * `tangent` (vector3): surface tangent vector, defaults to the current world-space tangent vector. 
    * `bitangent` (vector3): surface bitangent vector, defaults to the current world-space bitangent vector. 

<a id="node-creatematrix"> </a>

* **`creatematrix`**: build a 3x3 or 4x4 matrix from three vector3 or four vector3 or vector4 inputs.  A matrix44 may also be created from vector3 input values, in which case the fourth value will be set to 0.0 for in1-in3, and to 1.0 for in4 when creating the matrix44.
    * `in1` (vector3 or vector4): the vector for the first row of the matrix.  Default is (1,0,0) for matrix33 or (1,0,0,0) for matrix44.
    * `in2` (vector3 or vector4): the vector for the second row of the matrix.  Default is (0,1,0) for matrix33 or (0,1,0,0) for matrix44.
    * `in3` (vector3 or vector4): the vector for the third row of the matrix.  Default is (0,0,1) for matrix33 or (0,0,1,0) for matrix44.
    * `in4` (vector3 or vector4): For matrix44 output type, the vector for the fourth row of the matrix.  Default is (0, 0, 0, 1).

<a id="node-transpose"> </a>

* **`transpose`**: output the transpose of the incoming matrix.
    * `in` (matrix<em>NN</em>): the input value or nodename

<a id="node-determinant"> </a>

* **`determinant`**: output the float determinant of the incoming matrix<em>NN</em> stream.
    * `in` (matrix<em>NN</em>): the input value or nodename

<a id="node-invertmatrix"> </a>

* **`invertmatrix`**: output the inverse of the incoming matrix; if the input matrix is not invertible, the output matrix will consist of all floating-point "NaN" values.
    * `in` (matrix<em>NN</em>): the input value or nodename

<a id="node-rotate2d"> </a>

* **`rotate2d`**: rotate a vector2 value about the origin in 2D.
    * `in` (vector2): the input value or nodename
    * `amount` (float): the amount to rotate, specified in degrees, with positive values rotating the incoming vector counterclockwise.  Default is 0.

<a id="node-rotate3d"> </a>

* **`rotate3d`**: rotate a vector3 value about a specified unit axis vector.
    * `in` (vector3): the input value or nodename
    * `amount` (float): the amount to rotate, specified in degrees; default is 0.
    * `axis` (vector3): For vector3 inputs only, the unit axis vector about which to rotate; default is (0,1,0).

<a id="node-reflect"> </a>

* **`reflect`**: computes the vector3 reflection of an input vector against a surface normal vector.
    * `in` (vector3): the input vector to reflect, defaults to (1.0, 0.0, 0.0).
    * `normal` (vector3): the normal vector about which to reflect "in", defaults to the value of the "Nworld" (world space view direction) geometric property.  This vector is expected to be prenormalized to length 1.0.

<a id="node-refract"> </a>

* **`refract`**: computes the vetor3 refraction vector of an input vector through a surface with a given index of refraction.
    * `in` (vector3): the input vector to refract, defaults to (1.0, 0.0, 0.0).
    * `normal` (vector3): the normal vector about which to refract "in", defaults to the value of the "Nworld" (world space view direction) geometric property.  This vector is expected to be prenormalized to length 1.0.
    * `ior` (float): the index of refraction of the surface, defaults to 1.0.

<a id="node-place2d"> </a>

* **`place2d`** (NG): transform incoming UV texture coordinates for 2D texture placement.
    * `texcoord` (vector2): the input UV coordinate to transform; defaults to the current surface index=0 uv coordinate.
    * `pivot` (vector2): the pivot coordinate for scale and rotate: this is subtracted from u,v before applying scale/rotate, then added back after.  Default is (0,0).
    * `scale` (vector2): divide the u,v coord (after subtracting `pivot`) by this, so a scale (2,2) makes the texture image appear twice as big.  Negative values can be used to flip or flop the texture space.  Default is (1,1).
    * `rotate` (float): rotate u,v coord (after subtracting pivot) by this amount in degrees, so a positive value rotates UV coords counter-clockwise, and the image clockwise.  Default is 0.
    * `offset` (vector2): subtract this amount from the scaled/rotated/“pivot added back” UV coordinate; since U0,V0 is typically the lower left corner, a positive offset moves the texture image up and right.  Default is (0,0).
    * `operationorder` (integer enum): the order in which to perform the transform operations. "0" or "SRT" performs "<em>-pivot scale rotate translate +pivot</em>" as per the original implementation matching the behavior of certain DCC packages, and "1" or "TRS" performs "<em>-pivot translate rotate scale +pivot</em>" which does not introduce texture shear.  Default is 0 "SRT" for backward compatibility.

<a id="node-dot"> </a>

* **`dot`**: a no-op, passes its input through to its output unchanged.  Users can use dot nodes to shape edge connection paths or provide documentation checkpoints in node graph layout UI's.  Dot nodes may also pass uniform values from &lt;constant> or other nodes with uniform="true" outputs to uniform &lt;input>s and &lt;token>s.
    * `in` (any type): the nodename to be connected to the Dot node's "in" input.  Unlike inputs on other node types, the &lt;dot> node's input is specifically disallowed to provide a `channels` attribute: input data can only be passed through unmodified.


## Logical Operator Nodes

Logical operator nodes have one or two boolean typed inputs, and are used to construct higher level logical flow through the nodegraph.

<a id="node-and"> </a>

* **`and`**: logically And the two input boolean values.
  * `in1` (boolean): the value or nodename for the first input; the default is false.
  * `in2` (boolean): the value or nodename for the second input; the default is false.

<a id="node-or"> </a>

* **`or`**: logically Inclusive Or the two input boolean values.
  * `in1` (boolean): the value or nodename for the first input; the default is false.
  * `in2` (boolean): the value or nodename for the second input; the default is false.

<a id="node-xor"> </a>

* **`xor`**: logically Exclusive Or the two input boolean values.
  * `in1` (boolean): the value or nodename for the first input; the default is false.
  * `in2` (boolean): the value or nodename for the second input; the default is false.

<a id="node-not"> </a>

* **`not`**: logically Not the input boolean value.
  * `in` (boolean): the value or nodename for the input; the default is false.


## Adjustment Nodes

Adjustment nodes have one input named "in", and apply a specified function to values in the incoming stream.

<a id="node-contrast"> </a>

* **`contrast`** (NG): increase or decrease contrast of incoming float/color values using a linear slope multiplier.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `amount` (same type as `in` or float): slope multiplier for contrast adjustment, 0.0 to infinity range.  Values greater than 1.0 increase contrast, values between 0.0 and 1.0 reduce contrast.  Default is 1.0 in all channels.
    * `pivot` (same type as `in` or float): center pivot value of contrast adjustment; this is the value that will not change as contrast is adjusted.  Default is 0.5 in all channels.

<a id="node-remap"> </a>

* **`remap`**: linearly remap incoming values from one range of float/color/vector values to another.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `inlow` (same type as `in` or float): low value for input range; default is 0.0 in all channels
    * `inhigh` (same type as `in` or float): high value for input range; default is 1.0 in all channels
    * `outlow` (same type as `in` or float): low value for output range; default is 0.0 in all channels
    * `outhigh` (same type as `in` or float): high value for output range; default is 1.0 in all channels

<a id="node-range"> </a>

* **`range`** (NG): remap incoming values from one range of float/color/vector values to another, optionally applying a gamma correction "in the middle".  Input values below `inlow` or above `inhigh` are extrapolated unless `doclamp` is true, in which case the output values will be clamped to the `outlow`..`outhigh` range.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `inlow` (same type as `in` or float): low value for input range.  Default is 0.0 in all channels.
    * `inhigh` (same type as `in` or float): high value for input range.  Default is 1.0 in all channels.
    * `gamma` (same type as `in` or float): inverse exponent applied to input value after first transforming from `inlow`..`inhigh` to 0..1; `gamma` values greater than 1.0 make midtones brighter.  Default is 1.0 in all channels.
    * `outlow` (same type as `in` or float): low value for output range.  Default is 0.0 in all channels.
    * `outhigh` (same type as `in` or float): high value for output range.  Default is 1.0 in all channels.
    * `doclamp` (boolean): If true, the output is clamped to the range `outlow`..`outhigh`.  Default is false.

<a id="node-smoothstep"> </a>

* **`smoothstep`**: output a smooth (hermite-interpolated) remapping of input values from low-high to output 0-1.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `low` (same type as `in` or float): input low value; an input value of this or lower will result in an output value of 0; default is 0.0 in all channels
    * `high` (same type as `in` or float): input high value; an input value of this or higher will result in an output value of 1; default is 1.0 in all channels

<a id="node-luminance"> </a>

* **`luminance`**: (color3 or color4 only) output a grayscale value containing the luminance of the incoming RGB color in all color channels, computed using the dot product of the incoming color with the luma coefficients of the working colorspace; the alpha channel is left unchanged if present.
    * `in` (color3/color4): the input value or nodename
    * `lumacoeffs` (uniform color3): the luma coefficients of the current working color space; if no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.2722287, 0.6740818, 0.0536895] will be used.  Applications which support color management systems may choose to retrieve the luma coefficients of the working colorspace from the CMS to pass to the &lt;luminance> node's implementation directly, rather than exposing it to the user.

<a id="node-rgbtohsv"> </a>

* **`rgbtohsv`**: (color3 or color4 only) convert an incoming color from RGB to HSV space (with H and S ranging from 0 to 1); the alpha channel is left unchanged if present.  This conversion is not affected by the current color space.
    * `in` (color3/color4): the input value or nodename

<a id="node-hsvtorgb"> </a>

* **`hsvtorgb`**: (color3 or color4 only) convert an incoming color from HSV to RGB space; the alpha channel is left unchanged if present.  This conversion is not affected by the current color space.
    * `in` (color3/color4): the input value or nodename

<a id="node-hsvadjust"> </a>

* **`hsvadjust`** (NG): adjust the hue, saturation and value of an RGB color by converting the input color to HSV, adding amount.x to the hue, multiplying the saturation by amount.y, multiplying the value by amount.z, then converting back to RGB.  A positive "amount.x" rotates hue in the "red to green to blue" direction, with amount of 1.0 being the equivalent to a 360 degree (e.g. no-op) rotation.  Negative or greater-than-1.0 hue adjustment values are allowed, wrapping at the 0-1 boundaries.  The internal conversions between RGB and HSV spaces are not affected by the current color space.  For color4 inputs, the alpha value is unchanged.
    * `in` (color3 or color4): the input value or nodename
    * `amount` (vector3): the HSV adjustment; a value of (0, 1, 1) is "no change" and is the default.

<a id="node-saturate"> </a>

* **`saturate`** (NG): (color3 or color4 only) adjust the saturation of a color; the alpha channel will be unchanged if present.  Note that this operation is **not** equivalent to the "amount.y" saturation adjustment of `hsvadjust`, as that operator does not take the working or any other colorspace into account.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `amount` (float): a multiplier for saturation; the saturate operator performs a linear interpolation between the luminance of the incoming color value (copied to all three color channels) and the incoming color value itself.  Note that setting amount to 0 will result in an R=G=B gray value equal to the value that the `luminance` node (below) returns.  Default is 1.0.
    * `lumacoeffs` (uniform color3): the luma coefficients of the current working color space; if no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.272287, 0.6740818, 0.0536895] will be used.  Applications which support color management systems may choose to retrieve this value from the CMS to pass to the &lt;saturate> node's implementation directly, rather than exposing it to the user.

<a id="node-colorcorrect"> </a>

* **`colorcorrect`** (NG): Combines various adjustment nodes into one artist-friendly color correction node.  For color4 inputs, the alpha value is unchanged.
    * `in` (color3 or color4): the input color to be adjusted.
    * `hue` (float): Rotates the color hue, with values wrapping at 0-1 boundaries; default is 0.
    * `saturation` (float): Multiplies the input color saturation level; default is 1.
    * `gamma` (float): Applies a gamma correction to the color; default is 1.
    * `lift` (float): Raise the dark color values, leaving the white values unchanged; default is 0.
    * `gain` (float): Multiplier increases lighter color values, leaving black values unchanged; default is 1.
    * `contrast` (float): Linearly increase or decrease the color contrast; default is 1.
    * `contrastpivot` (float): Pivot value around which contrast applies. This value will not change as contrast is adjusted; default is 0.5.
    * `exposure` (float): Multiplier which increases or decreases color brightness by 2^value; default is 0.



## Compositing Nodes

Compositing nodes have two (required) inputs named `fg` and `bg`, and apply a function to combine them.  Compositing nodes are split into five subclassifications: [Premult Nodes](#premult-nodes), [Blend Nodes](#blend-nodes), [Merge Nodes](#merge-nodes), [Masking Nodes](#masking-nodes), and the [Mix Node](#mix-node).


### Premult Nodes

Premult nodes operate on 4-channel (color4) inputs/outputs, have one input named `in`, and either apply or unapply the alpha to the float or RGB color.

<a id="node-premult"> </a>

* **`premult`**: multiply the RGB channels of the input by the Alpha channel of the input.
    * `in` (color4): the input value or nodename; default is (0,0,0,1).

<a id="node-unpremult"> </a>

* **`unpremult`**: divide the RGB channels of the input by the Alpha channel of the input.  If the Alpha value is zero, the original color4 input value is passed through unchanged.
    * `in` (color4): the input value or nodename; default is (0,0,0,1).


### Blend Nodes

Blend nodes take two 1-4 channel inputs and apply the same operator to all channels (the math for alpha is the same as for R or RGB); below, "F" and "B" refer to any individual channel of the `fg` and `bg` inputs respectively.


<a id="node-plus"> </a>

* **`plus`**: returns the sum of the bg and fg inputs (B+F)
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "plus" operation (mix=1); default is 1.


<a id="node-minus"> </a>

* **`minus`**: returns the difference of bg and fg (B-F)
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "minus" operation (mix=1); default is 1.


<a id="node-difference"> </a>

* **`difference`**: returns the absolute-value difference of bg and fg (abs(B-F))
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "difference" operation (mix=1); default is 1.


<a id="node-burn"> </a>

* **`burn`**: a "burn" operation of bg and fg (1-(1-B)/F)
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "burn" operation (mix=1); default is 1.


<a id="node-dodge"> </a>

* **`dodge`**: a "dodge" operation of bg and fg (B/(1-F))
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "dodge" operation (mix=1); default is 1.


<a id="node-screen"> </a>

* **`screen`**: a "screen" operation of bg and fg (1-(1-F)(1-B))
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "screen" operation (mix=1); default is 1.


<a id="node-overlay"> </a>

* **`overlay`**: an "overlay" operation of bg and fg (2FB if B&lt;0.5, or 1-2(1-F)(1-B) if B>=0.5)
    * `bg` (float or color<em>N</em>): the background value or nodename; default is 0.
    * `fg` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "overlay" operation (mix=1); default is 1.



### Merge Nodes

Merge nodes take two 4-channel (color4) inputs and use the built-in alpha channel(s) to control the compositing of the `fg` and `bg` inputs; "F" and "B" refer to individual non-alpha channels of the `fg` and `bg` inputs respectively, while "f" and "b" refer to the alpha channels of the `fg` and `bg` inputs.  Merge nodes are not defined for 1-channel or 3-channel inputs, and cannot be used on vector<em>N</em> streams.


<a id="node-disjointover"> </a>

* **`disjointover`**: a "disjointover" operation returning RGB = F+B if f+b&lt;=1 or F+B(1-f)/b if f+b>1, and Alpha = min(f+b,1)
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "disjointover" operation (mix=1); default is 1.

<a id="node-in"> </a>

* **`in`**: an "in" operation returning RGB = Fb and Alpha = fb
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "in" operation (mix=1); default is 1.

<a id="node-mask"> </a>

* **`mask`**: a "mask" operation returning RGB = Bf and Alpha = bf
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "mask" operation (mix=1); default is 1.

<a id="node-matte"> </a>

* **`matte`**: a "matte" operation returning RGB = Ff+B(1-f) and Alpha = f+b(1-f)
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "matte" operation (mix=1); default is 1.

<a id="node-out"> </a>

* **`out`**: an "out" operation returning RGB = F(1-b) and Alpha = f(1-b)
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "out" operation (mix=1); default is 1.

<a id="node-over"> </a>

* **`over`**: an "over" operation returning RGB = F+B(1-f) and Alpha = f+b(1-f)
    * `bg` (color4): the background value or nodename; default is 0.
    * `fg` (color4): the foreground value or nodename; default is 0.
    * `mix` (float): a 0-1 mixing value between "bg" (mix=0) and the result of the "over" operation (mix=1); default is 1.



### Masking Nodes

Masking nodes take one 1-4 channel input `in` plus a separate float `mask` input and apply the same operator to all "in" channels; "F" refers to any individual channel of the `in` input, while "m" refers to the mask input.


<a id="node-inside"> </a>

* **`inside`**: an "inside" mask operation returning Fm
    * `in` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mask` (float): the 0-1 mask value or nodename; default is 1.

<a id="node-outside"> </a>

* **`outside`** an "outside" mask operation returning F(1-m)
    * `in` (float or color<em>N</em>): the foreground value or nodename; default is 0.
    * `mask` (float): the 0-1 mask value or nodename; default is 0.



### Mix Node

The Mix node takes two 1-4 channel inputs `fg` and `bg` plus a separate 1-channel (float) or N-channel (same type and number of channels as `fg` and `bg`) `mix` input and mixes the `fg` and `bg` according to the mix value, either uniformly for a "float" `mix` type, or per-channel for non-float `mix` types; "F" refers to any individual channel of the `in` input, while "m" refers to the appropriate channel of the mix input.

<a id="node-mix"> </a>

* **`mix`**: a "mix" operation blending from "bg" to "fg" according to the mix amount, returning Fm+B(1-m)
    * `bg` (float or color<em>N</em> or vector<em>N</em>): the background value or nodename; default is 0.
    * `fg` (same type as `bg`): the foreground value or nodename; default is 0.
    * `mix` (float or same type as `bg`): the 0-1 mixing value; default is 0.


See also the [Standard Shader Nodes](#standard-shader-nodes) section below for additional shader-semantic variants of the [`mix` node](#node-mix-shader).



## Conditional Nodes

Conditional nodes are used to compare values of two streams, or to select a value from one of several streams.


<a id="node-ifgreater"> </a>

* **`ifgreater`**: output the value of the `in1` or `in2` stream depending on whether the value of one test input is greater than the value of another.  Ifgreater nodes can be of output type float, integer, color<em>N</em>, vector<em>N</em> or matrix<em>NN</em>.  There is also a "boolean" output-type **`ifgreater`** node, with `value1` and `value2` inputs but no `in1` or `in2`: output is "true" if `value1` > `value2`.
    * `value1` (integer or float): the first value or nodename to compare.  Default is 1.0.
    * `value2` (integer or float): the second value or nodename to compare must be the same type as `value1`.  Default is 0.0.
    * `in1` (float or integer or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` > `value2`; must be the same type as the `ifgreater` node's output.  Default is 0.0 in all channels.
    * `in2` (float or integer or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` &lt;= `value2`; must be the same type as the `ifgreater` node's output.  Default is 0.0 in all channels.

<a id="node-ifgreatereq"> </a>

* **`ifgreatereq`**: output the value of the `in1` or `in2` stream depending on whether the value of one test input is greater than or equal to the value of another.  Ifgreatereq nodes can be of output type float, integer, color<em>N</em>, vector<em>N</em> or matrix<em>NN</em>. There is also a "boolean" output-type **`ifgreatereq`** node, with `value1` and `value2` inputs but no `in1` or `in2`: output is "true" if `value1` >= `value2`.
    * `value1` (integer or float): the first value or nodename to compare.  Default is 1.0.
    * `value2` (integer or float): the second value or nodename to compare; must be the same type as `value1`.  Default is 0.0.
    * `in1` (float or integer or color<em>N </em>or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` >= `value2`; must be the same type as the `ifgreatereq` node's output.  Default is 0.0 in all channels.
    * `in2` (float or integer or color<em>N </em>or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` &lt; `value2`; must be the same type as the `ifgreatereq` node's output.  Default is 0.0 in all channels.

<a id="node-ifequal"> </a>

* **`ifequal`**: output the value of the `in1` or `in2` stream depending on whether the value of two test inputs are equal or not.  Ifequal nodes can be of output type float, integer, color<em>N</em>, vector<em>N</em> or matrix<em>NN</em>. There is also a "boolean" output-type **`ifequal`** node, with `value1` and `value2` inputs but no `in1` or `in2`: output is "true" if `value1` == `value2`.
    * `value1` (boolean or integer or float): the first value or nodename to compare.  Default is 0 or "false".
    * `value2` (boolean or integer or float): the second value or nodename to compare; must be the same type as `value1`.  Default is 0 or "false".
    * `in1` (float or integer or color<em>N </em>or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` == `value2`; must be the same type as the `ifequal` node's output.  Default is 0.0 in all channels.
    * `in2` (float or integer or color<em>N </em>or vector<em>N</em> or matrix<em>NN</em>): the value or nodename to output if `value1` != `value2`; must be the same type as the `ifequal` node's output.  Default is 0.0 in all channels.

<a id="node-switch"> </a>

* **`switch`**: output the value of one of up to ten input streams, according to the value of a selector input `which`.  Switch nodes can be of output type float, color<em>N</em>, vector<em>N</em> or matrix<em>NN</em>, and have ten inputs, in1 through in10 (not all of which need be connected), which must match the output type.
    * `in1`, `in2`, `in3`, `in4`, `in5`, `in6`, `in7`, `in8`, `in9`, `in10` (float or color<em>N</em> or vector<em>N</em> or matrix<em>NN</em>): the values or nodenames to select from based on the value of the `which` input.  The types of the various `in`<em>N</em> inputs must match the type of the `switch` node itself.  The default value of all `in`<em>N</em> inputs is 0.0 in all channels.
    * `which` (integer or float): a selector to choose which input to take values from; the output comes from input "floor(`which`)+1", clamped to the 1-10 range.  So `which`&lt;1 will pass on the value from in1, 1&lt;=`which`&lt;2 will pass the value from in2, 2&lt;=`which`&lt;3 will pass the value from in3, and so on up to 9&lt;=`which` will pass the value from in10.  The default value of `which` is 0.



## Channel Nodes

Channel nodes are used to perform channel manipulations and data type conversions on streams.


<a id="node-extract"> </a>

* **`extract`**: extract the specified channel number from a color<em>N</em> or vector<em>N</em> stream.
    * `in` (color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `index` (integer): the channel number to extract.  For color<em>N</em> streams, use "0" to extract the red channel, "1" for green, "2" for blue and "3" for alpha; for vector<em>N</em> streams, use "0" to extract the x channel, "1" for y, "2" for z and "3" for w.  Default is 0.

<a id="node-convert"> </a>

* **`convert`**: convert a stream from one data type to another.  Only certain unambiguous conversions are supported; see list below.
    * `in` (boolean or integer or float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-combine2"> </a>
<a id="node-combine3"> </a>
<a id="node-combine4"> </a>

* **`combine2`**, **`combine3`**, **`combine4`**: combine the channels from two, three or four streams into the same total number of channels of a single output stream of a specified compatible type; please see the table below for a list of all supported combinations of input and output types.  For colorN output types, no colorspace conversion will take place; the channels are simply copied as-is.
    * `in1` (float/color3/vector2/vector3): the input value or nodename which will be sent to the N channels of the output; default is 0.0 in all channels
    * `in2` (float/vector2): the input value or nodename which will be sent to the next N channels of the output; default is 0.0 in all channels
    * `in3` (float): for **`combine3`** or **`combine4`**, the input value or nodename which will be sent to the next channel of the output after `in2`; default is 0.0
    * `in4` (float): for **`combine4`**, the input value or nodename which will be sent to the last channel of the output; default is 0.0

<a id="node-separate2"> </a>

* **`separate2`** (NG): output each of the channels of a vector2 as a separate float output.
    * `in` (vector2): the input value or nodename
    * `outx` (**output**, float): the value of x channel.
    * `outy` (**output**, float): the value of y channel.

<a id="node-separate3"> </a>

* **`separate3`** (NG): output each of the channels of a color3 or vector3 as a separate float output.
    * `in` (color3 or vector3): the input value or nodename
    * `outr`/`outx` (**output**, float): the value of the red (for color3 streams) or x (for vector3 streams) channel.
    * `outg`/`outy` (**output**, float): the value of the green (for color3 streams) or y (for vector3 streams) channel.
    * `outb`/`outz` (**output**, float): the value of the blue (for color3 streams) or z (for vector3 streams) channel.

<a id="node-separate4"> </a>

* **`separate4`** (NG): output each of the channels of a color4 or vector4 as a separate float output.
    * `in` (color4 or vector4): the input value or nodename
    * `outr`/`outx` (**output**, float): the value of the red (for color4 streams) or x (for vector4 streams) channel.
    * `outg`/`outy` (**output**, float): the value of the green (for color4 streams) or y (for vector4 streams) channel.
    * `outb`/`outz` (**output**, float): the value of the blue (for color4 streams) or z (for vector4 streams) channel.
    * `outa`/`outw` (**output**, float): the value of the alpha (for color4 streams) or w (for vector4 streams) channel.


The following input/output data type conversions are supported by **`convert`**:

* boolean or integer to float: output is 0.0 or 1.0
* boolean to integer: output is 0 or 1
* integer to boolean: true for any non-zero input value
* float/integer/boolean to color<em>N</em>/vector<em>N</em>: copy the input value to all channels of the output
* color<em>N</em> / vector<em>N</em> to color<em>M</em> / vector<em>M</em>
  * if _N_ is the same as _M_, then channels are directly copied. 
  * if _N_ is larger than _M_, then channels the first _N_ channels are used.
  * if _N_ is smaller than _M_, then channels are directly copied and additional channels are populated with 0, aside from the fourth channel which is populated with 1

Table of allowable input/output types for **`combine2`**, **`combine3`**, **`combine4`**:

| Operator | `type` | `in1` | `in2` | `in3` | `in4` | Output |
| --- | --- | --- | --- | --- | --- | --- |
| `combine2` | `vector2` | `float` "x" | `float` "y" | n/a | n/a | "xy" |
| `combine3` | `color3` | `float` "r" | `float` "g" | `float` "b" | n/a | "rgb" |
| `combine3` | `vector3` | `float` "x" | `float` "y" | `float` "z" | n/a | "xyz" |
| `combine4` | `color4` | `float` "r" | `float` "g" | `float` "b" | `float` "a" | "rgba" |
| `combine4` | `vector4` | `float` "x" | `float` "y" | `float` "z" | `float` "w" | "xyzw" |
| `combine2` | `color4` | `color3` "rgb" | `float` "a" | n/a | n/a | "rgba" |
| `combine2` | `vector4` | `vector3` "xyz" | `float` "w" | n/a | n/a | "xyzw" |
| `combine2` | `vector4` | `vector2` "xy" | `vector2` "zw" | n/a | n/a | "xyzw" |




## Convolution Nodes

Convolution nodes have one input named "in", and apply a defined convolution function on the input stream.  Some of these nodes may not be implementable in ray tracing applications; they are provided for the benefit of purely 2D image processing applications.


<a id="node-blur"> </a>

* **`blur`**: a convolution blur.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `size` (float): the size of the blur kernel, relative to 0-1 UV space; default is 0.
    * `filtertype` (uniform string): the spatial filter used in the blur, either "box" for a linear box filter, or "gaussian" for a gaussian filter.  Default is "box".

<a id="node-heighttonormal"> </a>

* **`heighttonormal`**: convert a scalar height map to a tangent-space normal map of type vector3.  The output normal map is encoded with all channels in the [0-1] range, enabling its storage in unsigned image formats.
    * `in` (float): the input value or nodename
    * `scale` (float): the scale of normal map deflections relative to the gradient of the height map.  Default is 1.0.

<br>


# Standard Shader Nodes

The Standard MaterialX Library defines the following nodes and node variants operating on "shader"-semantic types.  Standard library shaders do not respond to external illumination; please refer to the [**MaterialX Physically Based Shading Nodes**](./MaterialX.PBRSpec.md#materialx-pbs-library) document for definitions of additional nodes and shader constructors which do respond to illumination, as well as [**MaterialX NPR Shading Nodes**](./MaterialX.NPRSpec.md) for definitions of shaders and nodes applicable to non-photorealistic rendering.

<a id="node-surface-unlit"> </a>

* **`surface_unlit`**: an unlit surface shader node, representing a surface that can emit and transmit light, but does not receive illumination from light sources or other surfaces.  Output type surfaceshader.
    * `emission` (float): the surface emission amount; default is 1.0
    * `emission_color` (color3): surface emission color; default is (1, 1, 1)
    * `transmission` (float): the surface transmission amount; default is 0
    * `transmission_color` (color3): surface transmission color; default is (1, 1, 1)
    * `opacity` (float): surface cutout opacity; default is 1.0

<a id="node-displacement"> </a>

* **`displacement`**: Constructs a displacement shader describing geometric modification to surfaces.  Output type "displacementshader".
    * `displacement` (float or vector3): Scalar (along the surface normal direction) or vector displacement (in (dPdu, dPdv, N) tangent/normal space) for each position.  Default is 0.
    * `scale` (float): Scale factor for the displacement vector.  Default is 1.0.

<a id="node-mix-shader"> </a>

* **`mix`**: linear blend between two surface/displacement/volumeshader closures.
    * `bg` (surface/displacement/volumeshader): the name of the background shader-semantic node
    * `fg` (surface/displacement/volumeshader): the name of the foreground shader-semantic node
    * `mix` (float): the blending factor used to mix the two input closures
