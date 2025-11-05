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

### `image`

Samples data from a single image, or from a layer within a multi-layer image.  When used in the context of rendering a geometry, the image is mapped onto the geometry based on geometry UV coordinates, with the lower-left corner of an image mapping to the (0,0) UV coordinate (or to the fractional (0,0) UV coordinate for tiled images).

The type of the &lt;image> node determines the number of channels output, which may be less than the number of channels in the image file, outputting the first N channels from the image file.  So a `float` &lt;image> would return the Red channel of an RGB image, and a `color3` &lt;image> would return the RGB channels of an RGBA image.  If the type of the &lt;image> node has more channels than the referenced image file, then the output will contain zero values in all channels beyond the N channels of the image file.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.  The `filtertype` input supports options `closest` (nearest-neighbor single-sample), `linear`, and `cubic`.

@MX_TABLE_image@

<a id="node-tiledimage"> </a>

### `tiledimage`
Samples data from a single image, with provisions for tiling and offsetting the image across uv space.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

@MX_TABLE_tiledimage@

<a id="node-latlongimage"> </a>

### `latlongimage`
Samples an equiangular map along a view direction with adjustable latitudinal offset.

The `file` input can include one or more substitutions to change the file name that is accessed, as described in the [Filename Substitutions](./MaterialX.Specification.md#filename-substitutions) section in the main Specification document.

@MX_TABLE_latlongimage@

<a id="node-triplanarprojection"> </a>

### `triplanarprojection`
Samples data from three images (or layers within multi-layer images), and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.

@MX_TABLE_triplanarprojection@


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

### `constant`
Outputs a constant value.

@MX_TABLE_constant@


<a id="node-ramplr"> </a>

### `ramplr`
A left-to-right linear value ramp

@MX_TABLE_ramplr@

<a id="node-ramptb"> </a>

### `ramptb`
A top-to-bottom linear value ramp.

@MX_TABLE_ramptb@

<a id="node-ramp4"> </a>

### `ramp4`
A 4-corner bilinear value ramp.

@MX_TABLE_ramp4@

<a id="node-splitlr"> </a>

### `splitlr`
A left-right split matte, split at a specified `U` value.

@MX_TABLE_splitlr@

<a id="node-splittb"> </a>

### `splittb`
A top-bottom split matte, split at a specified `V`` value.

@MX_TABLE_splittb@

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

### `noise2d`
2D Perlin noise in 1, 2, 3 or 4 channels.

@MX_TABLE_noise2d@

<a id="node-noise3d"> </a>

### `noise3d`
3D Perlin noise in 1, 2, 3 or 4 channels.

@MX_TABLE_noise3d@

<a id="node-fractal2d"> </a>

* **`fractal2d`**: Zero-centered 2D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 2D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.
  * `amplitude` (float or vector<em>N</em>): the center-to-peak amplitude of the noise (peak-to-peak amplitude is 2x this value).  Default is 1.0.
  * `octaves` (integer): the number of octaves of noise to be summed.  Default is 3.
  * `lacunarity` (float or vector<em>N</em>): the exponential scale between successive octaves of noise; must be an integer value if period is non-zero so the result is properly tileable.  Default is 2.0.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
  * `diminish` (float or vector<em>N</em>): the rate at which noise amplitude is diminished for each octave.  Should be between 0.0 and 1.0; default is 0.5.  Vector<em>N</em>-output types can provide either a float (isotropic) or vector<em>N</em> (anisotropic) values for `lacunarity` and `diminish`.
  * `texcoord` (vector2): the 2D texture coordinate at which the noise is evaluated.  Default is to use the first set of texture coordinates.

<a id="node-fractal3d"> </a>

### `fractal3d`
Zero-centered 3D Fractal noise in 1, 2, 3 or 4 channels, created by summing several octaves of 3D Perlin noise, increasing the frequency and decreasing the amplitude at each octave.

@MX_TABLE_fractal3d@

<a id="node-cellnoise2d"> </a>

### `cellnoise2d`
2D cellular noise, 1 or 3 channels (type float or vector3).

@MX_TABLE_cellnoise2d@

<a id="node-cellnoise3d"> </a>

### `cellnoise3d`
3D cellular noise, 1 or 3 channels (type float or vector3).

@MX_TABLE_cellnoise3d@

<a id="node-worleynoise2d"> </a>

### `worleynoise2d`
2D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).

@MX_TABLE_worleynoise2d@

<a id="node-worleynoise3d"> </a>

### `worleynoise3d`
3D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).

@MX_TABLE_worleynoise3d@

<a id="node-unifiednoise2d"> </a>

### `unifiednoise2d`
(NG): a single node supporting 2D Perlin, Cell, Worley or Fractal noise in a unified interface.

@MX_TABLE_unifiednoise2d@

<a id="node-unifiednoise3d"> </a>

### `unifiednoise3d`
(NG): a single node supporting 3D Perlin, Cell, Worley or Fractal noise in a unified interface.

@MX_TABLE_unifiednoise3d@


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

### `checkerboard`
2D checkerboard pattern.

@MX_TABLE_checkerboard@

<a id="node-line"> </a>

### `line`
2D line pattern.

@MX_TABLE_line@

<a id="node-circle"> </a>

### `circle`
2D circle(disk) pattern.

@MX_TABLE_circle@

<a id="node-cloverleaf"> </a>

### `cloverleaf`
2D cloverleaf pattern: four semicircles on the edges of a square defined by center and radius.

@MX_TABLE_cloverleaf@

<a id="node-hexagon"> </a>

### `hexagon`
2D hexagon pattern.

@MX_TABLE_hexagon@

<a id="node-grid"> </a>

### `grid`
Creates a grid pattern of (1, 1, 1) white lines on a (0, 0, 0) black background with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

@MX_TABLE_grid@

<a id="node-crosshatch"> </a>

### `crosshatch`
Creates a crosshatch pattern with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

@MX_TABLE_crosshatch@

<a id="node-tiledcircles"> </a>

### `tiledcircles`
Creates a black and white pattern of circles with a defined tiling and size (diameter). Pattern can be regular or staggered.

@MX_TABLE_tiledcircles@

<a id="node-tiledcloverleafs"> </a>

### `tiledcloverleafs`
Creates a black and white pattern of cloverleafs with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

@MX_TABLE_tiledcloverleafs@

<a id="node-tiledhexagons"> </a>

### `tiledhexagons`
Creates a black and white pattern of hexagons with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

@MX_TABLE_tiledhexagons@



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

### `position`
The coordinates associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_position@

<a id="node-normal"> </a>

### `normal`
The normalized geometric normal associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_normal@

<a id="node-tangent"> </a>

### `tangent`
The geometric tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_tangent@

<a id="node-bitangent"> </a>

### `bitangent`
The geometric bi-tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_bitangent@

<a id="node-bump"> </a>

### `bump`
The normalized normal computed by offsetting the surface world space position along its world space normal.

@MX_TABLE_bump@

<a id="node-texcoord"> </a>

### `texcoord`
The 2D or 3D texture coordinates associated with the currently-processed data

@MX_TABLE_texcoord@

<a id="node-geomcolor"> </a>

### `geomcolor`
The color associated with the current geometry at the current position, generally bound via per-vertex color values. The type must match the type of the "color" bound to the geometry.

@MX_TABLE_geomcolor@

<a id="node-geompropvalue"> </a>

### `geompropvalue`
The value of the specified varying geometric property (defined using <geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

@MX_TABLE_geompropvalue@

<a id="node-geompropvalueuniform"> </a>

### `geompropvalueuniform`
The value of the specified uniform geometric property (defined using <geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

@MX_TABLE_geompropvalueuniform@


Additionally, the `geomcolor` and `geompropvalue` nodes for color3/color4-type properties can take a `colorspace` attribute to declare what colorspace the color property value is in; the default is "none" for no colorspace declaration (and hence no colorspace conversion).



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

@MX_TABLE_frame@

<a id="node-time"> </a>

### `time`
The current time in seconds, as defined by the host environment.

@MX_TABLE_time@


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

### `add`
Add a value to the incoming float/color/vector/matrix

@MX_TABLE_add@

<a id="node-subtract"> </a>

### `subtract`
Subtract a value from the incoming float/color/vector/matrix

@MX_TABLE_subtract@

<a id="node-multiply"> </a>

### `multiply`
Multiply two values together. Scalar and vector types multiply component-wise, while matrices multiply with the standard matrix product.

@MX_TABLE_multiply@

<a id="node-divide"> </a>

### `divide`
Divide one value by another. Scalar and vector types divide component-wise, while for matrices `in1` is multiplied with the inverse of `in2`.

@MX_TABLE_divide@

<a id="node-modulo"> </a>

### `modulo`
The remaining fraction after dividing an incoming float/color/vector by a value and subtracting the integer portion. Modulo always returns a non-negative result.

@MX_TABLE_modulo@

<a id="node-fract"> </a>

### `fract`
Returns the fractional part of the floating-point input.

@MX_TABLE_fract@

<a id="node-invert"> </a>

### `invert`
subtract the incoming float, color, or vector from `amount` in all channels, outputting: `amount - in`.

@MX_TABLE_invert@

<a id="node-absval"> </a>

### `absval`
The per-channel absolute value of the incoming float/color/vector.

@MX_TABLE_absval@

<a id="node-sign"> </a>

### `sign`
The per-channel sign of the incoming float/color/vector value: -1 for negative, +1 for positive, or 0 for zero.

@MX_TABLE_sign@

<a id="node-floor"> </a>

### `floor`
The per-channel nearest integer value less than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the floor(float) also has a variant outputting an integer type.

@MX_TABLE_floor@

<a id="node-ceil"> </a>

### `ceil`
The per-channel nearest integer value greater than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the ceil(float) also has a variant outputting an integer type.

@MX_TABLE_ceil@

<a id="node-round"> </a>

### `round`
Round each channel of the incoming float/color/vector values to the nearest integer value.

@MX_TABLE_round@

<a id="node-power"> </a>

### `power`
Raise incoming float/color values to the specified exponent, commonly used for "gamma" adjustment.

@MX_TABLE_power@

<a id="node-safepower"> </a>

### `safepower`
Raise incoming float/color values to the specified exponent. Negative "in1" values will result in negative output values.

@MX_TABLE_safepower@

<a id="node-sin"> </a>

### `sin`
The sine of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_sin@

<a id="node-cos"> </a>

### `cos`
The cosine of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_cos@

<a id="node-tan"> </a>

### `tan`
The tangent of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_tan@

<a id="node-asin"> </a>

### `asin`
The arcsine of the incoming value. The output will be expressed in radians.

@MX_TABLE_asin@

<a id="node-acos"> </a>

### `acos`
The arccosine of the incoming value. The output will be expressed in radians.

@MX_TABLE_acos@

<a id="node-atan2"> </a>

### `atan2`
the arctangent of the expression (`iny`/`inx`). The output will be expressed in radians.

@MX_TABLE_atan2@

<a id="node-sqrt"> </a>

### `sqrt`
The square root of the incoming value.

@MX_TABLE_sqrt@

<a id="node-ln"> </a>

### `ln`
The natural logarithm of the incoming value.

@MX_TABLE_ln@

<a id="node-exp"> </a>

### `exp`
$e$ to the power of the incoming value.

@MX_TABLE_exp@

<a id="node-clamp"> </a>

### `clamp`
Clamp incoming values per-channel to a specified range of float/color/vector values.

@MX_TABLE_clamp@

<a id="node-trianglewave"> </a>

### `trianglewave`
Generate a triangle wave from the given scalar input. The generated wave ranges from zero to one and repeats on integer boundaries.

@MX_TABLE_trianglewave@

<a id="node-min"> </a>

### `min`
Select the minimum of the two incoming values

@MX_TABLE_min@

<a id="node-max"> </a>

### `max`
Select the maximum of the two incoming values

@MX_TABLE_max@

<a id="node-normalize"> </a>

### `normalize`
Output the incoming vectorN stream normalized.

@MX_TABLE_normalize@

<a id="node-magnitude"> </a>

### `magnitude`
Output the float magnitude (vector length) of the incoming vectorN stream; cannot be used on float or colorN streams. Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.

@MX_TABLE_magnitude@

<a id="node-distance"> </a>

### `distance`
Measures the distance between two points in 2D, 3D, or 4D.

@MX_TABLE_distance@

<a id="node-dotproduct"> </a>

### `dotproduct`
Output the (float) dot product of two incoming vectorN streams; cannot be used on float or colorN streams.

@MX_TABLE_dotproduct@

<a id="node-crossproduct"> </a>

### `crossproduct`
Output the (vector3) cross product of two incoming vector3 streams; cannot be used on any other stream type. A disabled crossproduct node passes through the value of `in1` unchanged.

@MX_TABLE_crossproduct@

<a id="node-transformpoint"> </a>

### `transformpoint`
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformpoint@

<a id="node-transformvector"> </a>

### `transformvector`
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformvector@

<a id="node-transformnormal"> </a>

### `transformnormal`
Transform the incoming vector3 normal from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformnormal@

<a id="node-transformmatrix"> </a>

### `transformmatrix`
Transform the incoming vectorN by the specified matrix.

@MX_TABLE_transformmatrix@

<a id="node-normalmap"> </a>

### `normalmap`
Transform a normal vector from the encoded tangent space to world space. The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.

@MX_TABLE_normalmap@

<a id="node-creatematrix"> </a>

### `creatematrix`
Build a 3x3 or 4x4 matrix from three vector3 or four vector3 or vector4 inputs. A matrix44 may also be created from vector3 input values, in which case the fourth value will be set to 0.0 for `in1`-`in3`, and to 1.0 for `in4` when creating the matrix44.

@MX_TABLE_creatematrix@

<a id="node-transpose"> </a>

### `transpose`
Transpose the incoming matrix

@MX_TABLE_transpose@

<a id="node-determinant"> </a>

### `determinant`
Output the determinant of the incoming matrix.

@MX_TABLE_determinant@

<a id="node-invertmatrix"> </a>

### `invertmatrix`
Invert the incoming matrix.

@MX_TABLE_invertmatrix@

<a id="node-rotate2d"> </a>

### `rotate2d`
Rotate the incoming 2D vector about the origin.

@MX_TABLE_rotate2d@

<a id="node-rotate3d"> </a>

### `rotate3d`
Rotate the incoming 3D vector about the specified unit axis vector.

@MX_TABLE_rotate3d@

<a id="node-reflect"> </a>

### `reflect`
Reflect the incoming 3D vector about a surface normal vector.

@MX_TABLE_reflect@

<a id="node-refract"> </a>

### `refract`
Refract the incoming 3D vector through a surface with the given surface normal and relative index of refraction.

@MX_TABLE_refract@

<a id="node-place2d"> </a>

### `place2d`
Transform incoming 2D texture coordinates from one frame of reference to another.

@MX_TABLE_place2d@

<a id="node-dot"> </a>

* **`dot`**: a no-op, passes its input through to its output unchanged.  Users can use dot nodes to shape edge connection paths or provide documentation checkpoints in node graph layout UI's.  Dot nodes may also pass uniform values from &lt;constant> or other nodes with uniform="true" outputs to uniform &lt;input>s and &lt;token>s.
    * `in` (any type): the nodename to be connected to the Dot node's "in" input.


## Logical Operator Nodes

Logical operator nodes have one or two boolean typed inputs, and are used to construct higher level logical flow through the nodegraph.

<a id="node-and"> </a>

### `and`
logically AND the two input boolean values

@MX_TABLE_and@

<a id="node-or"> </a>

### `or`
logically Inclusive OR the two input boolean values

@MX_TABLE_or@

<a id="node-xor"> </a>

### `xor`
logically Exclusive OR the two input boolean values

@MX_TABLE_xor@

<a id="node-not"> </a>

### `not`
logically NOT the input boolean value

@MX_TABLE_not@


## Adjustment Nodes

Adjustment nodes have one input named "in", and apply a specified function to values in the incoming stream.

<a id="node-contrast"> </a>

### `contrast`
Increase or decrease the contrast of the incoming `in` values using `amount` as a linear slope multiplier.

@MX_TABLE_contrast@

<a id="node-remap"> </a>

### `remap`
Linearly remap incoming values from one range of values [`inlow`, `inhigh`] to another [`outlow`, `outhigh`].

@MX_TABLE_remap@

<a id="node-range"> </a>

### `range`
Remap incoming values from one range of values to another, optionally applying a gamma correction "in the middle".

@MX_TABLE_range@

<a id="node-smoothstep"> </a>

### `smoothstep`
Output a smooth, hermite-interpolated remapping of input values from [`low`, `high`] to [0,1].

@MX_TABLE_smoothstep@

<a id="node-luminance"> </a>

### `luminance`
Output a grayscale value containing the luminance of the incoming RGB color in all color channels.

Applications which support color management systems may choose to retrieve the luma coefficients of the working colorspace from the CMS to pass to the `luminance` node's implementation directly, rather than exposing it to the user.

@MX_TABLE_luminance@

<a id="node-rgbtohsv"> </a>

### `rgbtohsv`
Convert an incoming color from RGB to HSV space (with H and S ranging from 0 to 1); the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

@MX_TABLE_rgbtohsv@

<a id="node-hsvtorgb"> </a>

### `hsvtorgb`
Convert an incoming color from HSV to RGB space; the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

@MX_TABLE_hsvtorgb@

<a id="node-hsvadjust"> </a>

### `hsvadjust`
Adjust the hue, saturation and value of an RGB color by converting the input color to HSV, adding amount.x to the hue, multiplying the saturation by amount.y, multiplying the value by amount.z, then converting back to RGB.

@MX_TABLE_hsvadjust@

<a id="node-saturate"> </a>

### `saturate`
Adjust the saturation of a color, the alpha channel will be unchanged if present.

@MX_TABLE_saturate@

<a id="node-colorcorrect"> </a>

### `colorcorrect`
Combines various adjustment nodes into one artist-friendly color correction node. For color4 inputs, the alpha value is unchanged.

@MX_TABLE_colorcorrect@


## Compositing Nodes

Compositing nodes have two (required) inputs named `fg` and `bg`, and apply a function to combine them.  Compositing nodes are split into five subclassifications: [Premult Nodes](#premult-nodes), [Blend Nodes](#blend-nodes), [Merge Nodes](#merge-nodes), [Masking Nodes](#masking-nodes), and the [Mix Node](#mix-node).


### Premult Nodes

Premult nodes operate on 4-channel (color4) inputs/outputs, have one input named `in`, and either apply or unapply the alpha to the float or RGB color.

<a id="node-premult"> </a>

### `premult`
Multiply the R or RGB channels of the input by the Alpha channel of the input.

@MX_TABLE_premult@

<a id="node-unpremult"> </a>

### `unpremult`
Divide the RGB channels of the input by the Alpha channel of the input. If the Alpha value is zero, it is passed through unchanged.

@MX_TABLE_unpremult@


### Blend Nodes

Blend nodes take two 1-4 channel inputs and apply the same operator to all channels (the math for alpha is the same as for R or RGB); below, "F" and "B" refer to any individual channel of the `fg` and `bg` inputs respectively.


<a id="node-plus"> </a>

### `plus`
Add two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_plus@

<a id="node-minus"> </a>

### `minus`
Subtract two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_minus@

<a id="node-difference"> </a>

### `difference`
Absolute-value difference of two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_difference@

<a id="node-burn"> </a>

### `burn`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
1-(1-B)/F
```

@MX_TABLE_burn@

<a id="node-dodge"> </a>

### `dodge`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
B/(1-F)
```

@MX_TABLE_dodge@

<a id="node-screen"> </a>

### `screen`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
1-(1-F)*(1-B)
```

@MX_TABLE_screen@

<a id="node-overlay"> </a>

### `overlay`
Take two 1-4 channel inputs and apply the same operator to all channels:
```
2FB if B<0.5;
1-2(1-F)(1-B) if B>=0.5
```

@MX_TABLE_overlay@


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

@MX_TABLE_disjointover@

<a id="node-in"> </a>

### `in`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
RGB = Fb
Alpha = fb
```

@MX_TABLE_in@

<a id="node-mask"> </a>

### `mask`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
Bf  (alpha: bf)
```

@MX_TABLE_mask@

<a id="node-matte"> </a>

### `matte`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
Ff+B(1-f)  (alpha: f+b(1-f))
```

@MX_TABLE_matte@

<a id="node-out"> </a>

### `out`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
F(1-b)  (alpha: f(1-b))
```

@MX_TABLE_out@

<a id="node-over"> </a>

### `over`
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
```
F+B(1-f)  (alpha: f+b(1-f))
```

@MX_TABLE_over@


### Masking Nodes

Masking nodes take one 1-4 channel input `in` plus a separate float `mask` input and apply the same operator to all "in" channels; "F" refers to any individual channel of the `in` input, while "m" refers to the mask input.


<a id="node-inside"> </a>

### `inside`
An "inside" mask operation returning Fm

@MX_TABLE_inside@

<a id="node-outside"> </a>

### `outside`
An "outside" mask operation returning F(1-m)

@MX_TABLE_outside@


### Mix Node

The Mix node takes two 1-4 channel inputs `fg` and `bg` plus a separate 1-channel (float) or N-channel (same type and number of channels as `fg` and `bg`) `mix` input and mixes the `fg` and `bg` according to the mix value, either uniformly for a "float" `mix` type, or per-channel for non-float `mix` types; "F" refers to any individual channel of the `in` input, while "m" refers to the appropriate channel of the mix input.

<a id="node-mix"> </a>

### `mix`
A "mix" operation blending from "bg" to "fg" according to the mix amount, returning Fm+B(1-m)

@MX_TABLE_mix@

See also the [Standard Shader Nodes](#standard-shader-nodes) section below for additional shader-semantic variants of the [`mix` node](#node-mix-shader).



## Conditional Nodes

Conditional nodes are used to compare values of two streams, or to select a value from one of several streams.


<a id="node-ifgreater"> </a>

### `ifgreater`

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater than the `value2` input.

@MX_TABLE_ifgreater@

<a id="node-ifgreatereq"> </a>

### `ifgreatereq`

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater or equal to the `value2` input.

@MX_TABLE_ifgreatereq@

<a id="node-ifequal"> </a>

### `ifequal`

@MX_TABLE_ifequal@

<a id="node-switch"> </a>

### `switch`

Output the value of one of up to ten input streams, according to the value of a selector input `which`. Note that not all inputs need to be connected. The output has the same type as `in1`, with a default value of __zero__.

@MX_TABLE_switch@



## Channel Nodes

Channel nodes are used to perform channel manipulations and data type conversions on streams.


<a id="node-extract"> </a>

### `extract`

Isolate a single float channel from a __vectorN__ or __colorN__ stream. The output value is of type `float` with a default value of __zero__.

@MX_TABLE_extract@

The valid range for `index` should be clamped to $[0,N)$ in the user interface, where __N__ is the size of the input vector stream. `index` is a uniform, non-varying value. Any `index` values outside of the valid range should result in an error.

<a id="node-convert"> </a>

* **`convert`**: convert a stream from one data type to another.  Only certain unambiguous conversions are supported; see list below.
    * `in` (boolean or integer or float or color<em>N</em> or vector<em>N</em>): the input value or nodename

<a id="node-combine2"> </a>

### `combine2`

Combine the channels from two streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine2@

<a id="node-combine3"> </a>

### `combine3`

Combine the channels from three streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine3@

<a id="node-combine4"> </a>

### `combine4`

Combine the channels from four streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine4@

<a id="node-separate2"> </a>

### `separate2`

Split the channels of a 2-channel stream into separate float outputs.

@MX_TABLE_separate2@

For the vector2-input `in`, `outx` and `outy` correspond to the x- and y-components of `in`..

<a id="node-separate3"> </a>

### `separate3`

Split the channels of a 3-channel stream into separate float outputs.

@MX_TABLE_separate3@

When the input `in` is a color3, `outr`, `outg`, and `outb` correspond to the r-, g-, and b-components of `in`, respectively.

When the input `in` is a vector3, `outx`, `outy`, and `outz` correspond to the x-, y-, and z-components of `in`, respectively.

<a id="node-separate4"> </a>

### `separate4`

Split the channels of a 4-channel stream into separate float outputs.

@MX_TABLE_separate4@

When the input `in` is a color4, `outr`, `outg`, `outb`, and `outa` correspond to the r-, g-, b-, and alpha components of `in`, respectively.

When the input `in` is a vector4, `outx`, `outy`, `outz`, and `outw` correspond to the x-, y-, z-, and w-components of `in`, respectively.


## Convolution Nodes

Convolution nodes have one input named "in", and apply a defined convolution function on the input stream.  Some of these nodes may not be implementable in ray tracing applications; they are provided for the benefit of purely 2D image processing applications.


<a id="node-blur"> </a>

### `blur`

Applies a convolution blur to the input stream.

@MX_TABLE_blur@

<a id="node-heighttonormal"> </a>

### `heighttonormal`

Convert a scalar height map to a tangent-space normal map of type `vector3`. The output normal map is encoded with all channels in the [0-1] range, enabling its storage in unsigned image formats.

@MX_TABLE_heighttonormal@


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
