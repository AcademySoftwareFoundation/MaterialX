<!-----
MaterialX Standard Nodes v1.39
----->


# MaterialX Standard Nodes

**Version 1.39**  
Doug Smythe - Industrial Light & Magic  
Jonathan Stone - Lucasfilm Advanced Development Group  
November 9, 2025


# Introduction

The MaterialX Specification defines a content schema to describe materials, image processing and shading networks and how the nodes in those networks access textural and geometric information, in a platform- and shading-language-independent manner.

This document describes a specific set of **Standard Nodes** that can be used to read and process image and geometric attribute data, as well as create new image data procedurally.  These "stdlib" nodes are an essential core part of all MaterialX implementations.  Additional nodes are described in companion documents [**MaterialX Physically Based Shading Nodes**](./MaterialX.PBRSpec.md) and [**MaterialX NPR Shading Nodes**](./MaterialX.NPRSpec.md).

In the node descriptions below, tables define the names, allowable types, default values, and where appropriate, accepted values for the node's inputs and output(s).  For outputs, the Default value specified is the value output (or passed through from an input) if the node is disabled.  Node descriptions with multiple tables accept any combination of inputs/outputs/types described within any single table.

## Table of Contents

**[Introduction](#introduction)**  

**[Standard Source Nodes](#standard-source-nodes)**  
 [Texture Nodes](#texture-nodes)  
  [Texture Node Notes](#texture-node-notes)  
 [Procedural Nodes](#procedural-nodes)  
  [Procedural Node Notes](#procedural-node-notes)  
 [Noise Nodes](#noise-nodes)  
  [Noise Node Notes](#noise-node-notes)  
 [Shape Nodes](#shape-nodes)  
 [Geometric Nodes](#geometric-nodes)  
  [Geometric Node Notes](#geometric-node-notes)  
 [Application Nodes](#application-nodes)  
  [Application Node Notes](#application-node-notes)  

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

### `image`

Samples data from a single image, or from a layer within a multi-layer image.  When used in the context of rendering a geometry, the image is mapped onto the geometry based on geometry UV coordinates, with the lower-left corner of an image mapping to the (0,0) UV coordinate (or to the fractional (0,0) UV coordinate for tiled images).

The type of the &lt;image> node determines the number of channels output, which may be less than the number of channels in the image file, outputting the first N channels from the image file.  So a `float` &lt;image> would return the Red channel of an RGB image, and a `color3` &lt;image> would return the RGB channels of an RGBA image.  If the type of the &lt;image> node has more channels than the referenced image file, then the output will contain zero values in all channels beyond the N channels of the image file.

The `file` input value can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

If no value for `layer` is provided and the input file has multiple layers, then the "default" layer will be used, or "rgba" if there is no "default" layer. Note: the number of channels defined by the type of the `image` must match the number of channels in the named layer.

The `default` input is the default value to use if the file reference can not be resolved (e.g. if a geometry token, interface token, or hostattr is included in the filename but no substitution value or default is defined, or if the resolved file URI cannot be read), or if the specified layer does not exist in the file. The default value must be the same type as the &lt;image> element itself. If `default` is not defined, the default color value will be 0.0 in all channels.

|Port            |Description                                                                                                      |Type                  |Default  |Accepted Values                  |
|----------------|-----------------------------------------------------------------------------------------------------------------|----------------------|---------|---------------------------------|
|`file`          |The URI of the image file                                                                                        |filename              |__empty__|                                 |
|`layer`         |The name of the layer to extract from a multi-layer input file                                                   |string                |__empty__|                                 |
|`default`       |A default value to use if the file reference can not be resolved                                                 |Same as `out`         |__zero__ |                                 |
|`texcoord`      |The 2D texture coordinate at which the image data is read                                                        |vector2               |_UV0_    |                                 |
|`uaddressmode`  |Determines how U coordinates outside the 0-1 range are processed before sampling the image                       |string                |periodic |constant, clamp, periodic, mirror|
|`vaddressmode`  |Determines how V coordinates outside the 0-1 range are processed before sampling the image                       |string                |periodic |constant, clamp, periodic, mirror|
|`filtertype`    |The type of texture filtering to use                                                                             |string                |linear   |closest, linear, cubic           |
|`framerange`    |A string "minframe-maxframe", e.g. "10-99", to specify the range of frames that the image file is allowed to have|string                |__empty__|                                 |
|`frameoffset`   |A number that is added to the current frame number to get the image file frame number                            |integer               |0        |                                 |
|`frameendaction`|What to do when the resolved image frame number is outside the `framerange` range                                |string                |constant |constant, clamp, periodic, mirror|
|`out`           |Output: the sampled texture value                                                                                |float, colorN, vectorN|__zero__ |                                 |

<a id="node-tiledimage"> </a>

### `tiledimage`
Samples data from a single image, with provisions for tiling and offsetting the image across UV space.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

|Port                |Description                                                                                                      |Type                  |Default  |Accepted Values                  |
|--------------------|-----------------------------------------------------------------------------------------------------------------|----------------------|---------|---------------------------------|
|`file`              |The URI of the image file                                                                                        |filename              |__empty__|                                 |
|`default`           |A default value to use if the file reference can not be resolved                                                 |Same as `out`         |__zero__ |                                 |
|`texcoord`          |The 2D texture coordinate at which the image data is read                                                        |vector2               |_UV0_    |                                 |
|`uvtiling`          |The tiling rate for the given image along the U and V axes                                                       |vector2               |1.0, 1.0 |                                 |
|`uvoffset`          |The offset for the given image along the U and V axes                                                            |vector2               |0.0, 0.0 |                                 |
|`realworldimagesize`|The real-world size represented by the file image                                                                |vector2               |1.0, 1.0 |                                 |
|`realworldtilesize` |The real-world size of a single square 0-1 UV tile                                                               |vector2               |1.0, 1.0 |                                 |
|`filtertype`        |The type of texture filtering to use                                                                             |string                |linear   |closest, linear, cubic           |
|`framerange`        |A string "minframe-maxframe", e.g. "10-99", to specify the range of frames that the image file is allowed to have|string                |__empty__|                                 |
|`frameoffset`       |A number that is added to the current frame number to get the image file frame number                            |integer               |0        |                                 |
|`frameendaction`    |What to do when the resolved image frame number is outside the `framerange` range                                |string                |constant |constant, clamp, periodic, mirror|
|`out`               |Output: the sampled texture value                                                                                |float, colorN, vectorN|__zero__ |                                 |

<a id="node-latlongimage"> </a>

### `latlongimage`
Samples an equiangular map along a view direction with adjustable latitudinal offset.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

|Port      |Description                                                                        |Type    |Default      |
|----------|-----------------------------------------------------------------------------------|--------|-------------|
|`file`    |The URI of the image file                                                          |filename|__empty__    |
|`default` |A default value to use if the file reference can not be resolved                   |color3  |0.0, 0.0, 0.0|
|`viewdir` |The view direction determining the value sampled from the projected equiangular map|vector3 |0.0, 0.0, 1.0|
|`rotation`|The longitudinal sampling offset, in degrees                                       |float   |0.0          |
|`out`     |Output: the sampled texture value                                                  |color3  |0.0, 0.0, 0.0|

<a id="node-hextiledimage"> </a>

### `hextiledimage`
Samples data from a single image, with provisions for hex-tiling and randomizing the image across UV space.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

The `lumacoeffs` input represents the luma coefficients of the current working color space. If no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.2722287, 0.6740818, 0.0536895] will be used. Applications which support color management systems may choose to retrieve the luma coefficients of the working colorspace from the CMS to pass to the node's implementation directly, rather than exposing it to the user.

|Port                |Description                                                                                        |Type             |Default                        |
|--------------------|---------------------------------------------------------------------------------------------------|-----------------|-------------------------------|
|`file`              |The URI of the image file                                                                          |filename         |__empty__                      |
|`default`           |A default value to use if the file reference can not be resolved                                   |Same as `out`    |__zero__                       |
|`texcoord`          |The 2D texture coordinate at which the image data is read                                          |vector2          |_UV0_                          |
|`tiling`            |The tiling rate for the given image along the U and V axes                                         |vector2          |1.0, 1.0                       |
|`rotation`          |Per-tile rotation randomness in degrees                                                            |float            |0.0                            |
|`rotationrange`     |Range in degrees used to randomize rotation for each tile                                          |vector2          |0.0, 360.0                     |
|`scale`             |Per-tile scale randomness multiplier applied to tile size                                          |float            |1.0                            |
|`scalerange`        |Range of scale multipliers used to randomize tile scale                                            |vector2          |0.5, 2.0                       |
|`offset`            |Per-tile translation randomness in UV units                                                        |float            |1.0                            |
|`offsetrange`       |Range of offset values in UV units used to randomize tile positions                                |vector2          |0.0, 1.0                       |
|`falloff`           |Falloff width used to blend neighboring tiles at their edges; larger values produce smoother blends|float            |0.5                            |
|`falloffcontrast`   |Contrast applied to the falloff blending to sharpen (values >1) or soften (values <1) transitions  |float            |0.5                            |
|`lumacoeffs`        |The luma coefficients of the current working color space                                           |color3           |0.2722287, 0.6740818, 0.0536895|
|`out`               |Output: the sampled texture value                                                                  |colorN           |__zero__                       |

<a id="node-triplanarprojection"> </a>

### `triplanarprojection`
Samples data from three images (or layers within multi-layer images), and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.

|Port            |Description                                                                                                      |Type                  |Default  |Accepted Values                  |
|----------------|-----------------------------------------------------------------------------------------------------------------|----------------------|---------|---------------------------------|
|`filex`         |The URI of the image file to be projected in the direction from the +X axis back toward the origin               |filename              |__empty__|                                 |
|`filey`         |The URI of the image file to be projected in the direction from the +Y axis back toward the origin               |filename              |__empty__|                                 |
|`filez`         |The URI of the image file to be projected in the direction from the +Z axis back toward the origin               |filename              |__empty__|                                 |
|`layerx`        |The name of the layer to extract from a multi-layer input file for the X-axis projection                         |string                |__empty__|                                 |
|`layery`        |The name of the layer to extract from a multi-layer input file for the Y-axis projection                         |string                |__empty__|                                 |
|`layerz`        |The name of the layer to extract from a multi-layer input file for the Z-axis projection                         |string                |__empty__|                                 |
|`default`       |A default value to use if the file reference can not be resolved                                                 |Same as `out`         |__zero__ |                                 |
|`position`      |A spatially-varying input specifying the 3D position at which the projection is evaluated                        |vector3               |_Pobject_|                                 |
|`normal`        |A spatially-varying input specifying the 3D normal vector used for blending                                      |vector3               |_Nobject_|                                 |
|`upaxis`        |Which axis is considered to be 'up', either 0 for X, 1 for Y, or 2 for Z                                         |integer               |2        |0, 1, 2                          |
|`blend`         |Weighting factor for blending samples using the geometric normal, with higher values giving softer blending      |float                 |1.0      |                                 |
|`filtertype`    |The type of texture filtering to use                                                                             |string                |linear   |closest, linear, cubic           |
|`framerange`    |A string "minframe-maxframe", e.g. "10-99", to specify the range of frames that the image file is allowed to have|string                |__empty__|                                 |
|`frameoffset`   |A number that is added to the current frame number to get the image file frame number                            |integer               |0        |                                 |
|`frameendaction`|What to do when the resolved image frame number is outside the `framerange` range                                |string                |constant |constant, clamp, periodic, mirror|
|`out`           |Output: the blended texture value                                                                                |float, colorN, vectorN|__zero__ |                                 |


### Texture Node Notes

<a id="addressmode-values"> </a>

The following values are supported by `uaddressmode` and `vaddressmode` inputs of Texture nodes:

* “constant”: Texture coordinates outside the 0-1 range return the value of the node's `default` input.
* “clamp”: Texture coordinates are clamped to the 0-1 range before sampling the image.
* “periodic”: Texture coordinates outside the 0-1 range "wrap around", effectively being processed by a modulo 1 operation before sampling the image.
* "mirror": Texture coordinates outside the 0-1 range will be mirrored back into the 0-1 range, e.g. u=-0.01 will return the u=0.01 texture coordinate value, and u=1.01 will return the u=0.99 texture coordinate value.


<a id="filtertype-values"> </a>

The `filtertype` input for Texture nodes supports options `closest` (nearest-neighbor single-sample), `linear`, and `cubic`.


<a id="framerange-values"> </a>

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

### `constant`
Outputs a constant value.

|Port   |Description                         |Type                                    |Default |
|-------|------------------------------------|----------------------------------------|--------|
|`value`|The value that will be sent to `out`|float, colorN, vectorN, boolean, integer|__zero__|
|`out`  |Output: `value`                     |Same as `value`                         |__zero__|

|Port   |Description                         |Type           |Default|
|-------|------------------------------------|---------------|-------|
|`value`|The value that will be sent to `out`|matrixNN       |__one__|
|`out`  |Output: `value`                     |Same as `value`|__one__|

|Port   |Description                         |Type            |Default  |
|-------|------------------------------------|----------------|---------|
|`value`|The value that will be sent to `out`|string, filename|__empty__|
|`out`  |Output: `value`                     |Same as `value` |__empty__|


<a id="node-ramplr"> </a>

### `ramplr`
A left-to-right linear value ramp.

|Port      |Description                                                        |Type                  |Default |
|----------|-------------------------------------------------------------------|----------------------|--------|
|`valuel`  |Value at the left (U=0) edge                                       |float, colorN, vectorN|__zero__|
|`valuer`  |Value at the right (U=1) edge                                      |Same as `valuel`      |__zero__|
|`texcoord`|2D texture coordinate at which the ramp interpolation is evaluated |vector2               |_UV0_   |
|`out`     |Output: the interpolated ramp value                                |Same as `valuel`      |__zero__|

<a id="node-ramptb"> </a>

### `ramptb`
A top-to-bottom linear value ramp.

|Port      |Description                                                        |Type                  |Default |
|----------|-------------------------------------------------------------------|----------------------|--------|
|`valuet`  |Value at the top (V=1) edge                                        |float, colorN, vectorN|__zero__|
|`valueb`  |Value at the bottom (V=0) edge                                     |Same as `valuet`      |__zero__|
|`texcoord`|2D texture coordinate at which the ramp interpolation is evaluated |vector2               |_UV0_   |
|`out`     |Output: the interpolated ramp value                                |Same as `valuet`      |__zero__|

<a id="node-ramp4"> </a>

### `ramp4`
A 4-corner bilinear value ramp.

|Port      |Description                                                        |Type                  |Default |
|----------|-------------------------------------------------------------------|----------------------|--------|
|`valuetl` |Value at the top-left (U=0, V=1) corner                            |float, colorN, vectorN|__zero__|
|`valuetr` |Value at the top-right (U=1, V=1) corner                           |Same as `valuetl`     |__zero__|
|`valuebl` |Value at the bottom-left (U=0, V=0) corner                         |Same as `valuetl`     |__zero__|
|`valuebr` |Value at the bottom-right (U=1, V=0) corner                        |Same as `valuetl`     |__zero__|
|`texcoord`|2D texture coordinate at which the ramp interpolation is evaluated |vector2               |_UV0_   |
|`out`     |Output: the interpolated ramp value                                |Same as `valuetl`     |__zero__|

<a id="node-splitlr"> </a>

### `splitlr`
A left-right split matte, split at a specified U value.

|Port      |Description                                                         |Type                  |Default |
|----------|--------------------------------------------------------------------|----------------------|--------|
|`valuel`  |Value at the left (U=0) edge                                        |float, colorN, vectorN|__zero__|
|`valuer`  |Value at the right (U=1) edge                                       |Same as `valuel`      |__zero__|
|`center`  |U-coordinate of the split; left of it is `valuel`, right is `valuer`|float                 |0.5     |
|`texcoord`|2D texture coordinate at which the ramp interpolation is evaluated  |vector2               |_UV0_   |
|`out`     |Output: the interpolated split value                                |Same as `valuel`      |__zero__|

<a id="node-splittb"> </a>

### `splittb`
A top-bottom split matte, split at a specified V value.

|Port      |Description                                                        |Type                  |Default |
|----------|-------------------------------------------------------------------|----------------------|--------|
|`valuet`  |Value at the top (V=1) edge                                        |float, colorN, vectorN|__zero__|
|`valueb`  |Value at the bottom (V=0) edge                                     |Same as `valuet`      |__zero__|
|`center`  |V-coordinate of the split; below it is `valueb`, above is `valuet` |float                 |0.5     |
|`texcoord`|2D texture coordinate at which the ramp interpolation is evaluated |vector2               |_UV0_   |
|`out`     |Output: the interpolated split value                               |Same as `valuet`      |__zero__|

<a id="node-randomfloat"> </a>

### `randomfloat`
Produces a stable randomized float value between `min` and `max`, based on an input signal and optional `seed` value.  Uses a 2d cellnoise function to produce the output.

|Port      |Description                                                        |Type          |Default |
|----------|-------------------------------------------------------------------|--------------|--------|
|`in`      |Initial randomization seed                                         |float, integer|__zero__|
|`min`     |The minimum output value                                           |float         |__zero__|
|`max`     |The maximum output value                                           |float         |__one__ |
|`seed`    |Additional randomization seed                                      |integer       |__zero__|
|`out`     |Output: the randomized value                                       |float         |__zero__|

<a id="node-randomcolor"> </a>

### `randomcolor`
Produces a randomized RGB color within a randomized hue, saturation and brightness range, based on an input signal and optional `seed` value.  Output type color3.

|Port            |Description                                                  |Type          |Default |
|----------------|-------------------------------------------------------------|--------------|--------|
|`in`            |Initial randomization seed                                   |float, integer|__zero__|
|`huelow`        |The minimum hue value                                        |float         |__zero__|
|`huehigh`       |The maximum hue value                                        |float         |__one__ |
|`saturationlow` |The minimum saturation value                                 |float         |__zero__|
|`saturationhigh`|The maximum saturation value                                 |float         |__one__ |
|`brightnesslow` |The minimum brightness value                                 |float         |__zero__|
|`brightnesshigh`|The maximum brightness value                                 |float         |__one__ |
|`seed`          |Additional randomization seed                                |integer       |__zero__|
|`out`           |Output: the randomized RGB color value                       |color3        |__zero__|


### Procedural Node Notes

To scale or offset the input coordinates for `rampX` or `splitX` or any other node with a `texcoord` input, use a &lt;place2d> node, or a &lt;texcoord> or similar Geometric node processed by vector2 &lt;multiply>, &lt;rotate> and/or &lt;add> nodes, and connect to the node's `texcoord` input.



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

### `noise2d`
2D Perlin noise in 1, 2, 3 or 4 channels.

|Port       |Description                                              |Type                  |Default |
|-----------|---------------------------------------------------------|----------------------|--------|
|`amplitude`|The center-to-peak amplitude of the noise                |Same as `out` or float|__one__ |
|`pivot`    |The center value of the output noise                     |float                 |0.0     |
|`texcoord` |The 2D texture coordinate at which the noise is evaluated|vector2               |_UV0_   |
|`out`      |Output: the computed noise value                         |float, vectorN        |__zero__|

<a id="node-noise3d"> </a>

### `noise3d`
3D Perlin noise in 1, 2, 3 or 4 channels.

|Port       |Description                                    |Type                  |Default  |
|-----------|-----------------------------------------------|----------------------|---------|
|`amplitude`|The center-to-peak amplitude of the noise      |Same as `out` or float|__one__  |
|`pivot`    |The center value of the output noise           |float                 |0.0      |
|`position` |The 3D position at which the noise is evaluated|vector3               |_Pobject_|
|`out`      |Output: the computed noise value               |float, vectorN        |__zero__ |

<a id="node-fractal2d"> </a>

### `fractal2d`
Zero-centered 2D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 2D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.

|Port        |Description                                                    |Type                  |Default  |
|------------|---------------------------------------------------------------|----------------------|---------|
|`amplitude` |The center-to-peak amplitude of the noise                      |Same as `out` or float|__one__  |
|`octaves`   |The number of octaves of noise to be summed                    |integer               |3        |
|`lacunarity`|The exponential scale between successive octaves of noise      |float                 |2.0      |
|`diminish`  |The rate at which noise amplitude is diminished for each octave|float                 |0.5      |
|`texcoord`  |The 2D texture coordinate at which the noise is evaluated      |vector2               |_UV0_    |
|`out`       |Output: the computed noise value                               |float, vectorN        |__zero__ |

<a id="node-fractal3d"> </a>

### `fractal3d`
Zero-centered 3D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 3D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.

|Port        |Description                                                    |Type                  |Default  |
|------------|---------------------------------------------------------------|----------------------|---------|
|`amplitude` |The center-to-peak amplitude of the noise                      |Same as `out` or float|__one__  |
|`octaves`   |The number of octaves of noise to be summed                    |integer               |3        |
|`lacunarity`|The exponential scale between successive octaves of noise      |float                 |2.0      |
|`diminish`  |The rate at which noise amplitude is diminished for each octave|float                 |0.5      |
|`position`  |The 3D position at which the noise is evaluated                |vector3               |_Pobject_|
|`out`       |Output: the computed noise value                               |float, vectorN        |__zero__ |

<a id="node-cellnoise2d"> </a>

### `cellnoise2d`
2D cellular noise, 1 or 3 channels (type float or vector3).

|Port      |Description                                              |Type   |Default|
|----------|---------------------------------------------------------|-------|-------|
|`texcoord`|The 2D texture coordinate at which the noise is evaluated|vector2|_UV0_  |
|`out`     |Output: the computed noise value                         |float  |0.0    |

<a id="node-cellnoise3d"> </a>

### `cellnoise3d`
3D cellular noise, 1 or 3 channels (type float or vector3).

|Port      |Description                                    |Type   |Default  |
|----------|-----------------------------------------------|-------|---------|
|`position`|The 3D position at which the noise is evaluated|vector3|_Pobject_|
|`out`     |Output: the computed noise value               |float  |0.0      |

<a id="node-worleynoise2d"> </a>

### `worleynoise2d`
2D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features) values.

|Port      |Description                                              |Type                   |Default|Accepted Values        |
|----------|---------------------------------------------------------|-----------------------|-------|-----------------------|
|`texcoord`|The 2D texture coordinate at which the noise is evaluated|vector2                |_UV0_  |                       |
|`jitter`  |The amount to jitter the cell center position            |float                  |1.0    |                       |
|`style`   |The output style                                         |integer                |0      |0 (Distance), 1 (Solid)|
|`out`     |Output: the computed noise value                         |float, vector2, vector3|0.0    |                       |

<a id="node-worleynoise3d"> </a>

### `worleynoise3d`
3D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features) values.

|Port      |Description                                    |Type                   |Default  |Accepted Values        |
|----------|-----------------------------------------------|-----------------------|---------|-----------------------|
|`position`|The 3D position at which the noise is evaluated|vector3                |_Pobject_|                       |
|`jitter`  |The amount to jitter the cell center position  |float                  |1.0      |                       |
|`style`   |The output style                               |integer                |0        |0 (Distance), 1 (Solid)|
|`out`     |Output: the computed noise value               |float, vector2, vector3|__zero__ |                       |

<a id="node-unifiednoise2d"> </a>

### `unifiednoise2d`
A single node supporting 2D Perlin, Cell, Worley or Fractal noise in a unified interface.

|Port         |Description                                                                                |Type   |Default|Accepted Values        |
|-------------|-------------------------------------------------------------------------------------------|-------|-------|-----------------------|
|`texcoord`   |The 2D texture coordinate at which the noise is evaluated                                  |vector2|_UV0_  |                       |
|`freq`       |The noise frequency, with higher values producing smaller noise shapes.                    |vector2|1, 1   |                       |
|`offset`     |The amount to offset 2d space                                                              |vector2|0, 0   |                       |
|`jitter`     |The amount to jitter the cell center position                                              |float  |1      |                       |
|`outmin`     |The lowest output value                                                                    |float  |0      |                       |
|`outmax`     |The highest output value                                                                   |float  |1      |                       |
|`clampoutput`|If enabled the output is clamped between the min and max output values                     |boolean|true   |                       |
|`octaves`    |The number of octaves of noise to be summed                                                |integer|3      |                       |
|`lacunarity` |The exponential scale between successive octaves of noise                                  |float  |2      |                       |
|`diminish`   |The rate at which noise amplitude is diminished for each octave                            |float  |0.5    |                       |
|`type`       |The type of noise function to use.  One of 0 (Perlin), 1 (Cell), 2 (Worley), or 3 (Fractal)|integer|0      |                       |
|`style`      |The output style                                                                           |integer|0      |0 (Distance), 1 (Solid)|
|`out`        |Output: the computed noise value                                                           |float  |0.0    |                       |

<a id="node-unifiednoise3d"> </a>

### `unifiednoise3d`
A single node supporting 3D Perlin, Cell, Worley or Fractal noise in a unified interface.

|Port         |Description                                                                                |Type   |Default  |Accepted Values        |
|-------------|-------------------------------------------------------------------------------------------|-------|---------|-----------------------|
|`position`   |The 3D position at which the noise is evaluated                                            |vector3|_Pobject_|                       |
|`freq`       |The noise frequency, with higher values producing smaller noise shapes.                    |vector3|1, 1, 1  |                       |
|`offset`     |The amount to offset 3d space                                                              |vector3|0, 0, 0  |                       |
|`jitter`     |The amount to jitter the cell center position                                              |float  |1        |                       |
|`outmin`     |The lowest output value                                                                    |float  |0        |                       |
|`outmax`     |The highest output value                                                                   |float  |1        |                       |
|`clampoutput`|If enabled the output is clamped between the min and max output values                     |boolean|true     |                       |
|`octaves`    |The number of octaves of noise to be summed                                                |integer|3        |                       |
|`lacunarity` |The exponential scale between successive octaves of noise                                  |float  |2        |                       |
|`diminish`   |The rate at which noise amplitude is diminished for each octave                            |float  |0.5      |                       |
|`type`       |The type of noise function to use.  One of 0 (Perlin), 1 (Cell), 2 (Worley), or 3 (Fractal)|integer|0        |                       |
|`style`      |The output style                                                                           |integer|0        |0 (Distance), 1 (Solid)|
|`out`        |Output: the computed noise value                                                           |float  |0.0      |                       |


### Noise Node Notes

To scale or offset the noise pattern generated by a 3D noise node such as `noise3d`, `fractal3d` or `cellnoise3d`, use a &lt;position> or other [Geometric Node](#geometric-nodes) (see below) connected to vector3 &lt;multiply> and/or &lt;add> nodes, in turn connected to the noise node's `position` input.

To scale or offset the noise pattern generated by a 2D noise node such as `noise2d` or `cellnoise2d`, use a &lt;place2d> node, or a &lt;texcoord> or similar Geometric node processed by vector2 &lt;multiply>, &lt;rotate> and/or &lt;add> nodes, and connect to the node's `texcoord` input.



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

### `checkerboard`
2D checkerboard pattern.

|Port      |Description                                                                                          |Type   |Default      |
|----------|-----------------------------------------------------------------------------------------------------|-------|-------------|
|`color1`  |The first color used in the checkerboard pattern.                                                    |color3 |1.0, 1.0, 1.0|
|`color2`  |The second color used in the checkerboard pattern.                                                   |color3 |0.0, 0.0, 0.0|
|`uvtiling`|The tiling of the checkerboard pattern along each axis, with higher values producing smaller squares.|vector2|8, 8         |
|`uvoffset`|The offset of the checkerboard pattern along each axis                                               |vector2|0, 0         |
|`texcoord`|The input 2d space.                                                                                  |vector2|_UV0_        |
|`out`     |Output: the checkerboard pattern                                                                     |color3 |             |

<a id="node-line"> </a>

### `line`
2D line pattern: outputs 1 if texcoord is at less than `radius` distance from a line segment defined by `point1` and `point2`, otherwise outputs 0.

|Port      |Description                                                    |Type   |Default   |
|----------|---------------------------------------------------------------|-------|----------|
|`texcoord`|The input 2d space                                             |vector2|_UV0_     |
|`center`  |An offset value added to both the point1 and point2 coordinates|vector2|0, 0      |
|`radius`  |The radius or 'half thickness' of the line                     |float  |0.1       |
|`point1`  |The UV coordinate of the first endpoint                        |vector2|0.25, 0.25|
|`point2`  |The UV coordinate of the second endpoint                       |vector2|0.75, 0.75|
|`out`     |Output: the line pattern                                       |float  |          |

<a id="node-circle"> </a>

### `circle`
2D circle (disk) pattern: outputs 1 if texcoord is inside a circle defined by `center` and `radius`, otherwise outputs 0.

|Port      |Description                        |Type   |Default|
|----------|-----------------------------------|-------|-------|
|`texcoord`|The input 2d space                 |vector2|_UV0_  |
|`center`  |The center coordinate of the circle|vector2|0, 0   |
|`radius`  |The radius of the circle           |float  |0.5    |
|`out`     |Output: the circle pattern         |float  |       |

<a id="node-cloverleaf"> </a>

### `cloverleaf`
2D cloverleaf pattern: four semicircles on the edges of a square defined by `center` and `radius`.  Outputs 1 if texcoord is within the pattern, otherwise outputs 0.

|Port      |Description                            |Type   |Default|
|----------|---------------------------------------|-------|-------|
|`texcoord`|The input 2d space                     |vector2|_UV0_  |
|`center`  |The center coordinate of the cloverleaf|vector2|0, 0   |
|`radius`  |The radius of the cloverleaf           |float  |0.5    |
|`out`     |Output: the cloverleaf pattern         |float  |       |

<a id="node-hexagon"> </a>

### `hexagon`
2D hexagon pattern: outputs 1 if texcoord is inside a hexagon shape inscribed by a circle defined by `center` and `radius`; otherwise outputs 0.

|Port      |Description                         |Type   |Default|
|----------|------------------------------------|-------|-------|
|`texcoord`|The input 2d space                  |vector2|_UV0_  |
|`center`  |The center coordinate of the hexagon|vector2|0, 0   |
|`radius`  |The radius of the hexagon           |float  |0.5    |
|`out`     |Output: the hexagon pattern         |float  |       |

<a id="node-grid"> </a>

### `grid`
Creates a grid pattern of (1, 1, 1) white lines on a (0, 0, 0) black background with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

|Port       |Description                                                                  |Type   |Default |
|-----------|-----------------------------------------------------------------------------|-------|--------|
|`texcoord` |The input 2d space                                                           |vector2|_UV0_   |
|`uvtiling` |Tiling factor, with higher values producing a denser grid.                   |vector2|1.0, 1.0|
|`uvoffset` |UV Offset                                                                    |vector2|0.0, 0.0|
|`thickness`|The thickness of the grid lines                                              |float  |0.05    |
|`staggered`|If true, every other row will be offset 50% to produce a 'brick wall' pattern|boolean|false   |
|`out`      |Output: the grid pattern                                                     |color3 |        |

<a id="node-crosshatch"> </a>

### `crosshatch`
Creates a crosshatch pattern with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

|Port       |Description                                                                            |Type   |Default |
|-----------|---------------------------------------------------------------------------------------|-------|--------|
|`texcoord` |The input 2d space                                                                     |vector2|_UV0_   |
|`uvtiling` |Tiling factor, with higher values producing a denser grid.                             |vector2|1.0, 1.0|
|`uvoffset` |UV Offset                                                                              |vector2|0.0, 0.0|
|`thickness`|The thickness of the grid lines                                                        |float  |0.05    |
|`staggered`|If true, every other row will be offset 50% to produce an 'alternating diamond' pattern|boolean|false   |
|`out`      |Output: the crosshatch pattern                                                         |color3 |        |

<a id="node-tiledcircles"> </a>

### `tiledcircles`
Creates a black and white pattern of circles with a defined tiling and size (diameter). Pattern can be regular or staggered.

|Port       |Description                                                                                                                                                                         |Type   |Default |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|
|`texcoord` |The input 2d space                                                                                                                                                                  |vector2|_UV0_   |
|`uvtiling` |Tiling factor, with higher values producing a denser grid.                                                                                                                          |vector2|1.0, 1.0|
|`uvoffset` |UV Offset                                                                                                                                                                           |vector2|0.0, 0.0|
|`size`     |The diameter of the circles in the tiled pattern. If `size` is 1.0, the edges of adjacent circles in the tiling will exactly touch.                                                 |float  |0.5     |
|`staggered`|If true, every other row will be offset 50%, and the spacing of the tiling will be adjusted in the V direction to center the circles on the vertices of an equilateral triangle grid|boolean|false   |
|`out`      |Output: the tiled circle pattern                                                                                                                                                    |color3 |        |

<a id="node-tiledcloverleafs"> </a>

### `tiledcloverleafs`
Creates a black and white pattern of cloverleafs with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

|Port       |Description                                                                                                                                    |Type   |Default |
|-----------|-----------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|
|`texcoord` |The input 2d space                                                                                                                             |vector2|_UV0_   |
|`uvtiling` |Tiling factor, with higher values producing a denser grid.                                                                                     |vector2|1.0, 1.0|
|`uvoffset` |UV Offset                                                                                                                                      |vector2|0.0, 0.0|
|`size`     |The outer diameter of the cloverleafs in the tiled pattern. If size is 1.0, the edges of adjacent cloverleafs in the tiling will exactly touch.|float  |0.5     |
|`staggered`|If true, an additional pattern of cloverleafs will be generated in between the originals offset by 50% in both U and V                         |boolean|false   |
|`out`      |Output: the tiled cloverleaf pattern                                                                                                           |color3 |        |

<a id="node-tiledhexagons"> </a>

### `tiledhexagons`
Creates a black and white pattern of hexagons with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

|Port       |Description                                                                                                                                                                           |Type   |Default |
|-----------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|
|`texcoord` |The input 2d space                                                                                                                                                                    |vector2|_UV0_   |
|`uvtiling` |Tiling factor, with higher values producing a denser grid.                                                                                                                            |vector2|1.0, 1.0|
|`uvoffset` |UV Offset                                                                                                                                                                             |vector2|0.0, 0.0|
|`size`     |The inner diameter of the hexagons in the tiled pattern. if size is 1.0, the edges of adjacent hexagons in the U-direcction tiling will exactly touch                                 |float  |0.5     |
|`staggered`|If true, every other row will be offset 50%, and the spacing of the tiling will be adjusted in the V direction to center the hexagons on the vertices of an equilateral triangle grid.|boolean|false   |
|`out`      |Output: the tiled hexagon pattern                                                                                                                                                     |color3 |        |



## Geometric Nodes

Geometric nodes are used to reference local geometric properties from within a node graph:

```xml
  <position name="wp1" type="vector3">
    <input name="space" type="string" value="world"/>
  </position>
  <texcoord name="c1" type="vector2">
    <input name="index" type="integer" value="1"/>
  </texcoord>
```

Standard Geometric nodes:

<a id="node-position"> </a>

### `position`
The coordinates associated with the currently-processed data, as defined in a specific coordinate space.

|Port   |Description                                |Type   |Default      |Accepted Values     |
|-------|-------------------------------------------|-------|-------------|--------------------|
|`space`|The coordinate space of the output position|string |object       |model, object, world|
|`out`  |Output: the position in `space`            |vector3|0.0, 0.0, 0.0|                    |

<a id="node-normal"> </a>

### `normal`
The normalized geometric normal associated with the currently-processed data, as defined in a specific coordinate space.

|Port   |Description                                |Type   |Default      |Accepted Values     |
|-------|-------------------------------------------|-------|-------------|--------------------|
|`space`|The coordinate space of the output position|string |object       |model, object, world|
|`out`  |Output: the normal in `space`              |vector3|0.0, 1.0, 0.0|                    |

<a id="node-tangent"> </a>

### `tangent`
The geometric tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

|Port   |Description                                                                          |Type   |Default      |Accepted Values     |
|-------|-------------------------------------------------------------------------------------|-------|-------------|--------------------|
|`space`|The coordinate space of the output position                                          |string |object       |model, object, world|
|`index`|The index of the texcoord space to use to compute the tangent vector                 |integer|0            |                    |
|`out`  |Output: the tangent vector associated with the `index`th coordinate space, in `space`|vector3|1.0, 0.0, 0.0|                    |

<a id="node-bitangent"> </a>

### `bitangent`
The geometric bi-tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

|Port   |Description                                                                            |Type   |Default      |Accepted Values     |
|-------|---------------------------------------------------------------------------------------|-------|-------------|--------------------|
|`space`|The coordinate space of the output position                                            |string |object       |model, object, world|
|`index`|The index of the texcoord space to use to compute the bitangent vector                 |integer|0            |                    |
|`out`  |Output: the bitangent vector associated with the `index`th coordinate space, in `space`|vector3|0.0, 0.0, 1.0|                    |

<a id="node-bump"> </a>

### `bump`
The normalized normal computed by offsetting the surface world space position along its world space normal.

|Port       |Description                         |Type   |Default |
|-----------|------------------------------------|-------|--------|
|`height`   |Amount to offset the surface normal.|float  |0       |
|`scale`    |Scalar to adjust the height amount. |float  |1       |
|`normal`   |Surface normal                      |vector3|_Nworld_|
|`tangent`  |Surface tangent vector              |vector3|_Tworld_|
|`bitangent`|Surface bitangent vector            |vector3|_Bworld_|
|`out`      |Output: offset surface normal       |vector3|        |

<a id="node-texcoord"> </a>

### `texcoord`
The 2D or 3D texture coordinates associated with the currently-processed data

|Port   |Description                |Type            |Default |
|-------|---------------------------|----------------|--------|
|`index`|Texcoord index             |integer         |0       |
|`out`  |Output: texture coordinates|vector2, vector3|__zero__|

<a id="node-geomcolor"> </a>

### `geomcolor`
The color associated with the current geometry at the current position, generally bound via per-vertex color values. The type must match the type of the "color" bound to the geometry.

|Port   |Description                 |Type         |Default |
|-------|----------------------------|-------------|--------|
|`index`|index of the geometric color|integer      |0       |
|`out`  |Output: geometric color     |float, colorN|__zero__|

<a id="node-geompropvalue"> </a>

### `geompropvalue`
The value of the specified varying geometric property (defined by a &lt;geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

|Port      |Description                                                                         |Type                                    |Default  |
|----------|------------------------------------------------------------------------------------|----------------------------------------|---------|
|`geomprop`|The geometric property to be referenced                                             |string                                  |__empty__|
|`default` |A value to return if the specified `geomprop` is not defined on the current geometry|float, colorN, vectorN, boolean, integer|__zero__ |
|`out`     |Output: the `geomprop` value                                                        |Same as `default`                       |__zero__ |

<a id="node-geompropvalueuniform"> </a>

### `geompropvalueuniform`
The value of the specified uniform geometric property (defined by a &lt;geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

|Port      |Description                                                                                 |Type             |Default  |
|----------|--------------------------------------------------------------------------------------------|-----------------|---------|
|`geomprop`|The geometric property to be referenced                                                     |string           |__empty__|
|`default` |A value to return if the specified `geomprop` is not defined on the current geometry        |string, filename |__zero__ |
|`out`     |Output: A value to return if the specified `geomprop` is not defined on the current geometry|Same as `default`|__zero__ |


### Geometric Node Notes

A `colorspace` attribute may be specified for color3/color4-type properties of &lt;geomcolor> and &lt;geompropvalue> nodes to declare what colorspace the color property value is in; the default is "none" for no colorspace declaration (and hence no colorspace conversion).



## Application Nodes

Application nodes are used to reference application-defined properties within a node graph, and have no inputs:

```xml
  <frame name="f1" type="float"/>
  <time name="t1" type="float"/>
```

Standard Application nodes:

<a id="node-frame"> </a>

### `frame`
The current frame number as defined by the host environment.

|Port |Description                                            |Type |Default|
|-----|-------------------------------------------------------|-----|-------|
|`out`|Output: frame number as defined by the host environment|float|1.0    |

<a id="node-time"> </a>

### `time`
The current time in seconds, as defined by the host environment.

|Port |Description                                                       |Type |Default|
|-----|------------------------------------------------------------------|-----|-------|
|`out`|Output: current time in seconds as defined by the host environment|float|0.0    |


### Application Node Notes

Applications may use whatever method is appropriate to communicate the current frame number or time to the &lt;frame> or &lt;time> node's implementation, whether via an internal state variable, a custom input, dividing the current frame number by a local "frames per second" value (&lt;time> node only), or other method.  Real-time applications may return some variation of wall-clock time.


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

The inputs of compositing operators are called "fg" and "bg" (plus "alpha" for float and color3 variants, and "mix" for all variants of the `mix` operator), while the inputs of most other operators are called "in" if there is exactly one input, or "in1", "in2" etc. if there are more than one input.  If an implementation does not support a particular operator, it should generally pass through the "bg", "in" or "in1" input unchanged.

This section defines the Operator Nodes that all MaterialX implementations are expected to support.  Standard Operator Nodes are grouped into the following classifications: [Math Nodes](#math-nodes), [Adjustment Nodes](#adjustment-nodes), [Compositing Nodes](#compositing-nodes), [Conditional Nodes](#conditional-nodes), [Channel Nodes](#channel-nodes) and [Convolution Nodes](#convolution-nodes).



## Math Nodes

Math nodes have one or two spatially-varying inputs, and are used to perform a math operation on values in one spatially-varying input stream, or to combine two spatially-varying input streams using a specified math operation.  The given math operation is performed for each channel of the input stream(s), and the data type of each input must either match that of the input stream(s), or be a float value that will be applied to each channel separately.


<a id="node-add"> </a>

### `add`
Add a value to the incoming float/color/vector/matrix

|Port |Description                   |Type                           |Default |
|-----|------------------------------|-------------------------------|--------|
|`in1`|The primary input stream      |float, colorN, vectorN, integer|__zero__|
|`in2`|The stream to add to `in1`    |Same as `in1`                  |__zero__|
|`out`|Output: sum of `in1` and `in2`|Same as `in1`                  |`in1`   |

|Port |Description                   |Type           |Default |
|-----|------------------------------|---------------|--------|
|`in1`|The primary input stream      |colorN, vectorN|__zero__|
|`in2`|The stream to add to `in1`    |float          |__zero__|
|`out`|Output: sum of `in1` and `in2`|Same as `in1`  |`in1`   |

|Port |Description                   |Type                  |Default |
|-----|------------------------------|----------------------|--------|
|`in1`|The primary input stream      |matrixNN              |__one__ |
|`in2`|The stream to add to `in1`    |Same as `in1` or float|__zero__|
|`out`|Output: sum of `in1` and `in2`|Same as `in1`         |`in1`   |

<a id="node-subtract"> </a>

### `subtract`
Subtract a value from the incoming float/color/vector/matrix

|Port |Description                      |Type                           |Default |
|-----|---------------------------------|-------------------------------|--------|
|`in1`|The primary input stream         |float, colorN, vectorN, integer|__zero__|
|`in2`|The stream to subtract from `in1`|Same as `in1`                  |__zero__|
|`out`|Output: `in1` minus `in2`        |Same as `in1`                  |`in1`   |

|Port |Description                      |Type           |Default |
|-----|---------------------------------|---------------|--------|
|`in1`|The primary input stream         |colorN, vectorN|__zero__|
|`in2`|The stream to subtract from `in1`|float          |__zero__|
|`out`|Output: `in1` minus `in2`        |Same as `in1`  |`in1`   |

|Port |Description                      |Type                  |Default |
|-----|---------------------------------|----------------------|--------|
|`in1`|The primary input stream         |matrixNN              |__one__ |
|`in2`|The stream to subtract from `in1`|Same as `in1` or float|__zero__|
|`out`|Output: `in1` minus `in2`        |Same as `in1`         |`in1`   |

<a id="node-multiply"> </a>

### `multiply`
Multiply two values together. Scalar and vector types multiply component-wise, while matrices multiply using a standard matrix product.

|Port |Description                       |Type                  |Default |
|-----|----------------------------------|----------------------|--------|
|`in1`|The primary input stream          |float, colorN, vectorN|__zero__|
|`in2`|The stream to multiply `in1` by   |Same as `in1` or float|__one__ |
|`out`|Output: product of `in1` and `in2`|Same as `in1`         |`in1`   |

|Port |Description                       |Type         |Default|
|-----|----------------------------------|-------------|-------|
|`in1`|The primary input stream          |matrixNN     |__one__|
|`in2`|The stream to multiply `in1` by   |Same as `in1`|__one__|
|`out`|Output: product of `in1` and `in2`|Same as `in1`|`in1`  |

<a id="node-divide"> </a>

### `divide`
Divide one value by another. Scalar and vector types divide component-wise, while for matrices `in1` is multiplied by the inverse of `in2`.

|Port |Description                   |Type                  |Default |
|-----|------------------------------|----------------------|--------|
|`in1`|The primary input stream      |float, colorN, vectorN|__zero__|
|`in2`|The stream to divide `in1` by |Same as `in1` or float|__one__ |
|`out`|Output: `in1` divided by `in2`|Same as `in1`         |`in1`   |

|Port |Description                   |Type         |Default|
|-----|------------------------------|-------------|-------|
|`in1`|The primary input stream      |matrixNN     |__one__|
|`in2`|The stream to divide `in1` by |Same as `in1`|__one__|
|`out`|Output: `in1` divided by `in2`|Same as `in1`|`in1`  |

<a id="node-modulo"> </a>

### `modulo`
The remaining fraction after dividing an incoming float/color/vector by a value and subtracting the integer portion. Modulo always returns a non-negative result.

|Port |Description                  |Type                  |Default |Accepted Values|
|-----|-----------------------------|----------------------|--------|---------------|
|`in1`|The primary input stream     |float, colorN, vectorN|__zero__|               |
|`in2`|The stream to modulo `in1` by|Same as `in1` or float|__one__ |`in2` != 0     |
|`out`|Output: `in1` modulo `in2`   |Same as `in1`         |`in1`   |               |

<a id="node-fract"> </a>

### `fract`
Returns the fractional part of the floating-point input.

|Port |Description                        |Type                  |Default |
|-----|-----------------------------------|----------------------|--------|
|`in` |The primary input stream           |float, colorN, vectorN|__zero__|
|`out`|Output: The fractional part of `in`|Same as `in`          |`in`    |

<a id="node-invert"> </a>

### `invert`
Subtract the incoming float, color, or vector from `amount` in all channels, outputting: `amount - in`.

|Port    |Description                    |Type                  |Default |
|--------|-------------------------------|----------------------|--------|
|`in`    |The primary input stream       |float, colorN, vectorN|__zero__|
|`amount`|The value to subtract `in` from|Same as `in` or float |__one__ |
|`out`   |Output: `amount` minus `in`    |Same as `in`          |`in`    |

<a id="node-absval"> </a>

### `absval`
The per-channel absolute value of the incoming float/color/vector.

|Port |Description                   |Type                  |Default |
|-----|------------------------------|----------------------|--------|
|`in` |The primary input stream      |float, colorN, vectorN|__zero__|
|`out`|Output: absolute value of `in`|Same as `in`          |`in`    |

<a id="node-sign"> </a>

### `sign`
The per-channel sign of the incoming float/color/vector value: -1 for negative, +1 for positive, or 0 for zero.

|Port |Description             |Type                  |Default |
|-----|------------------------|----------------------|--------|
|`in` |The primary input stream|float, colorN, vectorN|__zero__|
|`out`|Output: sign of `in`    |Same as `in`          |`in`    |

<a id="node-floor"> </a>

### `floor`
The per-channel nearest integer value less than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that &lt;floor> of a float input also has a variant outputting an integer type.

|Port |Description                                       |Type                  |Default |
|-----|--------------------------------------------------|----------------------|--------|
|`in` |The primary input stream                          |float, colorN, vectorN|__zero__|
|`out`|Output: nearest integer less than or equal to `in`|Same as `in`          |`in`    |

|Port |Description                                       |Type   |Default|
|-----|--------------------------------------------------|-------|-------|
|`in` |The primary input stream                          |float  |0.0    |
|`out`|Output: nearest integer less than or equal to `in`|integer|`in`   |

<a id="node-ceil"> </a>

### `ceil`
The per-channel nearest integer value greater than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that &lt;ceil> of a float input also has a variant outputting an integer type.

|Port |Description                                          |Type                  |Default |
|-----|-----------------------------------------------------|----------------------|--------|
|`in` |The primary input stream                             |float, colorN, vectorN|__zero__|
|`out`|Output: nearest integer greater than or equal to `in`|Same as `in`          |`in`    |

|Port |Description                                          |Type   |Default|
|-----|-----------------------------------------------------|-------|-------|
|`in` |The primary input stream                             |float  |0.0    |
|`out`|Output: nearest integer greater than or equal to `in`|integer|`in`   |

<a id="node-round"> </a>

### `round`
Round each channel of the incoming float/color/vector values to the nearest integer value.

|Port |Description                                |Type                  |Default |
|-----|-------------------------------------------|----------------------|--------|
|`in` |The primary input stream                   |float, colorN, vectorN|__zero__|
|`out`|Output: `in` rounded to the nearest integer|Same as `in`          |`in`    |

|Port |Description                                |Type   |Default|
|-----|-------------------------------------------|-------|-------|
|`in` |The primary input stream                   |float  |0.0    |
|`out`|Output: `in` rounded to the nearest integer|integer|`in`   |

<a id="node-power"> </a>

### `power`
Raise incoming float/color values to the specified exponent, commonly used for "gamma" adjustment.

|Port |Description                   |Type                  |Default |
|-----|------------------------------|----------------------|--------|
|`in1`|The primary input stream      |float, colorN, vectorN|__zero__|
|`in2`|The exponent to raise `in1` to|Same as `in1` or float|__one__ |
|`out`|Output: `in1` raised to `in2` |Same as `in1`         |`in1`   |

<a id="node-safepower"> </a>

### `safepower`
Raise incoming float/color values to the specified exponent. Negative "in1" values will result in negative output values.

|Port |Description                   |Type                  |Default |
|-----|------------------------------|----------------------|--------|
|`in1`|The primary input stream      |float, colorN, vectorN|__zero__|
|`in2`|The exponent to raise `in1` to|Same as `in1` or float|__one__ |
|`out`|Output: `in1` raised to `in2` |Same as `in1`         |`in1`   |

<a id="node-sin"> </a>

### `sin`
The sine of the incoming value, which is expected to be expressed in radians.

|Port |Description             |Type          |Default |
|-----|------------------------|--------------|--------|
|`in` |The primary input stream|float, vectorN|__zero__|
|`out`|Output: sin of `in`     |Same as `in`  |`in`    |

<a id="node-cos"> </a>

### `cos`
The cosine of the incoming value, which is expected to be expressed in radians.

|Port |Description             |Type          |Default |
|-----|------------------------|--------------|--------|
|`in` |The primary input stream|float, vectorN|__zero__|
|`out`|Output: cos of `in`     |Same as `in`  |`in`    |

<a id="node-tan"> </a>

### `tan`
The tangent of the incoming value, which is expected to be expressed in radians.

|Port |Description             |Type          |Default |
|-----|------------------------|--------------|--------|
|`in` |The primary input stream|float, vectorN|__zero__|
|`out`|Output: tan of `in`     |Same as `in`  |`in`    |

<a id="node-asin"> </a>

### `asin`
The arcsine of the incoming value. The output will be expressed in radians.

|Port |Description             |Type          |Default |Accepted Values    |
|-----|------------------------|--------------|--------|-------------------|
|`in` |The primary input stream|float, vectorN|__zero__|[-__one__, __one__]|
|`out`|Output: asin of `in`    |Same as `in`  |`in`    |                   |

<a id="node-acos"> </a>

### `acos`
The arccosine of the incoming value. The output will be expressed in radians.

|Port |Description             |Type          |Default |Accepted Values    |
|-----|------------------------|--------------|--------|-------------------|
|`in` |The primary input stream|float, vectorN|__zero__|[-__one__, __one__]|
|`out`|Output: acos of `in`    |Same as `in`  |`in`    |                   |

<a id="node-atan2"> </a>

### `atan2`
The arctangent of the expression (`iny`/`inx`). The output will be expressed in radians.

|Port |Description                                                                                     |Type          |Default |
|-----|------------------------------------------------------------------------------------------------|--------------|--------|
|`iny`|Vertical component of the point to which the the angle is to be found                           |float, vectorN|__zero__|
|`inx`|Horizontal component of the point to which the angle is to be found                             |Same as `iny` |__one__ |
|`out`|Output: angle relative to the X-axis of the line joining the origin and the point (`inx`, `iny`)|Same as `iny` |`iny`   |

<a id="node-sqrt"> </a>

### `sqrt`
The square root of the incoming value. 

|Port |Description                |Type          |Default |Accepted Values     |
|-----|---------------------------|--------------|--------|--------------------|
|`in` |The primary input stream   |float, vectorN|__zero__|[__zero__, __+inf__)|
|`out`|Output: square root of `in`|Same as `in`  |`in`    |                    |

<a id="node-ln"> </a>

### `ln`
The natural logarithm of the incoming value. 

|Port |Description                      |Type          |Default|Accepted Values       |
|-----|---------------------------------|--------------|-------|----------------------|
|`in` |The primary input stream         |float, vectorN|__one__| (__zero__, __+inf__) |
|`out`|Output: natural logarithm of `in`|Same as `in`  |`in`   |                      |

<a id="node-exp"> </a>

### `exp`
$e$ to the power of the incoming value.

|Port |Description                |Type          |Default |
|-----|---------------------------|--------------|--------|
|`in` |The primary input stream   |float, vectorN|__zero__|
|`out`|Output: exponential of `in`|Same as `in`  |`in`    |

<a id="node-clamp"> </a>

### `clamp`
Clamp incoming values per-channel to a specified range of float/color/vector values.

|Port  |Description                                                       |Type                  |Default |
|------|------------------------------------------------------------------|----------------------|--------|
|`in`  |The input stream to be clamped                                    |float, colorN, vectorN|__zero__|
|`low` |Any value of `in` lower than this value will be set to this value |Same as `in` or float |__zero__|
|`high`|Any value of `in` higher than this value will be set to this value|Same as `low`         |__one__ |
|`out` |Output: clamped `in`                                              |Same as `in`          |`in`    |

<a id="node-trianglewave"> </a>

### `trianglewave`
Generate a triangle wave from the given scalar input. The generated wave ranges from zero to one and repeats on integer boundaries.

|Port |Description                  |Type |Default|
|-----|-----------------------------|-----|-------|
|`in` |The primary input stream     |float|0      |
|`out`|Output: generated wave signal|float|       |

<a id="node-min"> </a>

### `min`
Select the minimum of the two incoming values.

|Port |Description                       |Type                  |Default |
|-----|----------------------------------|----------------------|--------|
|`in1`|The first input stream            |float, colorN, vectorN|__zero__|
|`in2`|The second input stream           |Same as `in1` or float|__zero__|
|`out`|Output: minimum of `in1` and `in2`|Same as `in1`         |`in1`   |

<a id="node-max"> </a>

### `max`
Select the maximum of the two incoming values.

|Port |Description                       |Type                  |Default |
|-----|----------------------------------|----------------------|--------|
|`in1`|The first input stream            |float, colorN, vectorN|__zero__|
|`in2`|The second input stream           |Same as `in1` or float|__zero__|
|`out`|Output: maximum of `in1` and `in2`|Same as `in1`         |`in1`   |

<a id="node-normalize"> </a>

### `normalize`
Output the incoming vectorN stream normalized. 

|Port |Description            |Type        |Default |
|-----|-----------------------|------------|--------|
|`in` |Vector to be normalized|vectorN     |__zero__|
|`out`|Output: normalized `in`|Same as `in`|`in`    |

<a id="node-magnitude"> </a>

### `magnitude`
Output the float magnitude (vector length) of the incoming vectorN stream; cannot be used on float or colorN streams. Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.

|Port |Description              |Type   |Default |
|-----|-------------------------|-------|--------|
|`in` |Input vector stream      |vectorN|__zero__|
|`out`|Output: magnitude of `in`|float  |0.0     |

<a id="node-distance"> </a>

### `distance`
Measures the distance between two points in 2D, 3D, or 4D.

|Port |Description                             |Type         |Default |
|-----|----------------------------------------|-------------|--------|
|`in1`|Point to calculate distance from        |vectorN      |__zero__|
|`in2`|Point to calculate distance to          |Same as `in1`|__zero__|
|`out`|Output: distance between `in1` and `in2`|float        |        |

<a id="node-dotproduct"> </a>

### `dotproduct`
Output the (float) dot product of two incoming vectorN streams; cannot be used on float or colorN streams.

|Port |Description                           |Type         |Default |
|-----|--------------------------------------|-------------|--------|
|`in1`|The first input vector stream         |vectorN      |__zero__|
|`in2`|The second input vector stream        |Same as `in1`|__zero__|
|`out`|Output: dot product of `in1` and `in2`|float        |0.0     |

<a id="node-crossproduct"> </a>

### `crossproduct`
Output the (vector3) cross product of two incoming vector3 streams; cannot be used on any other stream type. A disabled crossproduct node passes through the value of `in1` unchanged.

|Port |Description                             |Type   |Default      |
|-----|----------------------------------------|-------|-------------|
|`in1`|The first input vector stream           |vector3|0.0, 0.0, 0.0|
|`in2`|The second input vector stream          |vector3|0.0, 0.0, 0.0|
|`out`|Output: cross product of `in1` and `in2`|vector3|`in1`        |

<a id="node-transformpoint"> </a>

### `transformpoint`
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

|Port       |Description                                                                                                                                          |Type   |Default      |
|-----------|-----------------------------------------------------------------------------------------------------------------------------------------------------|-------|-------------|
|`in`       |The point to be transformed                                                                                                                          |vector3|0.0, 0.0, 0.0|
|`fromspace`|The name of a vector space understood by the rendering target to transform the `in` point from; may be empty to specify the renderer's working space.|string |__empty__    |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` point to.                                          |string |__empty__    |
|`out`      |Output: point transformed from `fromspace` to `tospace`                                                                                              |vector3|`in`         |

<a id="node-transformvector"> </a>

### `transformvector`
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

|Port       |Description                                                                                                                                           |Type   |Default      |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------|-------|-------------|
|`in`       |The vector to be transformed                                                                                                                          |vector3|0.0, 0.0, 0.0|
|`fromspace`|The name of a vector space understood by the rendering target to transform the `in` vector from; may be empty to specify the renderer's working space.|string |__empty__    |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` vector to.                                          |string |__empty__    |
|`out`      |Output: point transformed from `fromspace` to `tospace`                                                                                               |vector3|`in`         |

<a id="node-transformnormal"> </a>

### `transformnormal`
Transform the incoming vector3 normal from one specified space to another; cannot be used on any other stream type.

|Port       |Description                                                                                                                                           |Type   |Default      |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------|-------|-------------|
|`in`       |The normal to be transformed                                                                                                                          |vector3|0.0, 0.0, 1.0|
|`fromspace`|The name of a vector space understood by the rendering target to transform the `in` normal from; may be empty to specify the renderer's working space.|string |__empty__    |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` normal to.                                          |string |__empty__    |
|`out`      |Output: point transformed from `fromspace` to `tospace`                                                                                               |vector3|`in`         |

<a id="node-transformmatrix"> </a>

### `transformmatrix`
Transform the incoming vectorN by the specified matrix.

|Port |Description                      |Type    |Default |
|-----|---------------------------------|--------|--------|
|`in` |Vector to be transformed         |vector2 |__zero__|
|`mat`|Matrix to transform `in` by      |matrix33|__one__ |
|`out`|Output: `in` transformed by `mat`|vector2 |`in`    |

|Port |Description                      |Type    |Default |
|-----|---------------------------------|--------|--------|
|`in` |Vector to be transformed         |vector3 |__zero__|
|`mat`|Matrix to transform `in` by      |matrixNN|__one__ |
|`out`|Output: `in` transformed by `mat`|vector3 |`in`    |

|Port |Description                      |Type    |Default |
|-----|---------------------------------|--------|--------|
|`in` |Vector to be transformed         |vector4 |__zero__|
|`mat`|Matrix to transform `in` by      |matrix44|__one__ |
|`out`|Output: `in` transformed by `mat`|vector4 |`in`    |

<a id="node-normalmap"> </a>

### `normalmap`
Transform a normal vector from the encoded tangent space to world space. The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.

|Port       |Description                                                                     |Type          |Default      |
|-----------|--------------------------------------------------------------------------------|--------------|-------------|
|`in`       |Input normal in space of frame defined by `normal`, `tangent`, `bitangent` ports|vector3       |0.5, 0.5, 1.0|
|`scale`    |Scaling factor to apply to the normal                                           |float, vector2|__one__      |
|`normal`   |Normal of the local frame from which to transform `in`                          |vector3       |_Nworld_     |
|`tangent`  |Tangent of the local frame from which to transform `in`                         |vector3       |_Tworld_     |
|`bitangent`|Bitangent of the local frame from which to transform `in`                       |vector3       |_Bworld_     |
|`out`      |Output: Ouptut normal in world space                                            |vector3       |`normal`     |

<a id="node-hextilednormalmap"> </a>

### `hextilednormalmap`
Samples data from a single normalmap, with provisions for hex-tiling and randomizing the normalmap across UV space.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

The `strength` input controls how strongly the sampled normal map affects the final normal. A value of 0.0 leaves the surface normal unchanged, 1.0 applies the sampled normal at full strength, and values >1.0 amplify the normal perturbation.

|Port                |Description                                                                                         |Type    |Default      |
|--------------------|----------------------------------------------------------------------------------------------------|--------|-------------|
|`file`              |The URI of the image file                                                                           |filename|__empty__    |
|`default`           |A default value to use if the file reference can not be resolved                                    |vector3 |0.5, 0.5, 1.0|
|`texcoord`          |The 2D texture coordinate at which the image data is read                                           |vector2 |_UV0_        |
|`tiling`            |The tiling rate for the given image along the U and V axes                                          |vector2 |1.0, 1.0     |
|`rotation`          |Per-tile rotation randomness in degrees                                                             |float   |0.0          |
|`rotationrange`     |Range in degrees used to randomize rotation for each tile                                           |vector2 |0.0, 360.0   |
|`scale`             |Per-tile scale randomness multiplier applied to tile size                                           |float   |1.0          |
|`scalerange`        |Range of scale multipliers used to randomize tile scale                                             |vector2 |0.5, 2.0     |
|`offset`            |Per-tile translation randomness in UV units                                                         |float   |1.0          |
|`offsetrange`       |Range of offset values in UV units used to randomize tile positions                                 |vector2 |0.0, 1.0     |
|`falloff`           |Falloff width used to blend neighboring tiles at their edges; larger values produce smoother blends |float   |0.5          |
|`strength`          |Controls how strongly the sampled normal map affects the final normal.                              |float   |1.0          |
|`flip_g`            |If true, negate the green channel of the sampled normal map to accommodate tangent-space conventions|boolean |false        |
|`normal`            |Surface normal                                                                                      |vector3 |_Nworld_     |
|`tangent`           |Surface tangent vector                                                                              |vector3 |_Tworld_     |
|`bitangent`         |Surface bitangent vector                                                                            |vector3 |_Bworld_     |
|`out`               |Output: the sampled normal map value                                                                |vector3 |0.5, 0.5, 1.0|

<a id="node-creatematrix"> </a>

### `creatematrix`
Build a 3x3 or 4x4 matrix from three vector3 or four vector3 or vector4 inputs. A matrix44 may also be created from vector3 input values, in which case the fourth value will be set to 0.0 for `in1`-`in3`, and to 1.0 for `in4` when creating the matrix44.

|Port |Description                   |Type    |Default      |
|-----|------------------------------|--------|-------------|
|`in1`|The first row of `out`        |vector3 |1.0, 0.0, 0.0|
|`in2`|The second row of `out`       |vector3 |0.0, 1.0, 0.0|
|`in3`|The third row of `out`        |vector3 |0.0, 0.0, 1.0|
|`out`|Output: the constructed matrix|matrix33|__one__      |

|Port |Description                             |Type    |Default      |
|-----|----------------------------------------|--------|-------------|
|`in1`|The first row of `out`, appended with 0 |vector3 |1.0, 0.0, 0.0|
|`in2`|The second row of `out`, appended with 0|vector3 |0.0, 1.0, 0.0|
|`in3`|The third row of `out`, appended with 0 |vector3 |0.0, 0.0, 1.0|
|`in4`|The fourth row of `out`, appended with 1|vector3 |0.0, 0.0, 0.0|
|`out`|Output: the constructed matrix          |matrix44|__one__      |

|Port |Description                   |Type    |Default           |
|-----|------------------------------|--------|------------------|
|`in1`|The first row of `out`        |vector4 |1.0, 0.0, 0.0, 0.0|
|`in2`|The second row of `out`       |vector4 |0.0, 1.0, 0.0, 0.0|
|`in3`|The third row of `out`        |vector4 |0.0, 0.0, 1.0, 0.0|
|`in4`|The fourth row of `out`       |vector4 |0.0, 0.0, 0.0, 1.0|
|`out`|Output: the constructed matrix|matrix44|__one__           |

<a id="node-transpose"> </a>

### `transpose`
Transpose the incoming matrix.

|Port |Description              |Type        |Default|
|-----|-------------------------|------------|-------|
|`in` |The input matrix         |matrixNN    |__one__|
|`out`|Output: transpose of `in`|Same as `in`|`in`   |

<a id="node-determinant"> </a>

### `determinant`
Output the determinant of the incoming matrix.

|Port |Description                |Type    |Default|
|-----|---------------------------|--------|-------|
|`in` |The input matrix           |matrixNN|__one__|
|`out`|Output: determinant of `in`|float   |1.0    |

<a id="node-invertmatrix"> </a>

### `invertmatrix`
Invert the incoming matrix.

|Port |Description            |Type        |Default|
|-----|-----------------------|------------|-------|
|`in` |The input matrix       |matrixNN    |__one__|
|`out`|Output: inverse of `in`|Same as `in`|`in`   |

<a id="node-rotate2d"> </a>

### `rotate2d`
Rotate the incoming 2D vector about the origin.

|Port    |Description                                                                        |Type   |Default |
|--------|-----------------------------------------------------------------------------------|-------|--------|
|`in`    |The input vector to rotate                                                         |vector2|0.0, 0.0|
|`amount`|The angle to rotate, specified in degrees. Positive values rotate counter-clockwise|float  |0.0     |
|`out`   |Output: rotated vector                                                             |vector2|`in`    |

<a id="node-rotate3d"> </a>

### `rotate3d`
Rotate the incoming 3D vector about the specified unit axis vector.

|Port    |Description                                                                        |Type   |Default      |
|--------|-----------------------------------------------------------------------------------|-------|-------------|
|`in`    |The input vector to rotate                                                         |vector3|0.0, 0.0, 0.0|
|`amount`|The angle to rotate, specified in degrees. Positive values rotate counter-clockwise|float  |0.0          |
|`axis`  |The unit axis vector to rotate `in` around                                         |vector3|0.0, 1.0, 0.0|
|`out`   |Output: rotated vector                                                             |vector3|`in`         |

<a id="node-reflect"> </a>

### `reflect`
Reflect the incoming 3D vector about a surface normal vector.

|Port    |Description                                             |Type   |Default      |
|--------|--------------------------------------------------------|-------|-------------|
|`in`    |Input vector to reflect                                 |vector3|1.0, 0.0, 0.0|
|`normal`|Vector normal to the surface about which to reflect `in`|vector3|_Nworld_     |
|`out`   |Output: reflection of `in` about `normal`               |vector3|             |

<a id="node-refract"> </a>

### `refract`
Refract the incoming 3D vector through a surface with the given surface normal and relative index of refraction.

|Port    |Description                                                                    |Type   |Default      |
|--------|-------------------------------------------------------------------------------|-------|-------------|
|`in`    |Input vector to refract                                                        |vector3|1.0, 0.0, 0.0|
|`normal`|Vector normal to the surface through which to refract `in`                     |vector3|_Nworld_     |
|`ior`   |The relative index of refraction of the interior of the surface to the exterior|float  |1.0          |
|`out`   |Output: refraction of `in` through `normal`                                    |vector3|             |

<a id="node-place2d"> </a>

### `place2d`
Transform incoming 2D texture coordinates from one frame of reference to another.

The `operationorder` input controls the order in which transform operations are performed. The `SRT` option performs -pivot, scale, rotate, translate, +pivot. The `TRS` option performs -pivot, translate, rotate, scale, +pivot.

|Port            |Description                                            |Type   |Default   |Accepted Values |
|----------------|-------------------------------------------------------|-------|----------|----------------|
|`texcoord`      |Input texture coordinates to transform                 |vector2|0.0, 0.0  |                |
|`pivot`         |Pivot point around which to rotate and scale `texcoord`|vector2|0.0,0.0   |                |
|`scale`         |Scaling factor to apply to `in`                        |vector2|1.0,1.0   |                |
|`rotate`        |Amount to rotate `in`, in degrees                      |float  |0.0       |                |
|`offset`        |Amount to translate `in`                               |vector2|0.0,0.0   |                |
|`operationorder`|The order in which transform operations are performed  |integer|0         |0 (SRT), 1 (TRS)|
|`out`           |Output: transformed texture coordinates                |vector2|`texcoord`|                |

<a id="node-dot"> </a>

### `dot`
A no-op, which passes its input through to its output unchanged.

Users can use dot nodes to shape edge connection paths or provide documentation checkpoints in node graph layout UI's. Dot nodes may also pass uniform values from `constant` or other nodes with uniform="true" outputs to uniform inputs and tokens.

|Port |Description                       |Type                                                                |Default |
|-----|----------------------------------|--------------------------------------------------------------------|--------|
|`in` |The input data stream             |float, colorN, vectorN, matrixNN, boolean, integer, string, filename|__zero__|
|`out`|Output: the unchanged input stream|Same as `in`                                                        |__zero__|


## Logical Operator Nodes

Logical operator nodes have one or two boolean typed inputs, and are used to construct higher level logical flow through the nodegraph.

<a id="node-and"> </a>

### `and`
Logically AND the two input boolean values.

|Port |Description            |Type   |Default|
|-----|-----------------------|-------|-------|
|`in1`|The first input stream |boolean|false  |
|`in2`|The second input stream|boolean|false  |
|`out`|Output: `in1` AND `in2`|boolean|`in1`  |

<a id="node-or"> </a>

### `or`
Logically Inclusive OR the two input boolean values.

|Port |Description            |Type   |Default|
|-----|-----------------------|-------|-------|
|`in1`|The first input stream |boolean|false  |
|`in2`|The second input stream|boolean|false  |
|`out`|Output: `in1` OR `in2` |boolean|`in1`  |

<a id="node-xor"> </a>

### `xor`
Logically Exclusive OR the two input boolean values.

|Port |Description            |Type   |Default|
|-----|-----------------------|-------|-------|
|`in1`|The first input stream |boolean|false  |
|`in2`|The second input stream|boolean|false  |
|`out`|Output: `in1` XOR `in2`|boolean|`in1`  |

<a id="node-not"> </a>

### `not`
Logically NOT the input boolean value.

|Port |Description     |Type   |Default|
|-----|----------------|-------|-------|
|`in` |The input stream|boolean|false  |
|`out`|Output: NOT `in`|boolean|true   |


## Adjustment Nodes

Adjustment nodes have one input named "in", and apply a specified function to values in the incoming stream.

<a id="node-contrast"> </a>

### `contrast`
Increase or decrease the contrast of the incoming `in` values using `amount` as a linear slope multiplier.

|Port    |Description                                                                                                                     |Type                  |Default |Accepted Values     |
|--------|--------------------------------------------------------------------------------------------------------------------------------|----------------------|--------|--------------------|
|`in`    |The input color stream to be adjusted                                                                                           |float, colorN, vectorN|__zero__|                    |
|`amount`|Slope multiplier for contrast adjustment. Values greater than 1.0 increase contrast, values between 0.0 and 1.0 reduce contrast.|Same as `in` or float |__one__ |[__zero__, __+inf__)|
|`pivot` |Center pivot value of contrast adjustment; this is the value that will not change as contrast is adjusted.                      |Same as `amount`      |__half__|                    |
|`out`   |Output: the adjusted color value                                                                                                |Same as `in`          |`in`    |                    |

<a id="node-remap"> </a>

### `remap`
Linearly remap incoming values from one range of values [`inlow`, `inhigh`] to another [`outlow`, `outhigh`].

|Port     |Description                    |Type                  |Default |
|---------|-------------------------------|----------------------|--------|
|`in`     |The input stream to be adjusted|float, colorN, vectorN|__zero__|
|`inlow`  |Low value for the input range  |Same as `in` or float |__zero__|
|`inhigh` |High value for the input range |Same as `inlow`       |__one__ |
|`outlow` |Low value for the output range |Same as `inlow`       |__zero__|
|`outhigh`|High value for the output range|Same as `inlow`       |__one__ |
|`out`    |Output: the adjusted value     |Same as `in`          |`in`    |

<a id="node-range"> </a>

### `range`
Remap incoming values from one range of values to another, optionally applying a gamma correction "in the middle". 

|Port     |Description                                             |Type                  |Default |
|---------|--------------------------------------------------------|----------------------|--------|
|`in`     |The input stream to be adjusted                         |float, colorN, vectorN|__zero__|
|`inlow`  |Low value for the input range                           |Same as `in` or float |__zero__|
|`inhigh` |High value for the input range                          |Same as `inlow`       |__one__ |
|`gamma`  |Reciprocal of the exponent applied to the remapped input|Same as `inlow`       |__one__ |
|`outlow` |Low value for the output range                          |Same as `inlow`       |__zero__|
|`outhigh`|High value for the output range                         |Same as `inlow`       |__one__ |
|`doclamp`|If true, the output is clamped to [`outlow`, `outhigh`] |boolean               |false   |
|`out`    |Output: the adjusted value                              |Same as `in`          |`in`    |

<a id="node-smoothstep"> </a>

### `smoothstep`
Output a smooth, hermite-interpolated remapping of input values from [`low`, `high`] to [0,1].

|Port  |Description                               |Type                  |Default |
|------|------------------------------------------|----------------------|--------|
|`in`  |The input value to the smoothstep function|float, colorN, vectorN|__zero__|
|`low` |Low value for the input range             |Same as `in` or float |__zero__|
|`high`|Low value for the output range            |Same as `low`         |__one__ |
|`out` |Output: the adjusted value                |Same as `in`          |`in`    |

<a id="node-luminance"> </a>

### `luminance`
Output a grayscale value containing the luminance of the incoming RGB color in all color channels.

The `lumacoeffs` input represents the luma coefficients of the current working color space. If no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.2722287, 0.6740818, 0.0536895] will be used. Applications which support color management systems may choose to retrieve the luma coefficients of the working colorspace from the CMS to pass to the &lt;luminance> node's implementation directly, rather than exposing it to the user.

|Port        |Description                                             |Type        |Default                        |
|------------|--------------------------------------------------------|------------|-------------------------------|
|`in`        |The input color stream to be converted                  |colorN      |__zero__                       |
|`lumacoeffs`|The luma coefficients of the current working color space|color3      |0.2722287, 0.6740818, 0.0536895|
|`out`       |Output: the luminance of `in`                           |Same as `in`|`in`                           |

<a id="node-rgbtohsv"> </a>

### `rgbtohsv`
Convert an incoming color from RGB to HSV space (with H and S ranging from 0 to 1); the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

|Port |Description                           |Type        |Default |
|-----|--------------------------------------|------------|--------|
|`in` |The input color stream to be converted|colorN      |__zero__|
|`out`|Output: `in` converted from RGB to HSV|Same as `in`|`in`    |

<a id="node-hsvtorgb"> </a>

### `hsvtorgb`
Convert an incoming color from HSV to RGB space; the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

|Port |Description                           |Type        |Default |
|-----|--------------------------------------|------------|--------|
|`in` |The input color stream to be converted|colorN      |__zero__|
|`out`|Output: `in` converted from HSV to RGB|Same as `in`|`in`    |

<a id="node-hsvadjust"> </a>

### `hsvadjust`
Adjust the hue, saturation and value of an RGB color by converting the input color to HSV, adding `amount.x` to the hue, multiplying the saturation by `amount.y`, multiplying the value by `amount.z`, then converting back to RGB.

Positive `amount.x` values rotate hue in the "red to green to blue" direction, with `amount.x` of 1.0 being the equivalent to a 360 degree (e.g. no-op) rotation.  Negative or greater-than-1.0 hue adjustment values are allowed, wrapping at the 0-1 boundaries.  The internal conversions between RGB and HSV spaces are not affected by the current color space, and for color4 inputs, the alpha value is left unchanged.

|Port    |Description                                                                 |Type        |Default      |
|--------|----------------------------------------------------------------------------|------------|-------------|
|`in`    |The input color stream to be adjusted                                       |colorN      |__zero__     |
|`amount`|Hue offset, saturation scale, and luminance scale in (x, y, z), respectively|vector3     |0.0, 1.0, 1.0|
|`out`   |Output: the adjusted value                                                  |Same as `in`|`in`         |

<a id="node-saturate"> </a>

### `saturate`
Adjust the saturation of a color, the alpha channel will be unchanged if present.

The `lumacoeffs` input represents the luma coefficients of the current working color space. If no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.2722287, 0.6740818, 0.0536895] will be used. Applications which support color management systems may choose to retrieve the luma coefficients of the working colorspace from the CMS to pass to the node's implementation directly, rather than exposing it to the user.

|Port        |Description                                                    |Type        |Default                        |
|------------|---------------------------------------------------------------|------------|-------------------------------|
|`in`        |The input color stream to be adjusted                          |colorN      |__zero__                       |
|`amount`    |Multiplier on the saturation `in`                              |float       |1.0                            |
|`lumacoeffs`|The luma coefficients to use to calculate the desaturated value|color3      |0.2722287, 0.6740818, 0.0536895|
|`out`       |Output: the adjusted value                                     |Same as `in`|`in`                           |

<a id="node-colorcorrect"> </a>

### `colorcorrect`
Combines various adjustment nodes into one artist-friendly color correction node. For color4 inputs, the alpha value is unchanged.

|Port           |Description                                                              |Type        |Default|
|---------------|-------------------------------------------------------------------------|------------|-------|
|`in`           |The input color stream                                                   |colorN      |__one__|
|`hue`          |Rotates the color hue                                                    |float       |0      |
|`saturation`   |Multiplies the input color saturation level                              |float       |1      |
|`gamma`        |Applies a gamma correction to the color                                  |float       |1      |
|`lift`         |Raises the dark color values, leaving the white values unchanged         |float       |0      |
|`gain`         |Multiplier increases lighter color values, leaving black values unchanged|float       |1      |
|`contrast`     |Linearly increase or decrease the color contrast                         |float       |1      |
|`contrastpivot`|Pivot value around which contrast applies                                |float       |0.5    |
|`exposure`     |Logarithmic brightness multiplier as 2^`exposure`                        |float       |0      |
|`out`          |Output: the color-corrected value                                        |Same as `in`|       |


## Compositing Nodes

Compositing nodes have two (required) inputs named `fg` and `bg`, and apply a function to combine them.  Compositing nodes are split into five subclassifications: [Premult Nodes](#premult-nodes), [Blend Nodes](#blend-nodes), [Merge Nodes](#merge-nodes), [Masking Nodes](#masking-nodes), and the [Mix Node](#mix-node).


### Premult Nodes

Premult nodes operate on 4-channel (color4) inputs/outputs, have one input named `in`, and either apply or unapply the alpha to the float or RGB color.

<a id="node-premult"> </a>

### `premult`
Multiply the RGB channels of the input by the Alpha channel of the input.

|Port |Description                         |Type  |Default           |
|-----|------------------------------------|------|------------------|
|`in` |The input stream to be premultiplied|color4|0.0, 0.0, 0.0, 1.0|
|`out`|Output: premultiplied `in`          |color4|`in`              |

<a id="node-unpremult"> </a>

### `unpremult`
Divide the RGB channels of the input by the Alpha channel of the input. If the Alpha value is zero, the `in` value is passed through unchanged.

|Port |Description                           |Type  |Default           |
|-----|--------------------------------------|------|------------------|
|`in` |The input stream to be unpremultiplied|color4|0.0, 0.0, 0.0, 1.0|
|`out`|Output: unpremultiplied `in`          |color4|`in`              |


### Blend Nodes

Blend nodes take two 1-4 channel inputs and apply the same operator to all channels (the math for alpha is the same as for R or RGB); below, "F" and "B" refer to any individual channel of the `fg` and `bg` inputs respectively.


<a id="node-plus"> </a>

### `plus`
Add two 1-4 channel inputs, with optional mixing between the bg input and the result.

|Port |Description                                                                       |Type         |Default |Accepted Values|
|-----|----------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                       |float, colorN|__zero__|               |
|`bg` |The background input stream                                                       |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'plus' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output: `fg` plus `bg`                                                            |Same as `fg` |`bg`    |               |

<a id="node-minus"> </a>

### `minus`
Subtract two 1-4 channel inputs, with optional mixing between the bg input and the result.

|Port |Description                                                                        |Type         |Default |Accepted Values|
|-----|-----------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                        |float, colorN|__zero__|               |
|`bg` |The background input stream                                                        |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'minus' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                             |Same as `fg` |`bg`    |               |

<a id="node-difference"> </a>

### `difference`
Absolute-value difference of two 1-4 channel inputs, with optional mixing between the bg input and the result.

|Port |Description                                                                             |Type         |Default |Accepted Values|
|-----|----------------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                             |float, colorN|__zero__|               |
|`bg` |The background input stream                                                             |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'difference' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                                  |Same as `fg` |`bg`    |               |

<a id="node-burn"> </a>

### `burn`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
1-(1-B)/F
```

|Port |Description                                                                       |Type         |Default |Accepted Values|
|-----|----------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                       |float, colorN|__zero__|               |
|`bg` |The background input stream                                                       |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'burn' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                            |Same as `fg` |`bg`    |               |

<a id="node-dodge"> </a>

### `dodge`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
B/(1-F)
```

|Port |Description                                                                        |Type         |Default |Accepted Values|
|-----|-----------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                        |float, colorN|__zero__|               |
|`bg` |The background input stream                                                        |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'dodge' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                             |Same as `fg` |`bg`    |               |

<a id="node-screen"> </a>

### `screen`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
1-(1-F)*(1-B)
```

|Port |Description                                                                         |Type         |Default |Accepted Values|
|-----|------------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                         |float, colorN|__zero__|               |
|`bg` |The background input stream                                                         |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'screen' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                              |Same as `fg` |`bg`    |               |

<a id="node-overlay"> </a>

### `overlay`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
2FB if B<0.5;
1-2(1-F)(1-B) if B>=0.5
```

|Port |Description                                                                          |Type         |Default |Accepted Values|
|-----|-------------------------------------------------------------------------------------|-------------|--------|---------------|
|`fg` |The foreground input stream                                                          |float, colorN|__zero__|               |
|`bg` |The background input stream                                                          |Same as `fg` |__zero__|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'overlay' operation (mix=1)|float        |1.0     |[0, 1]         |
|`out`|Output                                                                               |Same as `fg` |`bg`    |               |


### Merge Nodes

Merge nodes take two 4-channel (color4) inputs and use the built-in alpha channel(s) to control the compositing of the `fg` and `bg` inputs; "F" and "B" refer to individual non-alpha channels of the `fg` and `bg` inputs respectively, while "f" and "b" refer to the alpha channels of the `fg` and `bg` inputs.  Merge nodes are not defined for 1-channel or 3-channel inputs, and cannot be used on vector<em>N</em> streams.


<a id="node-disjointover"> </a>

### `disjointover`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
F+B         if f+b<=1
F+B(1-f)/b  if f+b>1
alpha: min(f+b,1)
```

|Port |Description                                                                               |Type  |Default           |Accepted Values|
|-----|------------------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                               |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                               |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'disjointover' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                                    |color4|`bg`              |               |

<a id="node-in"> </a>

### `in`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = Fb
Alpha = fb
```

|Port |Description                                                                     |Type  |Default           |Accepted Values|
|-----|--------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                     |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                     |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'in' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                          |color4|`bg`              |               |

<a id="node-mask"> </a>

### `mask`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = Bf
Alpha = bf
```

|Port |Description                                                                       |Type  |Default           |Accepted Values|
|-----|----------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                       |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                       |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'mask' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                            |color4|`bg`              |               |

<a id="node-matte"> </a>

### `matte`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = Ff+B(1-f)
Alpha = f+b(1-f)
```

|Port |Description                                                                        |Type  |Default           |Accepted Values|
|-----|-----------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                        |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                        |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'matte' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                             |color4|`bg`              |               |

<a id="node-out"> </a>

### `out`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = F(1-b)
Alpha = f(1-b)
```

|Port |Description                                                                      |Type  |Default           |Accepted Values|
|-----|---------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                      |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                      |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'out' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                           |color4|`bg`              |               |

<a id="node-over"> </a>

### `over`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = F+B(1-f)
Alpha = f+b(1-f)
```

|Port |Description                                                                       |Type  |Default           |Accepted Values|
|-----|----------------------------------------------------------------------------------|------|------------------|---------------|
|`fg` |The foreground input stream                                                       |color4|0.0, 0.0, 0.0, 0.0|               |
|`bg` |The background input stream                                                       |color4|0.0, 0.0, 0.0, 0.0|               |
|`mix`|A mixing value between `bg` (mix=0) and the result of the 'over' operation (mix=1)|float |1.0               |[0, 1]         |
|`out`|Output                                                                            |color4|`bg`              |               |


### Masking Nodes

Masking nodes take one 1-4 channel input `in` plus a separate float `mask` input and apply the same operator to all "in" channels; "F" refers to any individual channel of the `in` input, while "m" refers to the mask input.


<a id="node-inside"> </a>

### `inside`
An "inside" mask operation returning Fm

|Port  |Description                      |Type         |Default |Accepted Values|
|------|---------------------------------|-------------|--------|---------------|
|`in`  |The input stream to be masked    |float, colorN|__zero__|               |
|`mask`|The masking input signal         |float        |1.0     |[0, 1]         |
|`out` |Output: `in` multiplied by `mask`|Same as `in` |`in`    |               |

<a id="node-outside"> </a>

### `outside`
An "outside" mask operation returning F(1-m)

|Port  |Description                        |Type         |Default |Accepted Values|
|------|-----------------------------------|-------------|--------|---------------|
|`in`  |The input stream to be masked      |float, colorN|__zero__|               |
|`mask`|The masking input signal           |float        |1.0     |[0, 1]         |
|`out` |Output: `in` multiplied by 1-`mask`|Same as `in` |`in`    |               |


### Mix Node

The Mix node takes two 1-4 channel inputs `fg` and `bg` plus a separate 1-channel (float) or N-channel (same type and number of channels as `fg` and `bg`) `mix` input and mixes the `fg` and `bg` according to the mix value, either uniformly for a "float" `mix` type, or per-channel for non-float `mix` types; "F" refers to any individual channel of the `in` input, while "m" refers to the appropriate channel of the mix input.

<a id="node-mix"> </a>

### `mix`
A "mix" operation blending from "bg" to "fg" according to the mix amount, returning Fm+B(1-m)

|Port |Description                            |Type                  |Default |Accepted Values|
|-----|---------------------------------------|----------------------|--------|---------------|
|`fg` |The foreground input stream            |float, colorN, vectorN|__zero__|               |
|`bg` |The background input stream            |Same as `fg`          |__zero__|               |
|`mix`|The amount to mix `bg` to `fg`         |float                 |0.0     |[0, 1]         |
|`out`|Output: the result of the mix operation|Same as `fg`          |`bg`    |               |

|Port |Description                    |Type           |Default |
|-----|-------------------------------|---------------|--------|
|`fg` |The foreground input stream    |colorN, vectorN|__zero__|
|`bg` |The background input stream    |Same as `fg`   |__zero__|
|`mix`|The amount to mix `bg` to `fg` |Same as `fg`   |__zero__|
|`out`|Output                         |Same as `fg`   |`bg`    |

See also the [Standard Shader Nodes](#standard-shader-nodes) section below for additional shader-semantic variants of the [`mix` node](#node-mix-shader).



## Conditional Nodes

Conditional nodes are used to compare values of two streams, or to select a value from one of several streams.


<a id="node-ifgreater"> </a>

### `ifgreater`

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater than the `value2` input.

|Port    |Description                                       |Type                                     |Default |
|--------|--------------------------------------------------|-----------------------------------------|--------|
|`value1`|The first value to be compared                    |float, integer                           |__one__ |
|`value2`|The second value to be compared                   |Same as `value1`                         |__zero__|
|`in1`   |The value stream to output if `value1` > `value2` |float, colorN, vectorN, matrixNN, integer|__zero__|
|`in2`   |The value stream to output if `value1` <= `value2`|Same as `in1`                            |__zero__|
|`out`   |Output: the result of the comparison              |Same as `in1`                            |`in1`   |

|Port    |Description                        |Type            |Default |
|--------|-----------------------------------|----------------|--------|
|`value1`|The first value to be compared     |float, integer  |__one__ |
|`value2`|The second value to be compared    |Same as `value1`|__zero__|
|`out`   |Output: true if `value1` > `value2`|boolean         |false   |

<a id="node-ifgreatereq"> </a>

### `ifgreatereq`

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater or equal to the `value2` input.

|Port    |Description                                       |Type                                     |Default |
|--------|--------------------------------------------------|-----------------------------------------|--------|
|`value1`|The first value to be compared                    |float, integer                           |__one__ |
|`value2`|The second value to be compared                   |Same as `value1`                         |__zero__|
|`in1`   |The value stream to output if `value1` >= `value2`|float, colorN, vectorN, matrixNN, integer|__zero__|
|`in2`   |The value stream to output if `value1` < `value2` |Same as `in1`                            |__zero__|
|`out`   |Output: the result of the comparison              |Same as `in1`                            |`in1`   |

|Port    |Description                         |Type            |Default |
|--------|------------------------------------|----------------|--------|
|`value1`|The first value to be compared      |float, integer  |__one__ |
|`value2`|The second value to be compared     |Same as `value1`|__zero__|
|`out`   |Output: true if `value1` >= `value2`|boolean         |false   |

<a id="node-ifequal"> </a>

### `ifequal`

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is equal to the `value2` input.

|Port    |Description                                       |Type                                     |Default |
|--------|--------------------------------------------------|-----------------------------------------|--------|
|`value1`|The first value to be compared                    |float, integer                           |__one__ |
|`value2`|The second value to be compared                   |Same as `value1`                         |__zero__|
|`in1`   |The value stream to output if `value1` = `value2` |float, colorN, vectorN, matrixNN, integer|__zero__|
|`in2`   |The value stream to output if `value1` != `value2`|Same as `in1`                            |__zero__|
|`out`   |Output: the result of the comparison              |Same as `in1`                            |`in1`   |

|Port    |Description                                       |Type                                     |Default |
|--------|--------------------------------------------------|-----------------------------------------|--------|
|`value1`|The first value to be compared                    |boolean                                  |false   |
|`value2`|The second value to be compared                   |boolean                                  |false   |
|`in1`   |The value stream to output if `value1` = `value2` |float, colorN, vectorN, matrixNN, integer|__zero__|
|`in2`   |The value stream to output if `value1` != `value2`|Same as `in1`                            |__zero__|
|`out`   |Output: the result of the comparison              |Same as `in1`                            |`in1`   |

|Port    |Description                        |Type            |Default |
|--------|-----------------------------------|----------------|--------|
|`value1`|The first value to be compared     |float, integer  |__one__ |
|`value2`|The second value to be compared    |Same as `value1`|__zero__|
|`out`   |Output: true if `value1` = `value2`|boolean         |false   |

|Port    |Description                        |Type   |Default|
|--------|-----------------------------------|-------|-------|
|`value1`|The first value to be compared     |boolean|false  |
|`value2`|The second value to be compared    |boolean|false  |
|`out`   |Output: true if `value1` = `value2`|boolean|false  |

<a id="node-switch"> </a>

### `switch`

Output the value of one of up to ten input streams, according to the value of a selector input `which`. Note that not all inputs need to be connected. The output has the same type as `in1`, with a default value of __zero__.

|Port   |Description                                                                                                                |Type                            |Default |
|-------|---------------------------------------------------------------------------------------------------------------------------|--------------------------------|--------|
|`in1`  |Input stream to select from using `which`                                                                                  |float, colorN, vectorN, matrixNN|__zero__|
|`in2`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in3`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in4`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in5`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in6`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in7`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in8`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in9`  |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`in10` |Input stream to select from using `which`                                                                                  |Same as `in1`                   |__zero__|
|`which`|Selector to choose which input to take values from; the output comes from input floor(`which`)+1, clamped to the 1-10 range|float, integer                  |__zero__|
|`out`  |Output: the selected input                                                                                                 |Same as `in1`                   |`in1`   |



## Channel Nodes

Channel nodes are used to perform channel manipulations and data type conversions on streams.


<a id="node-extract"> </a>

### `extract`

Isolate a single float channel from a __vectorN__ or __colorN__ stream.

|Port   |Description                                 |Type           |Default |
|-------|--------------------------------------------|---------------|--------|
|`in`   |The input stream from which to extract `out`|colorN, vectorN|__zero__|
|`index`|The index of the channel in `in` to extract |integer        |0       |
|`out`  |Output: the `index`th channel of `in`       |float          |0.0     |

The valid range for `index` should be clamped to $[0,N)$ in the user interface, where __N__ is the size of the input vector stream. `index` is a uniform, non-varying value. Any `index` values outside of the valid range should result in an error.

<a id="node-convert"> </a>

### `convert`
Convert a stream from one data type to another.

|Port |Description                |Type           |Default |
|-----|---------------------------|---------------|--------|
|`in` |The input stream to convert|boolean        |false   |
|`out`|Output: the converted value|float, integer |__zero__|

|Port |Description                              |Type   |Default|
|-----|-----------------------------------------|-------|-------|
|`in` |The input stream to convert              |integer|0      |
|`out`|Output: true for any non-zero input value|boolean|false  |

|Port |Description                |Type   |Default|
|-----|---------------------------|-------|-------|
|`in` |The input stream to convert|integer|0      |
|`out`|Output: the converted value|float  |0.0    |

|Port |Description                             |Type                   |Default |
|-----|----------------------------------------|-----------------------|--------|
|`in` |The input stream to convert             |boolean, float, integer|__zero__|
|`out`|Output: copy input value to all channels|colorN, vectorN        |__zero__|

|Port |Description                |Type           |Default |
|-----|---------------------------|---------------|--------|
|`in` |The input stream to convert|colorN, vectorN|__zero__|
|`out`|Output: the converted value|colorM, vectorM|__zero__|

|Port |Description                                                      |Type                                    |Default |
|-----|-----------------------------------------------------------------|----------------------------------------|--------|
|`in` |The input stream to convert                                      |boolean, integer, float, colorN, vectorN|__zero__|
|`out`|Output: an unlit surface shader emitting the input value as color|surfaceshader                           |        |

For boolean input values, all numeric output values will be either __zero__ or __one__.

For colorN/vectorN to colorM/vectorM:

  * if _N_ is the same as _M_, then channels are directly copied.
  * if _N_ is larger than _M_, then the first _M_ channels are used and the excess channels ignored.
  * if _N_ is smaller than _M_, then the _N_ channels are directly copied and additional channels are populated with 0, aside from the fourth channel which is populated with 1.


<a id="node-combine2"> </a>

### `combine2`

Combine the channels from two streams into the same number of channels of a single output stream of a compatible type.

|Port |Description                                                      |Type   |Default |
|-----|-----------------------------------------------------------------|-------|--------|
|`in1`|The input stream that will be sent to the first channel of `out` |float  |0.0     |
|`in2`|The input stream that will be sent to the second channel of `out`|float  |0.0     |
|`out`|Output: the combined value                                       |vector2|0.0, 0.0|

|Port |Description                                                      |Type  |Default           |
|-----|-----------------------------------------------------------------|------|------------------|
|`in1`|The input stream that will be sent to the first channel of `out` |color3|0.0, 0.0, 0.0     |
|`in2`|The input stream that will be sent to the second channel of `out`|float |0.0               |
|`out`|Output: the combined value                                       |color4|0.0, 0.0, 0.0, 0.0|

|Port |Description                                                      |Type   |Default           |
|-----|-----------------------------------------------------------------|-------|------------------|
|`in1`|The input stream that will be sent to the first channel of `out` |vector3|0.0, 0.0, 0.0     |
|`in2`|The input stream that will be sent to the second channel of `out`|float  |0.0               |
|`out`|Output: the combined value                                       |vector4|0.0, 0.0, 0.0, 0.0|

|Port |Description                                                      |Type   |Default           |
|-----|-----------------------------------------------------------------|-------|------------------|
|`in1`|The input stream that will be sent to the first channel of `out` |vector2|0.0, 0.0          |
|`in2`|The input stream that will be sent to the second channel of `out`|vector2|0.0, 0.0          |
|`out`|Output: the combined value                                       |vector4|0.0, 0.0, 0.0, 0.0|

<a id="node-combine3"> </a>

### `combine3`

Combine the channels from three streams into the same number of channels of a single output stream of a compatible type.

|Port |Description                                                      |Type           |Default |
|-----|-----------------------------------------------------------------|---------------|--------|
|`in1`|The input stream that will be sent to the first channel of `out` |float          |0.0     |
|`in2`|The input stream that will be sent to the second channel of `out`|float          |0.0     |
|`in3`|The input stream that will be sent to the third channel of `out` |float          |0.0     |
|`out`|Output: the combined value                                       |color3, vector3|__zero__|

<a id="node-combine4"> </a>

### `combine4`

Combine the channels from four streams into the same number of channels of a single output stream of a compatible type.

|Port |Description                                                      |Type           |Default |
|-----|-----------------------------------------------------------------|---------------|--------|
|`in1`|The input stream that will be sent to the first channel of `out` |float          |0.0     |
|`in2`|The input stream that will be sent to the second channel of `out`|float          |0.0     |
|`in3`|The input stream that will be sent to the third channel of `out` |float          |0.0     |
|`in4`|The input stream that will be sent to the fourth channel of `out`|float          |0.0     |
|`out`|Output: the combined value                                       |color4, vector4|__zero__|

<a id="node-separate2"> </a>

### `separate2`

Split the channels of a 2-channel stream into separate float outputs.

|Port  |Description                     |Type   |Default |
|------|--------------------------------|-------|--------|
|`in`  |The input stream to be separated|vector2|0.0, 0.0|
|`outx`|Output: the x channel of `in`   |float  |0.0     |
|`outy`|Output: the y channel of `in`   |float  |0.0     |

For the vector2-input `in`, `outx` and `outy` correspond to the x- and y-components of `in`.

<a id="node-separate3"> </a>

### `separate3`

Split the channels of a 3-channel stream into separate float outputs.

|Port  |Description                     |Type  |Default      |
|------|--------------------------------|------|-------------|
|`in`  |The input stream to be separated|color3|0.0, 0.0, 0.0|
|`outr`|Output: the r channel of `in`   |float |0.0          |
|`outg`|Output: the g channel of `in`   |float |0.0          |
|`outb`|Output: the b channel of `in`   |float |0.0          |

|Port  |Description                     |Type   |Default      |
|------|--------------------------------|-------|-------------|
|`in`  |The input stream to be separated|vector3|0.0, 0.0, 0.0|
|`outx`|Output: the x channel of `in`   |float  |0.0          |
|`outy`|Output: the y channel of `in`   |float  |0.0          |
|`outz`|Output: the z channel of `in`   |float  |0.0          |

When the input `in` is a color3, `outr`, `outg`, and `outb` correspond to the r-, g-, and b-components of `in`, respectively.

When the input `in` is a vector3, `outx`, `outy`, and `outz` correspond to the x-, y-, and z-components of `in`, respectively.

<a id="node-separate4"> </a>

### `separate4`

Split the channels of a 4-channel stream into separate float outputs.

|Port  |Description                     |Type  |Default           |
|------|--------------------------------|------|------------------|
|`in`  |The input stream to be separated|color4|0.0, 0.0, 0.0, 0.0|
|`outr`|Output: the r channel of `in`   |float |0.0               |
|`outg`|Output: the g channel of `in`   |float |0.0               |
|`outb`|Output: the b channel of `in`   |float |0.0               |
|`outa`|Output: the a channel of `in`   |float |0.0               |

|Port  |Description                     |Type   |Default           |
|------|--------------------------------|-------|------------------|
|`in`  |The input stream to be separated|vector4|0.0, 0.0, 0.0, 0.0|
|`outx`|Output: the x channel of `in`   |float  |0.0               |
|`outy`|Output: the y channel of `in`   |float  |0.0               |
|`outz`|Output: the z channel of `in`   |float  |0.0               |
|`outw`|Output: the w channel of `in`   |float  |0.0               |

When the input `in` is a color4, `outr`, `outg`, `outb`, and `outa` correspond to the r-, g-, b-, and alpha components of `in`, respectively.

When the input `in` is a vector4, `outx`, `outy`, `outz`, and `outw` correspond to the x-, y-, z-, and w-components of `in`, respectively.


## Convolution Nodes

Convolution nodes have one input named "in", and apply a defined convolution function on the input stream.  Some of these nodes may not be implementable in ray tracing applications; they are provided for the benefit of purely 2D image processing applications.


<a id="node-blur"> </a>

### `blur`

Applies a convolution blur to the input stream.

|Port        |Description                                |Type                  |Default |Accepted Values|
|------------|-------------------------------------------|----------------------|--------|---------------|
|`in`        |The input stream to be blurred             |float, colorN, vectorN|__zero__|               |
|`size`      |The size of the blur kernel in 0-1 UV space|float                 |0.0     |               |
|`filtertype`|The spatial filter used in the blur        |string                |box     |box, gaussian  |
|`out`       |Output: the blurred `in`                   |Same as `in`          |`in`    |               |

<a id="node-heighttonormal"> </a>

### `heighttonormal`

Convert a scalar height map to a tangent-space normal map of type `vector3`. The output normal map is encoded with all channels in the [0-1] range, enabling its storage in unsigned image formats.

|Port      |Description                                                                      |Type   |Default      |
|----------|---------------------------------------------------------------------------------|-------|-------------|
|`in`      |The input scalar height map                                                      |float  |0.0          |
|`scale`   |Multiplier applied to the `in` signal                                            |float  |1.0          |
|`texcoord`|The texture coordinates that the heightfield gradient is computed with respect to|vector2|_UV0_        |
|`out`     |Output: tangent-space normal computed from `in`                                  |vector3|0.5, 0.5, 1.0|


<br>


# Standard Shader Nodes

The Standard MaterialX Library defines the following nodes and node variants operating on "shader"-semantic types.  Standard library shaders do not respond to external illumination; please refer to the [**MaterialX Physically Based Shading Nodes**](./MaterialX.PBRSpec.md#materialx-pbs-library) document for definitions of additional nodes and shader constructors which do respond to illumination, as well as [**MaterialX NPR Shading Nodes**](./MaterialX.NPRSpec.md) for definitions of shaders and nodes applicable to non-photorealistic rendering.

<a id="node-surface-unlit"> </a>

### `surface_unlit`
An unlit surface shader node, representing a surface that can emit and transmit light, but does not receive illumination from light sources or other surfaces.

|Port                |Description                          |Type         |Default      |
|--------------------|-------------------------------------|-------------|-------------|
|`emission`          |The surface emission amount          |float        |1.0          |
|`emission_color`    |Surface emission color               |color3       |1.0, 1.0, 1.0|
|`transmission`      |The surface transmission amount      |float        |0.0          |
|`transmission_color`|Surface transmission color           |color3       |1.0, 1.0, 1.0|
|`opacity`           |Surface cutout opacity               |float        |1.0          |
|`out`               |Output: unlit surface shader closure |surfaceshader|             |

<a id="node-displacement"> </a>

### `displacement`
Constructs a displacement shader describing geometric modification to surfaces.

The scalar signature displaces along the surface normal direction, while the vector signature allows displacement in tangent/normal space using (dPdu, dPdv, N) coordinates.

|Port          |Description                             |Type              |Default |
|--------------|----------------------------------------|------------------|--------|
|`displacement`|Displacement amount or direction        |float, vector3    |__zero__|
|`scale`       |Scale factor for the displacement       |float             |1.0     |
|`out`         |Output: the computed displacement shader|displacementshader|        |

<a id="node-mix-shader"> </a>

### `mix`
A linear blend between two surface/displacement/volumeshader closures.

|Port  |Description                                           |Type         |Default|
|------|------------------------------------------------------|-------------|-------|
|`bg`  |The background surface closure                        |surfaceshader|       |
|`fg`  |The foreground surface closure                        |surfaceshader|       |
|`mix` |The blending factor used to mix the two input closures|float        |0.0    |
|`out` |Output: surface shader closure                        |surfaceshader|       |

|Port  |Description                                           |Type              |Default|
|------|------------------------------------------------------|------------------|-------|
|`bg`  |The background displacement closure                   |displacementshader|       |
|`fg`  |The foreground displacement closure                   |displacementshader|       |
|`mix` |The blending factor used to mix the two input closures|float             |0.0    |
|`out` |Output: displacement shader closure                   |displacementshader|       |

|Port  |Description                                           |Type        |Default|
|------|------------------------------------------------------|------------|-------|
|`bg`  |The background volume closure                         |volumeshader|       |
|`fg`  |The foreground volume closure                         |volumeshader|       |
|`mix` |The blending factor used to mix the two input closures|float       |0.0    |
|`out` |Output: volume shader closure                         |volumeshader|       |

