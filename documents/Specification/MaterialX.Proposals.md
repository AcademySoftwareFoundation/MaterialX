<!-----
MaterialX Proposals v1.39
----->


# MaterialX: Proposed Additions and Changes

**Proposals for Version 1.39**  
September 15, 2024


# Introduction

The [MaterialX Specification](./MaterialX.Specification.md) has historically included descriptions of not just current functionality, but also forward-looking proposed functionality intended for eventual implementation.  We believe it will be beneficial to provide clarity on which functionality is currently supported in the library, and which sections document proposed additions.

As such, those forward-looking proposals have been moved from the formal Specification documents into this Proposed Additions and Changes document to be discussed and debated. These descriptions can then be migrated into the appropriate formal Specification document once actually implemented in the code base.  New proposals for changes and additions to MaterialX may be added to this document once a generally favorable consensus from the community is reached.



## Table of Contents

**[Introduction](#introduction)**  

**[Proposals: General](#propose-general)**  

**[Proposals: Elements](#propose-elements)**  

**[Proposals: Stdlib Nodes](#propose-stdlib-nodes)**  

**[Proposals: PBR Nodes](#propose-pbr-nodes)**  

**[Proposals: NPR Nodes](#propose-npr-nodes)**  


<p>&nbsp;<p><hr><p>

# Proposals: General<a id="propose-general"></a>


## Color Spaces

When the OCIO NanoColor library (provide link) becomes available, MaterialX should support the official colorspace names in that spec, with the current MaterialX colorspace names supported as aliases.

MaterialX should also support the following color spaces:
* `lin_rec2020`
* `g22_rec2020`



<p>&nbsp;<p><hr><p>

# Proposals: Elements<a id="propose-elements"></a>


### AOV Output Elements

(Summary for README.md: **New Support for Shader AOVs**

Previously, MaterialX used custom types with a structure of output variables to define shader AOVs.  But this approach was not very flexible and in fact had not been implemented.  In v1.39, nodegraph-based shader implementations can include new [&lt;aovoutput> elements](./MaterialX.Specification.md#aov-output-elements) to define AOVs which renderers can use to output additional channels of information in addition to the final shading result, while file-based &lt;implementation>s can similarly define AOVs using [&lt;aov> elements](./MaterialX.Specification.md#implementation-aov-elements).
)

A functional nodegraph with either a "shader" or "material"-semantic output type may contain a number of &lt;aovoutput> elements to declare arbitrary output variables ("AOVs") which the renderer can see and output as additional streams of information.  AOVoutputs must be of type float, color3 or vector3 for pre-shading "pattern" values, or BSDF or EDF for shader-node output values; the renderer is expected to extract the appropriate color-like information from BSDF and EDF types.  AOVs defined within a shader-semantic node instantiated within this functional nodegraph may be "passed along" and potentially renamed (but may not be modified or operated on in any way) by providing a sourceaov attribute in the &lt;aovoutput>.

```xml
  <aovoutput name="name" type="type" aovname="aovname"
             nodename="node_to_connect_to" [sourceaov="aovname"]/>
```

The attributes for &lt;aovoutput> elements are:

* name (string, required): a user-chosen name for this aov output definition element.
* type (string, required): the type of the AOV, which must be one of the supported types listed above.
* aovname (string, required): the name that the renderer should use for the AOV.
* nodename (string, required): the name of the node whose output defines the AOV value.
* sourceaov (string, optional): If nodename is a surfaceshader type, the name of the output AOV defined within nodename to pass along as the output AOV.  The type of the sourceaov defined within nodename must match the &lt;aovoutput> type.

Examples:

```xml
  <aovoutput name="Aalbedo" type="color3" aovname="albedo"
             nodename="coat_affected_diffuse_color"/>
  <aovoutput name="Adiffuse" type="BSDF" aovname="diffuse">
             nodename="diffuse_bsdf"/>
```

#### AovOutput Example

Example of using &lt;aovoutput> with sourceaov to forward AOVs from within an instantiation of a shader-semantic node; this assumes that &lt;standard_surface> has itself defined &lt;aovoutput>s for "diffuse" and "specular" AOVs:

```xml
  <nodegraph name="NG_basic_surface_srfshader" nodedef="ND_basic_surface_srfshader">
    <image name="i_diff1" type="color3">
      <input name="file" type="filename"
                 value="txt/[diff_map_effect]/[diff_map_effect].<UDIM>.tif"/>
    </image>
    <mix name="diffmix" type="color3">
      <input name="bg" type="color3" interfacename="diff_albedo"/>
      <input name="fg" type="color3" nodename="i_diff1"/>
      <input name="mix" type="float" interfacename="diff_map_mix"/>
    </mix>
    <standard_surface name="stdsurf1" type="surfaceshader">
      <input name="base_color" type="color3" nodename="diffmix"/>
      <input name="diffuse_roughness" type="float" interfacename="roughness"/>
      <input name="specular_color" type="color3" interfacename="spec_color"/>
      <input name="specular_roughness" type="float" interfacename="roughness"/>
      <input name="specular_IOR" type="float" interfacename="spec_ior"/>
    </standard_surface>
    <output name="out" type="surfaceshader" nodename="stdsurf1"/>
    <aovoutput name="NGAalbedo" type="color3" aovname="albedo" nodename="diffmix"/>
    <aovoutput name="NGAdiffuse" type="BSDF" aovname="diffuse" nodename="stdsurf1"
                  sourceaov="diffuse"/>
    <aovoutput name="NGAspecular" type="BSDF" aovname="specular" nodename="stdsurf1"
                  sourceaov="specular"/>
  </nodegraph>
```

Layered shaders or materials must internally handle blending of AOV-like values from source layers before outputting them as AOVs: there is currently no facility for blending AOVs defined within post-shading blended surfaceshaders.

Note: while it is syntactically possible to create &lt;aovoutput>s for geometric primitive values such as shading surface point and normal accessed within a nodegraph, it is preferred that renderers derive such information directly from their internal shading state or geometric primvars.



#### Implementation AOV Elements

An &lt;implementation> element with a file attribute defining an external compiled implementation of a surface shader may contain one or more &lt;aov> elements to declare the names and types of arbitrary output variables ("AOVs") which the shader can output to the renderer.  AOVs must be of type float, color3, vector3, BSDF or EDF.  Note that in MaterialX, AOVs for pre-shading "pattern" colors are normally of type color3, while post-shaded color-like values are normally of type BSDF and emissive color-like values are normally of type EDF.  An &lt;implementation> with a `nodegraph` attribute may not contain &lt;aov> elements; instead, &lt;aovoutput> elements within the nodegraph should be used.

```xml
  <implementation name="IM_basicsurface_surface_rmanris"
                  nodedef="ND_basic_surface_surface" implname="basic_srf"
                  target="rmanris" file="basic_srf.C">
    ...<inputs>...
    <aov name="IMalbedo" type="color3" aovname="albedo"/"/>
    <aov name="IMdiffuse" type="BSDF" aovname="diffuse"/"/>
  </implementation>
```



### Material Inheritance

Materials can inherit from other materials, to add or change shaders connected to different inputs; in this example, a displacement shader is added to the above "Mgold" material to create a new "Mgolddsp" material:

```xml
  <noise2d name="noise1" type="float">
    <input name="amplitude" type="float" value="1.0"/>
    <input name="pivot" type="float" value="0.0"/>
  </noise2d>
  <displacement name="stddsp" type="displacementshader">
    <input name="displacement" type="float" nodename="noise1"/>
    <input name="scale" tpe="float" value="0.1"/>
  </displacement>
  <surfacematerial name="Mgolddsp" type="material" inherit="Mgold">
    <input name="displacementshader" type="displacementshader" nodename="stddsp"/>
  </surfacematerial>
```

Inheritance of material-type custom nodes is also allowed, so that new or changed input values can be applied on top of those specified in the inherited material.


### Named Constants

The MaterialX data library contains many nodes that have inputs with a number of common semantic default values, for instance zero or one. The MaterialX data library also uses a number of different concrete types, for example `float`, `color3` or `vector4`. These types each have different concrete values to represent these semantic defaults. &lt;typedef> elements can be extended with child &lt;constant> elements defining these named values.

Attributes for &lt;constant> elements:

* `name` (string, required): a unique name for this &lt;constant> element
* `value` (string, required): the concrete value for the given type that should be used during the substitution.

```xml
  <typedef name="float">
    <constant name="zero" value="0.0"/>
    <constant name="one" value="1.0"/>
  </typedef>
  <typedef name="color3">
    <constant name="zero" value="0.0,0.0,0.0"/>
    <constant name="one" value="1.0,1.0,1.0"/>
  </typedef>
  <typedef name="vector4">
    <constant name="zero" value="0.0,0.0,0.0,0.0"/>
    <constant name="one" value="1.0,1.0,1.0,1.0"/>
  </typedef>
```

These named constants can be used anywhere a concrete value is accepted, ie. in any `value` or `default` attribute. The named constant is referenced by prefixing the name of the named value with `"Constant:"`.

```xml
  <input name="in1" type="color3" value="Constant:zero"/>
  <input name="in2" type="float" value="Constant:one"/>
```

This example, when combined with the example &lt;typedef> elements above, is functionally identical to:

```xml
  <input name="in1" type="color3" value="0.0,0.0,0.0"/>
  <input name="in2" type="float" value="1.0"/>
```

Not every &lt;typedef> is required to define each named constant. Expansion of a named constant will raise an error if a specified name is not defined for the given &lt;typedef>.

### Templated Definition Elements

To reduce potential repetitive data library elements, any number of elements can be enclosed within a &lt;template> element scope. The elements inside the &lt;template> scope are concretely instantiated using the provided template attributes.

Attributes for &lt;template> elements:

* `name` (string, required): a unique name for this &lt;template>
* `varnames` (string, required): the names of variables to be used for substitution in the contained elements as a comma-separated list.
* `options` (string, required): the set of concrete values to be used during the substitution of the specified `varnames` in elements contained within the &lt;template>, specified as a string of comma-separated lists of comma-separated strings surrounded by parentheses.

When a &lt;template> is expanded, all the child elements are instantiated exactly once for each entry in the `options` array. These instances replace the original &lt;template> element in the document. They are placed at the same scope level as the original &lt;template> element in the document.

Each instance of the contents contained inside the &lt;template> is processed as it is instantiated. This processing step inspects all attribute values in all the contained elements and replaces any occurrence of any specified string with the corresponding value from the `options` array. The specific string used for the string replacement matching is constructed by wrapping any of the values in the `varnames` list with `@` characters. For example, if `varnames` has the value `"foo"`, then the matching string would be `"@foo@"`. We surround the value of `varnames` with the `@` characters to ensure we do not have any accidental matches.

```xml
  <template name="TP_ND_multiply" varames="typeName" options="(float, color3, vector4)">
    <nodedef name="ND_multiply_@typeName@" node="multiply" nodegroup="math">
      <input name="in1" type="@typeName@" value="Value:zero" />
      <input name="in2" type="@typeName@" value="Value:one" />
      <output name="out" type="@typeName@" defaultinput="in1" />
    </nodedef>
  </template>
```

In this example, the provided `varnames` attribute is `"typeName"`, and so the matching string is `"@typeName@"`. Each instance of this string is then replaced by each element in the `options` string array, `"float"`, `"color3“`, and `"vector4"`.

```xml
  <nodedef name="ND_multiply_float" node="multiply" nodegroup="math">
    <input name="in1" type="float" value="Value:zero" />
    <input name="in2" type="float" value="Value:one" />
    <output name="out" type="float" defaultinput="in1" />
  </nodedef>
  <nodedef name="ND_multiply_color3" node="multiply" nodegroup="math">
    <input name="in1" type="color3" value="Value:zero" />
    <input name="in2" type="color3" value="Value:one" />
    <output name="out" type="color3" defaultinput="in1" />
  </nodedef>
  <nodedef name="ND_multiply_vector4" node="multiply" nodegroup="math">
    <input name="in1" type="vector4" value="Value:zero" />
    <input name="in2" type="vector4" value="Value:one" />
    <output name="out" type="vector4" defaultinput="in1" />
  </nodedef>
```

When used in combination with the [Named Constants](#named-constants) expansion, this example would become the following concrete set of fully expanded elements.

```xml
  <nodedef name="ND_multiply_float" node="multiply" nodegroup="math">
    <input name="in1" type="float" value="0.0" />
    <input name="in2" type="float" value="1.0" />
    <output name="out" type="float" defaultinput="in1" />
  </nodedef>
  <nodedef name="ND_multiply_color3" node="multiply" nodegroup="math">
    <input name="in1" type="color3" value="0.0,0.0,0.0" />
    <input name="in2" type="color3" value="1.0,1.0,1.0" />
    <output name="out" type="color3" defaultinput="in1" />
  </nodedef>
  <nodedef name="ND_multiply_vector4" node="multiply" nodegroup="math">
    <input name="in1" type="vector4" value="0.0,0.0,0.0,0.0" />
    <input name="in2" type="vector4" value="1.0,1.0,1.0,1.0" />
    <output name="out" type="vector4" defaultinput="in1" />
  </nodedef>
```

When multiple variable names are specified, each of those variables will be substituted simultaneously. This means that the list of options corresponding to each variable is required to be the same length.

```xml
<template name="TP_multivariable_example" varnames="numberName, numberValue" options="(one, two, three), (1.0, 2.0, 3.0)">
    <nodedef name="ND_constantFloat_@numberName@" node="example" nodegroup="math">
        <input name="in1" type="float" value="@numberValue@" />
        <output name="out" type="float" defaultinput="in1" />
    </nodedef>
</template>
```

If there is a mismatch in the lengths of the options lists, then an error will be raised during expansion. The example below demonstrates different lengths of options lists; this is invalid syntax.

```xml
<!-- INVALID EXAMPLE - DO NOT COPY -->
<template name="TP_invalid_example" varnames="numberName, numberValue" options="(one, two, three, four), (1.0, 2.0)">
    <nodedef name="ND_constantFloat_@numberName@" node="example" nodegroup="math">
        <input name="in1" type="float" value="@numberValue@" />
        <output name="out" type="float" defaultinput="in1" />
    </nodedef>
</template>
```

The `TP_multivariable_example` would be expanded to three concrete instances of the elements it contains, and both `@numberName@` and `@numberValue@` strings would be replaced.

```xml
<nodedef name="ND_constantFloat_one" node="example" nodegroup="math">
    <input name="in1" type="float" value="1.0" />
    <output name="out" type="float" defaultinput="in1" />
</nodedef>
<nodedef name="ND_constantFloat_two" node="example" nodegroup="math">
    <input name="in1" type="float" value="2.0" />
    <output name="out" type="float" defaultinput="in1" />
</nodedef>
<nodedef name="ND_constantFloat_three" node="example" nodegroup="math">
    <input name="in1" type="float" value="3.0" />
    <output name="out" type="float" defaultinput="in1" />
</nodedef>
```

&lt;template> elements can be nested, and then are expanded sequentially, from the outer scope to the inner scope. This allows &lt;template> element variable substitution to be used to define other template attributes.

```xml
<template name="TP_ND_add_matrix" varnames="typeName" options="(matrix33, matrix44)">
    <template name="TP_ND_add_@typeName@FA" varnames="nodeDefExt,floatTypeName" options="(@typeName@,@typeName@FA), (@typeName@,float)">
        <nodedef name="ND_add_@nodeDefExt@" node="add" nodegroup="math">
            <input name="in1" type="@typeName@" value="Constant:one" />
            <input name="in2" type="@floatTypeName@" value="Constant:zero" />
            <output name="out" type="@typeName@" defaultinput="in1" />
        </nodedef>
    </template>
</template>
```

Here we have two nested &lt;template> elements. `TP_ND_add_matrix` will be expanded first. This is important as here the inner &lt;template> element uses the `@typeName@` template variable reference in its definition. This first &lt;template> would be expanded resulting in two instances of the contained elements, since the options list `(matrix33, matrix44)` has length two.

```xml
<template name="TP_ND_add_matrix33FA" varnames="nodeDefExt,floatTypeName" options="(matrix33,matrix33FA), (matrix33,float)">
    <nodedef name="ND_add_@nodeDefExt@" node="add" nodegroup="math">
        <input name="in1" type="matrix33" value="Constant:one" />
        <input name="in2" type="@floatTypeName@" value="Constant:zero" />
        <output name="out" type="matrix33" defaultinput="in1" />
    </nodedef>
</template>
<template name="TP_ND_add_matrix44FA" varnames="nodeDefExt,floatTypeName" options="(matrix44,matrix44FA), (matrix44,float)">
    <nodedef name="ND_add_@nodeDefExt@" node="add" nodegroup="math">
        <input name="in1" type="matrix44" value="Constant:one" />
        <input name="in2" type="@floatTypeName@" value="Constant:zero" />
        <output name="out" type="matrix44" defaultinput="in1" />
    </nodedef>
</template>
```

This results in two "inner" scoped &lt;template> elements, `TP_ND_add_matrix33FA` and `TP_ND_add_matrix44FA`. These then both each get expanded and both result in a further two instances of each set of contained elements, because all the options lists for each of the variables are also of length two, `(matrix33,matrix33FA)` and `(matrix33,float)`.

```xml
<nodedef name="ND_add_matrix33" node="add" nodegroup="math">
    <input name="in1" type="matrix33" value="Constant:one" />
    <input name="in2" type="matrix33" value="Constant:zero" />
    <output name="out" type="matrix33" defaultinput="in1" />
</nodedef>
<nodedef name="ND_add_matrix33FA" node="add" nodegroup="math">
    <input name="in1" type="matrix33" value="Constant:one" />
    <input name="in2" type="float" value="Constant:zero" />
    <output name="out" type="matrix33" defaultinput="in1" />
</nodedef>
<nodedef name="ND_add_matrix44FA" node="add" nodegroup="math">
    <input name="in1" type="matrix44" value="Constant:one" />
    <input name="in2" type="matrix44" value="Constant:zero" />
    <output name="out" type="matrix44" defaultinput="in1" />
</nodedef>
<nodedef name="ND_add_matrix44" node="add" nodegroup="math">
    <input name="in1" type="matrix44" value="Constant:one" />
    <input name="in2" type="float" value="Constant:zero" />
    <output name="out" type="matrix44" defaultinput="in1" />
</nodedef>
```
<p>&nbsp;<p><hr><p>

# Proposals: Stdlib Nodes<a id="propose-stdlib-nodes"></a>


### Procedural Nodes

<a id="node-tokenvalue"> </a>

* **`tokenvalue`**: a constant "interface token" value, may only be connected to &lt;token>s in nodes, not to &lt;input>s.
    * `value` (any uniform non-shader-semantic type): the token value to output; "enum" and "enumvalues" attributes may be provided to define a specific set of allowed token values.



### Noise Nodes

<a id="node-cellnoise1d"> </a>

1D Cell noise was proposed an an alternative approach to random value generation.

* **`cellnoise1d`**: 1D cellular noise, 1 or 3 channels (type float or vector3).
    * `period` (float or vector3): the positive integer distance at which the noise function returns the same value for input coordinate repeated at that step.  Default is 0, meaning the noise is not periodic.
    * `in` (float): the 1D coordinate at which the noise is evaluated.

<a id="node-worleynoise2d"> </a>

Expanded 2D Worley noise to support different distance metrics and periodicity. 

* **`worleynoise2d`**: 2D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).
    * `metric` (uniform string): the distance metric to return, one of "distance" (Euclidean distance to feature), "distance2" (Euclidean distance squared), "manhattan" or "chebyshev".  Default is "distance".
    * `period` (float or vector3): the positive integer distance at which the noise function returns the same value for texture coordinates repeated at that step.  Default is 0, meaning the noise is not periodic.

<a id="node-worleynoise3d"> </a>

Expanded 3D Worley noise to support different distance metrics and periodicity.

* **`worleynoise3d`**: 3D Worley noise using centered jitter, outputting float (distance metric to closest feature), vector2 (distance metrics to closest 2 features) or vector3 (distance metrics to closest 3 features).
    * `metric` (uniform string): the distance metric to return, one of "distance" (Euclidean distance to feature), "distance2" (Euclidean distance squared), "manhattan" or "chebyshev".  Default is "distance".
    * `period` (float or vector3): the positive integer distance at which the noise function returns the same value for position coordinates repeated at that step.  Default is 0, meaning the noise is not periodic.

#### Periodic Noises

In #1201 it was decided that separate periodic versions of all of the noises is preferred to adding it to the existing noises.

### Shape Nodes




### Geometric Nodes

<a id="node-bump"> </a>

* **`bump`**: Existing node, proposal to add a vector3 `bitangent` input

Note: when &lt;geompropvalueuniform> is added, the text in the first paragraph of the Specification about Node Inputs should be revised to include "&lt;geompropvalueuniform>" as an example of "or any other node whose output is explicitly declared to be uniform".


### Global Nodes

<a id="node-ambientocclusion"> </a>

* **`ambientocclusion`**: Compute the ambient occlusion at the current surface point, returning a scalar value between 0 and 1.  Ambient occlusion represents the accessibility of each surface point to ambient lighting, with larger values representing greater accessibility to light.  This node must be of type float.
    * `coneangle` (float): the half-angle of a cone about the surface normal, within which geometric surface features are considered as potential occluders.  The unit for this input is degrees, and its default value is 90.0 (full hemisphere).
    * `maxdistance` (float): the maximum distance from the surface point at which geometric surface features are considered as potential occluders.  Defaults to 1e38, e.g. "unlimited".



### Application Nodes

<a id="node-updirection"> </a>

* **`updirection`**: the current scene "up vector" direction, as defined by the shading environment.  This node must be of type vector3.
    * `space` (uniform string):  the space in which to return the up vector direction, defaults to "world". 



### Math Nodes

<a id="node-transformcolor"> </a>

* **`transformcolor`**: transform the incoming color from one specified colorspace to another, ignoring any colorspace declarations that may have been provided upstream.  For color4 types, the alpha channel value is unaffected.
    * `in` (color3 or color4): the input color.
    * `fromspace` (uniform string): the name of a standard colorspace or a colorspace understood by the application to transform the `in` color from; may be empty (the default) to specify the document's working colorspace.
    * `tospace` (uniform string): the name of a standard colorspace or a colorspace understood by the application to transform the `in` color to; may be empty (the default) to specify the document's working colorspace.

<a id="node-triplanarblend"> </a>

* **`triplanarblend`** (NG): samples data from three inputs, and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.
    * `inx` (float or colorN): the image to be projected in the direction from the +X axis back toward the origin.  Default is 0 in all channels.
    * `iny` (float or colorN): the image to be projected in the direction from the +Y axis back toward the origin with the +X axis to the right.  Default is 0 in all channels.
    * `inz` (float or colorN): the image to be projected in the direction from the +Z axis back toward the origin.  Default is 0 in all channels.
    * `position` (vector3): a spatially-varying input specifying the 3D position at which the projection is evaluated.  Default is to use the current 3D object-space coordinate.
    * `normal` (vector3): a spatially-varying input specifying the 3D normal vector used for blending.  Default is to use the current object-space surface normal.
    * `blend` (float): a 0-1 weighting factor for blending the three axis samples using the geometric normal, with higher values giving softer blending.  Default is 1.0.
    * `filtertype` (uniform string): the type of texture filtering to use; standard values include "closest" (nearest-neighbor single-sample), "linear", and "cubic".  If not specified, an application may use its own default texture filtering method.



### Adjustment Nodes

<a id="node-curveinversecubic"> </a>

* **`curveinversecubic`**: remap a 0-1 input float value using an inverse Catmull-Rom spline lookup on the input `knots` values.  Outputs a 0-1 float interpolant value.
    * `in` (float): the input value or nodename
    * `knots` (uniform floatarray): the list of non-uniformly distributed input values defining the curve for the remapping.  At least 2 values must be provided, and the first and last knot have multiplicity 2.

<a id="node-curveuniformlinear"> </a>

* **`curveuniformlinear`**: output a float, color<em>N</em> or vector<em>N</em> value linearly interpolated between a number of `knotvalues` values, using the value of `in` as the interpolant.
    * `in` (float): the input interpolant value or nodename
    * `knotvalues` (uniform floatarray or color<em>N</em>array or vector<em>N</em>array): the array of at least 2 values to interpolate between.

<a id="node-curveuniformcubic"> </a>

* **`curveuniformcubic`**: output a float, color<em>N</em> or vector<em>N</em> value smoothly interpolated between a number of `knotvalues` values using a Catmull-Rom spline with the value of `in` as the interpolant.
    * `in` (float): the input interpolant value or nodename
    * `knotvalues` (uniform floatarray or color<em>N</em>array or vector<em>N</em>array): the array of at least 2 values to interpolate between.

<a id="node-curveadjust"> </a>

* **`curveadjust`** (NG): output a smooth remapping of input values using the centripetal Catmull-Rom cubic spline curve defined by specified knot values, using an inverse spline lookup on input knot values and a forward spline through output knot values.  All channels of the input will be remapped using the same curve.
    * `in` (float or colorN or vectorN): the input value or nodename
    * `numknots` (uniform integer): the number of values in the knots and knotvalues arrays
    * `knots` (uniform floatarray): the list of input values defining the curve for the remapping.  At least 2 and at most 16 values must be provided.
    * `knotvalues` (uniform floatarray): the list of output values defining the curve for the remapping.  Must be the same length as knots.

<a id="node-curvelookup"> </a>

* **`curvelookup`** (NG): output a float, colorN or vectorN value smoothly interpolated between a number of knotvalue values, using the position of in within knots as the knotvalues interpolant.
    * `in` (float): the input interpolant value or nodename
    * `numknots` (uniform integer): the number of values in the knots and knotvalues arrays
    * `knots` (uniform floatarray): the list of knot values to interpolate in within.  At least 2 and at most 16 values must be provided.
    * `knotvalues` (uniform floatarray or colorNarray or vectorNarray): the values at each knot position to interpolate between. Must be the same length as knots.



### Compositing Nodes



### Conditional Nodes

<a id="node-ifelse"> </a>

* **`ifelse`**: output the value of one of two input streams, according to whether the value of a boolean selector input is true or false
    * `infalse`, `intrue` (float or color<em>N</em> or vector<em>N</em>): the values or nodenames to select from based on the value of the `which` input.  The types of the various `in<em>N</em>` inputs must match the type of the `switch` node itself.  The default value of all `in<em>N</em>` inputs is 0.0 in all channels.
    * `which` (boolean): a selector to choose which input to take values from; default is "false".



### Channel Nodes

<a id="node-extractrowvector"> </a>

* **`extractrowvector`**: extract the specified row vector number from a matrix<em>N</em> stream.
    * `in` (matrix<em>N</em>): the input value or nodename
    * `index` (integer): the row number to extract, should be 0-2 for matrix33 streams, or 0-3 for matrix44 streams.

<a id="node-separatecolor4"> </a>

* **`separatecolor4`** (NG): output the RGB and alpha channels of a color4 as separate outputs.
    * `in` (color4): the input value or nodename
    * `outcolor` (output, color3): the RGB channel values.
    * `outa` (output, float): the value of the alpha channel.



<p>&nbsp;<p><hr><p>

# Proposals: PBR Nodes<a id="propose-pbr-nodes"></a>



<p>&nbsp;<p><hr><p>

# Proposals: NPR Nodes<a id="propose-npr-nodes"></a>

