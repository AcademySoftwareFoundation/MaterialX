<!-----
MaterialX Supplemental Notes v1.39
----->


# MaterialX: Supplemental Notes

**Version 1.39**  
Doug Smythe - Industrial Light & Magic  
Jonathan Stone - Lucasfilm Advanced Development Group  
October 25, 2022  



# Introduction

This document details additional information about MaterialX and how it may be incorporated into studio pipelines.  The document describes a number of additional Supplemental Nodes providing enhanced functionality over the basic Standard Nodes, as well as a recommended naming convention for node definition elements and a directory structure to define packages of node definitions and implementations from various sources.



## Table of Contents

**[Introduction](#introduction)**  

**[Supplemental Nodes](#supplemental-nodes)**  
 [Supplemental Texture Nodes](#supplemental-texture-nodes)  
 [Supplemental Procedural Nodes](#supplemental-procedural-nodes)  
 [Supplemental Math Nodes](#supplemental-math-nodes)  
 [Supplemental Adjustment Nodes](#supplemental-adjustment-nodes)  
 [Supplemental Channel Nodes](#supplemental-channel-nodes)  

**[Recommended Element Naming Conventions](#recommended-element-naming-conventions)**  

**[Material and Node Library File Structure](#material-and-node-library-file-structure)**  
 [Examples](#examples)  

**[Definitions, Assets, and Libraries](#definitions-assets-and-libraries)**  
 [Organization Using Node Graphs](#organization-using-node-graphs)  
 [Publishing Definitions](#publishing-definitions)  
 [Dependencies and Organization](#dependencies-and-organization)  
 [Deployment, Transmission, and Translation](#deployment-transmission-and-translation)  



# Supplemental Nodes

The MaterialX Specification defines a number of Standard Nodes, which all implementations of MaterialX are expected to support, to the degree their host applications allow.  These nodes are the basic "building blocks" upon which more complex node functionality can be built.

This section describes a number of supplemental nodes for MaterialX.  These nodes are considered part of MaterialX, but are typically implemented using graphs of standard MaterialX nodes rather than being implemented for specific targets.  Certain applications may choose to implement these supplemental nodes using native coding languages for efficiency.  It is also expected that various applications will choose to extend these supplemental nodes with additional parameters and additional functionality.


### Supplemental Texture Nodes

* **`tiledimage`**: samples data from a single image, with provisions for tiling and offsetting the image across uv space.
    * `file` (uniform filename): the URI of an image file.  The filename can include one or more substitutions to change the file name (including frame number) that is accessed, as described in **Filename Substitutions** in the main Specification document.
    * `default` (float or color<em>N</em> or vector<em>N</em>): a default value to use if the `file` reference can not be resolved (e.g. if a &lt;geomtoken>, [interfacetoken] or {hostattr} is included in the filename but no substitution value or default is defined, or if the resolved file URI cannot be read), or if the specified `layer` does not exist in the file.  The `default` value must be the same type as the `<image>` element itself.  If `default` is not defined, the default color value will be 0.0 in all channels.
    * `texcoord` (vector2): the name of a vector2-type node specifying the 2D texture coordinate at which the image data is read.  Default is to use the current u,v coordinate.
    * `uvtiling` (vector2): the tiling rate for the given image along the U and V axes. Mathematically equivalent to multiplying the incoming texture coordinates by the given vector value. Default value is (1.0, 1.0).
    * `uvoffset` (vector2): the offset for the given image along the U and V axes. Mathematically equivalent to subtracting the given vector value from the incoming texture coordinates. Default value is (0.0, 0.0).
    * `realworldimagesize` (vector2): the real-world size represented by the `file` image, with unittype "distance".  A `unit` attribute may be provided to indicate the units that `realworldimagesize` is expressed in.
    * `realworldtilesize` (vector2): the real-world size of a single square 0-1 UV tile, with unittype "distance".  A `unit` attribute may be provided to indicate the units that `realworldtilesize` is expressed in.
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.

    ```
    <tiledimage name="in3" type="color3">
      <input name="file" type="filename" value="textures/mytile.tif"/>
      <input name="default" type="color3" value="0.0,0.0,0.0"/>
      <input name="uvtiling" type="vector2" value="3.0,3.0"/>
      <input name="uvoffset" type="vector2" value="0.5,0.5"/>
    </tiledimage>
    ```

* **`triplanarprojection`**: samples data from three images (or layers within multi-layer images), and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.
    * `filex` (uniform filename): the URI of an image file to be projected in the direction from the +X axis back toward the origin.
    * `filey` (uniform filename): the URI of an image file to be projected in the direction from the +Y axis back toward the origin with the +X axis to the right.
    * `filez` (uniform filename): the URI of an image file to be projected in the direction from the +Z axis back toward the origin.
    * `layerx` (uniform string): the name of the layer to extract from a multi-layer input file for the x-axis projection.  If no value for `layerx` is provided and the input file has multiple layers, then the "default" layer will be used, or "rgba" if there is no "default" layer.  Note: the number of channels defined by the `type` of the `<image>` must match the number of channels in the named layer.
    * `layery` (uniform string): the name of the layer to extract from a multi-layer input file for the y-axis projection.
    * `layerz` (uniform string): the name of the layer to extract from a multi-layer input file for the z-axis projection.
    * `default` (float or color<em>N</em> or vector<em>N</em>): a default value to use if any `file<em>X</em>` reference can not be resolved (e.g. if a &lt;geomtoken>, [interfacetoken] or {hostattr} is included in the filename but no substitution value or default is defined, or if the resolved file URI cannot be read)  The `default` value must be the same type as the `<triplanarprojection>` element itself.  If `default` is not defined, the default color value will be 0.0 in all channels.
    * `position` (vector3): a spatially-varying input specifying the 3D position at which the projection is evaluated.  Default is to use the current 3D object-space coordinate.
    * `normal` (vector3): a spatially-varying input specifying the 3D normal vector used for blending.  Default is to use the current object-space surface normal.
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.

    ```
    <triplanarprojection name="tri4" type="color3">
      <input name="filex" type="filename" value="<colorname>.X.tif"/>
      <input name="filey" type="filename" value="<colorname>.Y.tif"/>
      <input name="filez" type="filename" value="<colorname>.Z.tif"/>
      <input name="default" type="color3" value="0.0,0.0,0.0"/>
    </triplanarprojection>
    ```


### Supplemental Procedural Nodes

* **`ramp4`**: a 4-corner bilinear value ramp.
    * `valuetl` (float or color<em>N</em> or vector<em>N</em>): the value at the top-left (U0V1) corner
    * `valuetr` (float or color<em>N</em> or vector<em>N</em>): the value at the top-right (U1V1) corner
    * `valuebl` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom-left (U0V0) corner
    * `valuebr` (float or color<em>N</em> or vector<em>N</em>): the value at the bottom-right (U1V0) corner
    * `texcoord` (vector2, optional): the name of a vector2-type node specifying the 2D texture coordinate at which the ramp interpolation is evaluated.  Default is to use the first set of texture coordinates.

* **`unifiednoise2d`**: a single node supporting 2D Perlin, Cell, Worley or Fractal noise in a unified interface.
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

* **`unifiednoise3d`**: a single node supporting 3D Perlin, Cell, Worley or Fractal noise in a unified interface.
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



### Supplemental Math Nodes

* **`place2d`**: transform incoming UV texture coordinates for 2D texture placement.
    * `texcoord` (vector2): the input UV coordinate to transform; defaults to the current surface index=0 uv coordinate.
    * `pivot` (vector2): the pivot coordinate for scale and rotate: this is subtracted from u,v before applying scale/rotate, then added back after.  Default is (0,0).
    * `scale` (vector2): divide the u,v coord (after subtracting `pivot`) by this, so a scale (2,2) makes the texture image appear twice as big.  Negative values can be used to flip or flop the texture space.  Default is (1,1).
    * `rotate` (float): rotate u,v coord (after subtracting pivot) by this amount in degrees, so a positive value rotates UV coords counter-clockwise, and the image clockwise.  Default is 0.
    * `offset` (vector2): subtract this amount from the scaled/rotated/“pivot added back” UV coordinate; since U0,V0 is typically the lower left corner, a positive offset moves the texture image up and right.  Default is (0,0).
    * `operationorder` (integer enum): the order in which to perform the transform operations. "0" or "SRT" performs <em>-pivot scale rotate translate +pivot</em> as per the original implementation matching the behavior of certain DCC packages, and "1" or "TRS" performs <em>-pivot translate rotate scale +pivot</em> which does not introduce texture shear.  Default is 0 "SRT" for backward compatibility.

* **`safepower`**: raise incoming float/color values to the specified exponent.  Unlike the standard &lt;power> node, negative `in1` values for &lt;safepower> will result in negative output values, e.g. `out = sign(in1)*pow(abs(in1),in2)`.
    * `in1` (float or color<em>N</em> or vector<em>N</em>): the value or nodename for the primary input
    * `in2` (same type as `in` or float): exponent value or nodename; default is 1.0 in all channels

* **`triplanarblend`**: samples data from three inputs, and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.
    * inx (float or colorN): the image to be projected in the direction from the +X axis back toward the origin.  Default is 0 in all channels.
    * iny (float or colorN): the image to be projected in the direction from the +Y axis back toward the origin with the +X axis to the right.  Default is 0 in all channels.
    * inz (float or colorN): the image to be projected in the direction from the +Z axis back toward the origin.  Default is 0 in all channels.
    * position (vector3): a spatially-varying input specifying the 3D position at which the projection is evaluated.  Default is to use the current 3D object-space coordinate.
    * normal (vector3): a spatially-varying input specifying the 3D normal vector used for blending.  Default is to use the current object-space surface normal.
    * filtertype (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.



### Supplemental Adjustment Nodes

* **`contrast`**: increase or decrease contrast of incoming float/color values using a linear slope multiplier.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `amount` (same type as `in` or float): slope multiplier for contrast adjustment, 0.0 to infinity range.  Values greater than 1.0 increase contrast, values between 0.0 and 1.0 reduce contrast.  Default is 1.0 in all channels.
    * `pivot` (same type as `in` or float): center pivot value of contrast adjustment; this is the value that will not change as contrast is adjusted.  Default is 0.5 in all channels.

* **`range`**: remap incoming values from one range of float/color/vector values to another, optionally applying a gamma correction "in the middle".  Input values below `inlow` or above `inhigh` are extrapolated unless `doclamp` is true, in which case the output values will be clamped to the `outlow`..`outhigh` range.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `inlow` (same type as `in` or float): low value for input range.  Default is 0.0 in all channels.
    * `inhigh` (same type as `in` or float): high value for input range.  Default is 1.0 in all channels.
    * `gamma` (same type as `in` or float): inverse exponent applied to input value after first transforming from `inlow`..`inhigh` to 0..1; `gamma` values greater than 1.0 make midtones brighter.  Default is 1.0 in all channels.
    * `outlow` (same type as `in` or float): low value for output range.  Default is 0.0 in all channels.
    * `outhigh` (same type as `in` or float): high value for output range.  Default is 1.0 in all channels.
    * `doclamp` (boolean): If true, the output is clamped to the range `outlow`..`outhigh`.  Default is false.

* **`hsvadjust`**: adjust the hue, saturation and value of an RGB color by converting the input color to HSV, adding amount.x to the hue, multiplying the saturation by amount.y, multiplying the value by amount.z, then converting back to RGB.  A positive "amount.x" rotates hue in the "red to green to blue" direction, with amount of 1.0 being the equivalent to a 360 degree (e.g. no-op) rotation.  Negative or greater-than-1.0 hue adjustment values are allowed, wrapping at the 0-1 boundaries.  For color4 inputs, the alpha value is unchanged.
    * `in` (color3 or color4): the input value or nodename
    * `amount` (vector3): the HSV adjustment; a value of (0, 1, 1) is "no change" and is the default.

* **`saturate`**: (color3 or color4 only) adjust the saturation of a color; the alpha channel will be unchanged if present.  Note that this operation is **not** equivalent to the "amount.y" saturation adjustment of `hsvadjust`, as that operator does not take the working or any other colorspace into account.
    * `in` (float or color<em>N</em> or vector<em>N</em>): the input value or nodename
    * `amount` (float): a multiplier for saturation; the saturate operator performs a linear interpolation between the luminance of the incoming color value (copied to all three color channels) and the incoming color value itself.  Note that setting amount to 0 will result in an R=G=B gray value equal to the value that the `luminance` node (below) returns.  Default is 1.0.
    * `lumacoeffs` (uniform color3): the luma coefficients of the current working color space; if no specific color space can be determined, the ACEScg (ap1) luma coefficients [0.272287, 0.6740818, 0.0536895] will be used.  Applications which support color management systems may choose to retrieve this value from the CMS to pass to the &lt;saturate> node's implementation directly, rather than exposing it to the user.

* **`colorcorrect`**: Combines various adjustment nodes into one artist-friendly color correction node.  For color4 inputs, the alpha value is unchanged.
    * `in` (color3 or color4): the input color to be adjusted.
    * `hue` (float): Rotates the color hue, with values wrapping at 0-1 boundaries; default is 0.
    * `saturation` (float): Multiplies the input color saturation level; default is 1.
    * `gamma` (float): Applies a gamma correction to the color; default is 1.
    * `lift` (float): Raise the dark color values, leaving the white values unchanged; default is 0.
    * `gain` (float): Multiplier increases lighter color values, leaving black values unchanged; default is 1.
    * `contrast` (float): Linearly increase or decrease the color contrast; default is 1.
    * `contrastpivot` (float): Pivot value around which contrast applies. This value will not change as contrast is adjusted; default is 0.5.
    * `exposure` (float): Multplier which increases or decreases color brightness by 2^value; default is 0.

* **`curveadjust`**: output a smooth remapping of input values using the centripetal Catmull-Rom cubic spline curve defined by specified knot values, using an inverse spline lookup on input knot values and a forward spline through output knot values.  All channels of the input will be remapped using the same curve.
    * `in` (float or colorN or vectorN): the input value or nodename
    * `numknots` (uniform integer): the number of values in the knots and knotvalues arrays
    * `knots` (uniform floatarray): the list of input values defining the curve for the remapping.  At least 2 and at most 16 values must be provided.
    * `knotvalues` (uniform floatarray): the list of output values defining the curve for the remapping.  Must be the same length as knots.

* **`curvelookup`**: output a float, colorN or vectorN value smoothly interpolated between a number of knotvalue values, using the position of in within knots as the knotvalues interpolant.
    * `in` (float): the input interpolant value or nodename
    * `numknots` (uniform integer): the number of values in the knots and knotvalues arrays
    * `knots` (uniform floatarray): the list of knot values to interpolate in within.  At least 2 and at most 16 values must be provided.
    * `knotvalues` (uniform floatarray or colorNarray or vectorNarray): the values at each knot position to interpolate between. Must be the same length as knots.



### Supplemental Channel Nodes

* **`separate2`**: output each of the channels of a vector2 as a separate float output.
    * `in` (vector2): the input value or nodename
    * `outx` (**output**, float): the value of x channel.
    * `outy` (**output**, float): the value of y channel.

* **`separate3`**: output each of the channels of a color3 or vector3 as a separate float output.
    * `in` (color3 or vector3): the input value or nodename
    * `outr`/`outx` (**output**, float): the value of the red (for color3 streams) or x (for vector3 streams) channel.
    * `outg`/`outy` (**output**, float): the value of the green (for color3 streams) or y (for vector3 streams) channel.
    * `outb`/`outz` (**output**, float): the value of the blue (for color3 streams) or z (for vector3 streams) channel.

* **`separate4`**: output each of the channels of a color4 or vector4 as a separate float output.
    * `in` (color4 or vector4): the input value or nodename
    * `outr`/`outx` (**output**, float): the value of the red (for color4 streams) or x (for vector4 streams) channel.
    * `outg`/`outy` (**output**, float): the value of the green (for color4 streams) or y (for vector4 streams) channel.
    * `outb`/`outz` (**output**, float): the value of the blue (for color4 streams) or z (for vector4 streams) channel.
    * `outa`/`outw` (**output**, float): the value of the alpha (for color4 streams) or w (for vector4 streams) channel.

* **`separatecolor4`**: output the RGB and alpha channels of a color4 as separate outputs.
    * `in` (color4): the input value or nodename
    * `outcolor` (output, color3): the RGB channel values.
    * `outa` (output, float): the value of the alpha channel.



# Recommended Element Naming Conventions

While MaterialX elements can be given any valid name as described in the MaterialX Names section of the main specification, adhering to the following recommended naming conventions will make it easier to predict the name of a nodedef for use in implementation and nodegraph elements as well as help reduce the possibility of elements from different sources having the same name.

**Nodedef**:  "ND\__nodename_\__outputtype_[\__target_][\__version_]", or for nodes with multiple input types for a given output type (e.g. &lt;convert>), "ND\__nodename_\__inputtype_\__outputtype_[\__target_][\__version_]".

**Implementation**: "IM\__nodename_[\__inputtype_]\__outputtype_[\__target_][\__version_]".

**Nodegraph**, as an implementation for a node: "NG\__nodename_[\__inputtype_]\__outputtype_[\__target_][\__version_]".



# Material and Node Library File Structure

As studios and vendors develop libraries of shared definitions and implementations of MaterialX materials and nodes for various targets, it becomes beneficial to have a consistent, logical organizational structure for the files on disk that make up these libraries.  In this section, we propose a structure for files defining libraries of material nodes, &lt;nodedef>s, nodegraph implementations and actual target-specific native source code, as well as a mechanism for applications and MaterialX content to find and reference files within these libraries.

Legend for various components within folder hierarchies:

| Term | Description |
| --- | --- |
| _libname_ | The name of the library; the MaterialX Standard nodes are the "stdlib" library.  Libraries may choose to declare themselves to be in the <em>libname</em> namespace, although this is not required. |
| _target_ | The target for an implementation, e.g. "glsl", "oslpattern", "osl" or "mdl". |
| _sourcefiles_ | Source files (including includes and makefiles) for the target, in whatever format and structure the applicable build system requires. |


Here is the suggested structure and naming for the various files making up a MaterialX material or node definition library setup.  Italicized terms should be replaced with appropriate values, while boldface terms should appear verbatim.  The optional "\_\*" component of the filename can be any useful descriptor of the file's contents, e.g. "\_ng" for nodegraphs or "\_mtls" for materials.


 _libname_/_libname_**\_defs.mtlx**  (1)  
 _libname_/_libname_\_\***.mtlx**  (2)  
 _libname_/_target_/_libname_\_target[\_\*]**\_impl.mtlx**  (3)  
 _libname_/_target_/_sourcefiles_  (4)  



1. Nodedefs and other definitions in library _libname_.
2. Additional elements (e.g. nodegraph implementations for nodes, materials, etc.) in library _libname_.
3. Implementation elements for _libname_ specific to target _target_.
4. Source code files for _libname_ implementations specific to target _target_.

Note that nodedef files and nodegraph-implementation files go at the top _libname_ level, while &lt;implementation> element files go under the corresponding _libname_/_target_ level, next to their source code files.  This is so that studios may easily install only the implementations that are relevant to them, and applications can easily locate the implementations of nodes for specific desired targets.  Libraries are free to add additional arbitrarily-named folders for related content, such as an "images" subfolder for material library textures.

The _libname_\_defs.mtlx file typically contains nodedefs for the library, but may also contain other node types such as implementation nodegraphs, materials, looks, and any other element types.  The use of additional _libname_\_\*.mtlx files is optional, but those files should be Xincluded by the _libname_\_defs.mtlx file.

A file referenced by a MaterialX document or tool (e.g. XInclude files, filenames in &lt;image> or other MaterialX nodes, or command-line arguments in MaterialX tools) can be specified using either a relative or a fully-qualified absolute filesystem path.  A relative path is interpreted to be relative to either the location of the referencing MaterialX document itself, or relative to a location found within the current MaterialX search path: this path may be specified via an application setting (e.g. the `--path` option in MaterialXView) or globally using the MATERIALX_SEARCH_PATH environment variable.  These search paths are used for both XIncluded definitions and filename input values (e.g. images for nodes or source code for &lt;implementation>s), and applications may choose to define different search paths for different contexts if desired, e.g. for document processing vs. rendering.

The standard libraries `stdlib` and `pbrlib` are typically included _automatically_ by MaterialX applications, rather than through explicit XInclude directives within .mtlx files.  Non-standard libraries are included into MaterialX documents by XIncluding the top-level _libname_/_libname_\_defs.mtlx file, which is expected to in turn XInclude any additional .mtlx files needed by the library.


### Examples

In the examples below, MXROOT is a placeholder for one of the root paths defined in the current MaterialX search path.

A library of studio-custom material shading networks and example library materials:

```
    MXROOT/mtllib/mtllib_defs.mtlx                (material nodedefs and nodegraphs)
    MXROOT/mtllib/mtllib_mtls.mtlx                (library of materials using mtllib_defs)
    MXROOT/mtllib/images/*.tif                    (texture files used by mtllib_mtls <image> nodes)
```

Documents may include the above library using

```
   <xi:include href="mtllib/mtllib_defs.mtlx"/>
```

and that file would XInclude `mtllib_mtls.mtlx`.  &lt;Image> nodes within `mtllib_mtls.mtlx` would use `file` input values such as "images/bronze_color.tif", e.g. relative to the path of the `mtllib_mtls.mtlx` file itself.

Standard node definitions and reference OSL implementation:

```
    MXROOT/stdlib/stdlib_defs.mtlx                    (standard library node definitions)
    MXROOT/stdlib/stdlib_ng.mtlx                      (supplemental library node nodegraphs)
    MXROOT/stdlib/osl/stdlib_osl_impl.mtlx            (stdlib OSL implementation elem file)
    MXROOT/stdlib/osl/*.{h,osl} (etc)                 (stdlib OSL source files)
```

Layout for "genglsl" and "genosl" implementations of "stdlib" for MaterialX's shadergen component, referencing the above standard `stdlib_defs.mtlx` file:

```
    # Generated-GLSL implementations
    MXROOT/stdlib/genglsl/stdlib_genglsl_impl.mtlx    (stdlib genGLSL implementation file)
    MXROOT/stdlib/genglsl/stdlib_genglsl_cm_impl.mtlx (stdlib genGLSL color-mgmt impl. file)
    MXROOT/stdlib/genglsl/*.{inline,glsl}             (stdlib common genGLSL code)

    # Generated-OSL implementations
    MXROOT/stdlib/genosl/stdlib_genosl_impl.mtlx      (stdlib genOSL implementation file)
    MXROOT/stdlib/genosl/stdlib_genosl_cm_impl.mtlx   (stdlib genOSL color-mgmt impl. file)
    MXROOT/stdlib/genosl/*.{inline,osl}               (stdlib common genOSL code)
```

Layout for the shadergen PBR shader library ("pbrlib") with implementations for "genglsl" and "genosl" (generated GLSL and OSL, respectively) targets:

```
    MXROOT/pbrlib/pbrlib_defs.mtlx                    (PBR library definitions)
    MXROOT/pbrlib/pbrlib_ng.mtlx                      (PBR library nodegraphs)
    MXROOT/pbrlib/genglsl/pbrlib_genglsl_impl.mtlx    (pbr impl file referencing genGLSL source)
    MXROOT/pbrlib/genglsl/*.{inline,glsl}             (pbr common genGLSL code)
    MXROOT/pbrlib/genosl/pbrlib_genosl_impl.mtlx      (pbr impl file referencing genOSL source)
    MXROOT/pbrlib/genosl/*.{inline,osl}               (pbr common genOSL code)
```



# Definitions, Assets, and Libraries

In this section we propose a set of guidelines for managing unique definitions or assets and organization into libraries, wherein:

* Definitions: Correspond directly to &lt;nodedefs> which may or be either source code implementations or based on existing node definitions.
* Assets: is a term which corresponds to a definition plus any additional metadata on a definition and /or related resources such as input images.  These can be organized in logical groupings based on a desired semantic.
* Libraries: are a collection of assets.


### Organization Using Node Graphs

While it is possible to just have a set of connected nodes in a document, it is not possible to have any formal unique interface. This can invariably lead to nodes which have duplicate names, the inability to control what interfaces are exposed and inability to maintain variations over time.

Thus the base requirement for a definition is to encapsulate the nodes into a &lt;nodegraph>. This provides for:

1. Hiding Complexity: Where all nodes are scoped with the graph. For user interaction point, it makes possible the ability to “drill down” into a graph as needed but otherwise a black box representation can be provided.
2. Identifier / Path Uniqueness : The nodegraph name decreases the chances of name clashes. For example two top level nodes both called “foo” would have unique paths “bar1/foo” and “bar2/foo” when placed into two nodegraphs “bar1” and “bar2”.  
3. Interface / node signature control where specific inputs may be exposed via “interfacename” connections and outputs as desired.  This differs from “hiding” inputs or outputs which do not change the signature. The former enforces what is exposed to the user while the latter are just interface hints.

For individual inputs it is recommended to add the following additional attributes as required:

1. Real World Units : If an input value depends on scene / geometry size then a unit attribute should always be added. For example if the graph represents floor tile, then to place it properly the size of the tile. A preset set of “distance” units is provided as part of the standard library.
2. Colorspace: If an input value is represented in a given color space then to support proper transformation into rendering color space this attribute should be set. A preset set of colorspace names conforming to the AcesCg 1.2 configuration is provided as part of the standard library.

Though not strictly required it is very useful to have appropriate default values as without these the defaults will be zero values. Thus for example a “scale” attribute for a texture node should not be left to default to zero.


### Publishing Definitions

From a &lt;nodegraph> a definition can be (new &lt;nodedef>) created or “published”. Publishing allows for the following important capabilities:

1. Reuse: The ability to reuse an unique node definition as opposed to duplicating graph implementations.
2. Variation: The ability to create or apply variations to an instance independent from the implementation.
3. Interoperability: Support definitions with common properties that are mutually understood by all consumers be exchanged.

In order to support these capabilities It is recommended that the following attributes always be specified:

1. A unique name identifier: This can follow the ND\_ and NG\_ convention described. It is recommended that the signature of the interface also be encoded to provide uniqueness, especially if the definition may be polymorphic.
2. A namespace identifier (\*): When checking uniqueness namespace is always used so it is not required to be part of the name identifier to provide uniqueness.
    * It should not be used as it will result in the namespace being prepended multiple times. E.g. a “foo” node with a namespace “myspace” has a unique identifier of “myspace:node”. If the node is named “myspace:node”, then the resulting identifier is “myspace:myspace:node”.
    * Note that import of a Document will prepend namespaces as required without namespace duplication.
3. A version identifier: While this can be a general string, it is recommended that this be a template with a specific format to allow for known increment numbering. E.g. The format may be “v#.#” to support minor and major versioning. This requires that only one out of all versions be tagged as the default version. Care should be taken to ensure this as the first one found will be used as the default.
4. A nodegroup identifier: This can be one mechanism used for definition organization, or for user interface presentation. It is also used for shader generation to some extent as it provides hints as to the type of node. For example &lt;image> nodes are in the “texture2d” node group.
5. A documentation string. Though not strictly required this provides some ability to tell what a node does, and can be used as a user interface helper. Currently there is no formatting associated with this but it is possible to embed a format.

Note that utilities which codify publishing logic are provided as part of the core distribution.

To support variation it is proposed that both &lt;token>s and &lt;variant>s be used.



1. Tokens: These allow for the sample “template” be used for filenames with having to create new definitions. This can reduce the number of required definitions but also reduce the number of presets required. For example tokens can be used to control the format, resolution of the desired filename identifier.
2. Variants and Variant Sets: There are no hard-and-fast “rules” for when to create a definition vs use a definition with variants but one possible recommendation is to use variants when there are no interface signature differences (_Discuss_?). Some advantages include the fact that variants may be packaged and deployed independent of definitions and/or new definitions do not need to be deployed per variation. Note that for now only value overrides are possible.


### Dependencies and Organization

The more definitions are added including those based on other definitions, the harder it can be to discover what definitions are required given documents with some set of node instances.

To support separability of dependents, the following logical high level semantic breakdown is proposed:



1. Static “Core” library definitions. These include stdlib, pbrlib and bxdf. The recommendation is to always load these in and assume that they exist. For separability, it is recommended that these all be stored in a single runtime Document.
2. Static custom library definitions. These are based on core libraries. The recommendation is to not directly reference any core libraries using the Xinclude mechanism. This can result in duplicate possibly conflicting definitions being loaded. The “upgrade” mechanism will ensure that all core and custom libraries are upgraded to the appropriate target version. For separability, it is recommended that these all be stored in a single runtime Document.
3. Dynamically created definitions.  If this capability is allowed then it can be useful to have a fixed set of locations that these definitions can update. This could be local to the user or to update an existing custom library.

Additional groupings can be added to provide semantic organization (such as by “nodegroup”) though the recommendation is that they live within a common library root or package.

For an asset with dependent resources there are many options. Two of which are:



1. Co-locate resources with the definition. This allows for easier “packaging” of a single asset such as for transmission purposes but this can require additional discovery logic to find resources associated with a definition and may result in duplication of resources.
2. Located in a parallel folder structure to definitions. The onus is to maintain this parallel structure but the search logic complexity is the same for resources as it is for definitions.

If a definition is a source code implementation, then additional path search logic is required for discoverability during generation.

The following search paths are available:



* MATERIALX_SEARCH_PATH: This environment variable is used as part of both definition and resource search (e.g. relative filename path support).
* Source code paths: This can be registered at time of code generation as part of the generation “context”. It is recommended to follow the source path locations which would be relative to any custom definitions, using the “language” identifier of the code generator to discover the appropriate source code files.

An example usage of pathing can be found in the sample Viewer. The logic is as follows:



* The module/binary path is set as the default “root” definition path. Additional definition roots are included via MATERIALX_SEARCH_PATH. 
* The set of roots are considered to be the parent of “resources” and “libraries” folders, for resource and definitions respectively.
* The search path root for resources would be the “`<rootpath>/resources`” by default. This allows for handling of resources which are part of an assets definition. For example a brick shader located at “`/myroot/shaders/brick.mtlx`” may have the brick textures referenced at location “`/myroot/textures/brick.exr`”. Setting a single search path to “`/myroot`” handles the “parallel” folder organization mentioned, with the relative reference being “`textures/brick.exr`”
* For any shader at a given path a path to that shader can be added when resolving dependent resources. This can be used to handle the co-located folder organization. For example the shader may reside in “`/myroot/shader/brick.mtlx`”, and the texture in “`/myroot/shader/textures/brick.exr`”. Setting a root to “`myroot/shader`” and a relative reference to “`textures/brick.exr`” will allow for proper discovery.

For runtime, it is recommended that instead of reading in documents that they be “imported”. This allows for mechanisms such as namespace handling as well as tagging of source location (“sourceURI”) to occur per document. At some point all dependent documents need to be merged into a single one as there is no concept of referenced in-memory documents. Tagging is a useful mechanism to allow filtering out / exclusion of definitions from the main document content. For example, the main document can be “cleared” out while retaining the library definitions.

As there may be definitions dependent on other definitions, it is never recommended to unload core libraries, and care be taken when unloading custom or dynamic libraries. It may be useful to re-load all definitions if there is a desire to unload any one library.

Note that code generation is context based. If the context is not cleared, then dependent source code implementations will be retained. It is recommended to clear implementations if definitions are unloaded.


### Deployment, Transmission, and Translation

Given a set of definitions it is useful to consider how it will be deployed.

Some deployments for which file access may be restricted or accessing many files is a performance issue, pre-package up definitions, source and associated resources may be required. For example, this is currently used for the sample Web viewer deployment.

Some deployments may not want to handle non-core definitions or may not be able to handle (complex) node graphs. Additionally the definition may be transmitted as a shader. Thus, when creating a new definition it is recommended to determine the level of support for:



1. Flattening: Can the definition be converted to a series of nodes which have source code implementations.
2. Baking: Can the definition be converted to an image.
3. Translation: Can the implementation be converted mapped / converted to another implementation which can be consumed.
4. Shader Reflection: Can the desired metadata be passed to the shader for introspection.

Additional metadata which is not a formal part of the specification may lead to the inability to be understood by consumers.

