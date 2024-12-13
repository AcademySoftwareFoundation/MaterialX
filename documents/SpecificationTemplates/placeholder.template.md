## Place holder for specification document

To generate the markdown table for the node ports just add the string `@MX_TABLE_<name>@` in the markdown file.


## Texture Nodes

### image
Samples data from a single image, or from a layer within a multi-layer image.

@MX_TABLE_image@

---

### tiledimage
Samples data from a single image, with provisions for tiling and offsetting the image across uv space.

@MX_TABLE_tiledimage@

---

### latlongimage
Samples an equiangular map along a view direction with adjustable latitudinal offset.

@MX_TABLE_latlongimage@

---

### triplanarprojection
Samples data from three images (or layers within multi-layer images), and projects a tiled representation of the images along each of the three respective coordinate axes, computing a weighted blend of the three samples using the geometric normal.

@MX_TABLE_triplanarprojection@


## Procedural Nodes

### constant
Outputs a constant value.

@MX_TABLE_constant@

The output, `out`, of this node corresponds directly to its input, `value`, in both value and type.

$$out = value$$

---

### ramplr
A left-to-right linear value ramp

@MX_TABLE_ramplr@

The ramp interpolation is calculated using the variable `U`, which corresponds to the `x` co-ordinate of the `texcoord` input.

$$U = Clamp(texcoord.x, 0.0, 1.0)$$

By default the first set of UV coordinates will be used for `texcoord`.

The output, `out`, of this node is the linear interpolation of `value_l` and `value_r` with respect to interpolant parameter `U`:

\[
\begin{align*}
U &= \text{Clamp}(\text{texcoord}_x, 0.0, 1.0) \\
\text{out}_i &= \text{value}_{r_i} \cdot U + \text{value}_{l_i} \cdot (1 - U)
\end{align*}
\]

---

### ramptb
A top-to-bottom linear value ramp.

@MX_TABLE_ramptb@

The ramp interpolation is calculated using the variable `V`, which corresponds to the `y` co-ordinate of the `texcoord` input.

$$V = Clamp(texcoord.y, 0.0, 1.0)$$

By default the first set of UV coordinates will be used for `texcoord`.

The output, `out`, of this node is the linear interpolation of `value_l` and `value_r` with respect to interpolant parameter `V`:

\[
\begin{align*}
V &= \text{Clamp}(\text{texcoord}_y, 0.0, 1.0) \\
\text{out}_i &= \text{value}_{r_i} \cdot V + \text{value}_{l_i} \cdot (1 - V)
\end{align*}
\]

---

### ramp4
A 4-corner bilinear value ramp.

@MX_TABLE_ramp4@


The output, `out`, is given by a bilinear interpolation of `valuetl`, `valuetr`, `valuebl`, `valuebr`, with respect to interpolant parameters derived from the `texcoord` parameter


\[
\begin{align*}
S &= \text{Clamp}(\text{texcoord}_x, 0.0, 1.0) \\
T &= \text{Clamp}(\text{texcoord}_y, 0.0, 1.0) \\
\text{Top}_{\text{mix}} &= \text{Mix}(\text{valuetl}, \text{valuetr}, S) \\
\text{Bottom}_{\text{mix}} &= \text{Mix}(\text{valuebl}, \text{valuebr}, S) \\
\text{out} &= \text{Mix}(\text{Top}_{\text{mix}}, \text{Bottom}_{\text{mix}}, T)
\end{align*}
\]

---

### splitlr
A left-right split matte, split at a specified `U` value.

@MX_TABLE_splitlr@

The output, `out`, is given by linear interpolation of `value_l` and `value_r` with respect to linear interpolant, `T`, given by:

\[
\begin{align*}
T &= \text{aastep}(\text{center}, \text{texcoord}_x) \\
\text{out}_i &= \text{value}_{r_i} \cdot T + \text{value}_l \cdot (1 - T)
\end{align*}
\]

---

### splittb
A top-bottom split matte, split at a specified `V`` value.

@MX_TABLE_splittb@

The output, `out`, is given by linear interpolation of `value_l` and `value_r` with respect to linear interpolant, `T`, given by:

\[
\begin{align*}
T &= \text{aastep}(\text{center}, \text{texcoord}_y) \\
\text{out}_i &= \text{value}_{r_i} \cdot T + \text{value}_l \cdot (1 - T)
\end{align*}
\]

---

### noise2d
2D Perlin noise in 1, 2, 3 or 4 channels.

@MX_TABLE_noise2d@

---

### noise3d
@MX_TABLE_noise3d@

---

### fractal3d
@MX_TABLE_fractal3d@

---

### cellnoise2s
@MX_TABLE_cellnoise2d@

---

### cellnoise3d
@MX_TABLE_cellnoise3d@

---

### worleynoise2d
@MX_TABLE_worleynoise2d@

---

### worleynoise3d
@MX_TABLE_worleynoise3d@

---

### unifiednoise2d
@MX_TABLE_unifiednoise2d@

The `type` input must be used to select an underlying noise to evaluate.

| `type` | `node`           |
|--------|------------------|
| 0      | `noise2d`        |
| 1      | `cellnoise2d`    |
| 2      | `worleynoise2d`  |
| 3      | `fractalnoise3d` |

** NOTE there is currently no `fractalnoise2d` - texcoord is promoted to position**

The `texcoord` input must be scaled by the `freq` input value and then have the `offset` input value added to it.  This computed result is then used as the `texcoord` input in the corresponding noise type.

$$computedTexcoord_i = (texcoord_i * freq_i) + offset_i$$

The `jitter` input must be used as the `jitter` input of the `worleynoise2d` nose if that noise type is selected corresponding input

The `octaves`, `lacunarity` and `diminish` inputs must be used as the corresponding inputs of the `fractalnoise2d` node if that type is selected.

The `out` output must be computed by taking the computed result from the selected noise type. That value must then remapped to the range defined by the `outmin` and `outmax` inputs, and if the `clampoutput` input is true, then the remapped value must be clamped between these two values.

$$remapped = computed * (outmax - outmin) + outmin$$

$$out =
\begin{cases}
remapped       & \text{if } clampoutput \text{ is } false \text{ or } remapped \in [outmin, outmax]\\
outmin         & \text{if } clampoutput \text{ is } true \text{ and } remapped < outmin\\
outmax         & \text{if } clampoutput \text{ is } true \text{ and } remapped > outmax\\
\end{cases}$$

---

### unifiednoise3d
@MX_TABLE_unifiednoise3d@

The `type` input must be used to select an underlying noise to evaluate. 

| `type` | `node`           |
|--------|------------------|
| 0      | `noise3d`        |
| 1      | `cellnoise3d`    |
| 2      | `worleynoise3d`  |
| 3      | `fractalnoise3d` |

The `position` input must be scaled by the `freq` input value and then have the `offset` input value added to it.  This computed result is then used as the `position` input in the corresponding noise type.

$$computedPosition_i = (position_i * freq_i) + offset_i$$

The `jitter` input must be used as the `jitter` input of the `worleynoise3d` nose if that noise type is selected corresponding input 

The `octaves`, `lacunarity` and `diminish` inputs must be used as the corresponding inputs of the `fractalnoise3d` node if that type is selected.

The `out` output must be computed by taking the computed result from the selected noise type. That value must then remapped to the range defined by the `outmin` and `outmax` inputs, and if the `clampoutput` input is true, then the remapped value must be clamped between these two values.

$$remapped = computed * (outmax - outmin) + outmin$$

$$out =
\begin{cases}
remapped       & \text{if } clampoutput \text{ is } false \text{ or } remapped \in [outmin, outmax]\\
outmin         & \text{if } clampoutput \text{ is } true \text{ and } remapped < outmin\\
outmax         & \text{if } clampoutput \text{ is } true \text{ and } remapped > outmax\\
\end{cases}$$

---

### randomfloat:
Produces a stable randomized float value between 'min' and 'max', based on an 'input' signal and 'seed' value. Uses a 2d cellnoise function to produce the output.

@MX_TABLE_randomfloat@

\[
\begin{aligned}
s &= \text{float}(\text{seed}) \\
x &= \begin{cases}
    \text{in} \cdot 4096, & \text{if in is float} \\
    \text{float}(\text{in}), & \text{if in is integer}
\end{cases} \\
\vec{v} &= (x, s) \\
n &= \text{cellnoise2d}(\vec{v}) \quad \text{where } n \in [0, 1] \\
\text{out} &= \min + n \cdot (\max - \min)
\end{aligned}
\]

---

### randomcolor: 
Produces a randomized RGB color within a randomized hue, saturation and brightness range, based on an 'input' signal and 'seed' value. Output type color3.

@MX_TABLE_randomcolor@

The output, `out`, is a random color computed by the method below:

\[
\begin{aligned}
\text{seed}_f &= \text{float}(\text{seed}) \\
\text{seed}_\text{hue} &= \left\lceil \text{seed}_f + 413.3 \right\rceil \\
\text{seed}_\text{sat} &= \left\lceil \text{seed}_f + 1522.4 \right\rceil \\
\text{seed}_\text{val} &= \left\lceil \text{seed}_f + 1813.8 \right\rceil \\
\text{rand}_\text{hue} &= \text{RandomFloat}(\text{input}, \text{seed}_\text{hue}) \\
\text{rand}_\text{sat} &= \text{RandomFloat}(\text{input}, \text{seed}_\text{sat}) \\
\text{rand}_\text{val} &= \text{RandomFloat}(\text{input}, \text{seed}_\text{val}) \\
\text{hue} &= \text{hue}_{\text{low}} + \text{rand}_\text{hue} \cdot (\text{hue}_{\text{high}} - \text{hue}_{\text{low}}) \\
\text{sat} &= \text{sat}_{\text{low}} + \text{rand}_\text{sat} \cdot (\text{sat}_{\text{high}} - \text{sat}_{\text{low}}) \\
\text{val} &= \text{val}_{\text{low}} + \text{rand}_\text{val} \cdot (\text{val}_{\text{high}} - \text{val}_{\text{low}}) \\
\text{out} &= \text{HSVtoRGB}(\text{hue}, \text{sat}, \text{val})
\end{aligned}
\]

## Shape Nodes

### checkerboard
2D checkerboard pattern.


@MX_TABLE_checkerboard@
Draws a checkerboard pattern
[there will be a vector illustration figure here]

---

### line
2D line pattern.


@MX_TABLE_line@

Returns 1 if texcoord is at less than radius distance from a line segment defined by point1 and point2; otherwise returns 0. Note that this makes the shapes at the end
of the lines semi-circles.
[there will be a vector illustration figure here]

---

### circle
2D circle(disk) pattern.

@MX_TABLE_circle@

Returns 1 if texcoord is inside a circle defined by center and radius; otherwise returns 0
[there will be a vector illustration figure here]

---

### cloverleaf
2D cloverleaf pattern: four semicircles on the edges of a square defined by center and radius.

@MX_TABLE_cloverleaf@

Returns 1 if texcoord is inside a cloverleaf shape described by four semicircles on the edges of a square defined by center and radius; otherwise returns 0.
[there will be a vector illustration figure here]

---

### hexagon
2D hexagon pattern.

@MX_TABLE_hexagon@

Returns 1 if texcoord is inside a hexagon shape inscribed by a circle defined by center and radius; otherwise returns 0.
[there will be a vector illustration figure here]

---

### grid
Creates a grid pattern of (1, 1, 1) white lines on a (0, 0, 0) black background with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

@MX_TABLE_grid@

---

### crosshatch
Creates a crosshatch pattern with the given tiling, offset, and line thickness. Pattern can be regular or staggered.

@MX_TABLE_crosshatch@

---

### tiledcircles
Creates a black and white pattern of circles with a defined tiling and size (diameter). Pattern can be regular or staggered.

@MX_TABLE_tiledcircles@

---

### tiledcloverleafs
Creates a black and white pattern of cloverleafs with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

@MX_TABLE_tiledcloverleafs@

---

### tiledhexagons
Creates a black and white pattern of hexagons with a defined tiling and size (diameter of the circles circumscribing the shape). Pattern can be regular or staggered.

@MX_TABLE_tiledhexagons@

## Geometric Nodes

### position
The coordinates associated with the currently-processed data, as defined in a specific coordinate space.


@MX_TABLE_position@


Outputs the three-dimensional coordinate (x,y,z) of the currently processed data.

---

### normal
The normalized geometric normal associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_normal@

Outputs the normalized observer/ray facing normal. If the geometry has texture coordinates, this is equal to the normalized cross product of tangent and bitangent.
When going between the different coordinate spaces, the transformation matrices of the spaces are applied to the normal as their inverse transpose
(or equivalently, transpose inverse), with the exception of the model space, which

---

### tangent
The geometric tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_tangent@


Outputs the un-normalized partial derivative of "position" with respect to the first element of the texture coordinate touple for the given texture index. If there are no
texture coordinates on the geometry, or the texture coordinate index is out of range, the output falls back to the default vector3(0,0,0). Note that the tangent is
not necessarily orthogonal to the "bitangent".
For curves, tangent goes along the length of the curve.

---

### bitangent
The geometric bi-tangent vector associated with the currently-processed data, as defined in a specific coordinate space.

@MX_TABLE_bitangent@

Outputs the un-normalized partial derivative at "position" with respect to the second element 'v' in the texture coordinate touple ('u','v'). If there are no
texture coordinates on the geometry, or the texture coordinate index is out of range, the output falls back to the default vector3(0,0,0). For curves, bitangent goes across the width of the curve.

---

### bump
The normalized normal computed by offsetting the surface world space position along its world space normal.

@MX_TABLE_bump@

Outputs the un-normalized partial derivative at "position" with respect to the second element 'v' in the texture coordinate touple ('u','v'). If there are no
texture coordinates on the geometry, or the texture coordinate index is out of range, the output falls back to the default vector3(0,0,0). For curves, bitangent goes across the width of the curve.

---

### texcoord
The 2D or 3D texture coordinates associated with the currently-processed data

@MX_TABLE_texcoord@


---


### geomcolor
The color associated with the current geometry at the current position, generally bound via per-vertex color values. The type must match the type of the "color" bound to the geometry.

@MX_TABLE_geomcolor@

---

### geompropvalue
The value of the specified varying geometric property (defined using <geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

@MX_TABLE_geompropvalue@

---

### geompropvalueuniform
The value of the specified uniform geometric property (defined using <geompropdef>) of the currently-bound geometry. This node's type must match that of the referenced geomprop.

@MX_TABLE_geompropvalueuniform@

---

### Geometric Spaces

There are two types of spaces: affine spaces are defined by an affine linear transform (scale, rotate, translate, shear), and mapping spaces that maps each point

|Space            | Description                                       |
|-----------------|---------------------------------------------------|
|"model"          | optional mapping space
|"object"         | position on the deformed (displaced and subdivided) geometry before any transforms are applied |
|"world"          | position in object space with all transforms applied|

## Application Nodes

### frame
The current frame number as defined by the host environment.

@MX_TABLE_frame@

Applications may use any appropriate method to communicate the current frame number to the node implementation, whether via an internal state variable, a custom input, or other approach.

### time

The current time in seconds, as defined by the host environment.

@MX_TABLE_time@

Applications may use any appropriate method to communicate the current time to the node implementation, whether via an internal state variable, a custom input, dividing the current frame number by a local "frames per second" value, or other method; real-time applications may return some variation of wall-clock time.


## Math Nodes

### add
Add a value to the incoming float/color/vector/matrix

@MX_TABLE_add@

`out` shall be computed as the component-wise addition of `in1` and `in2`

```math
\mathtt{out}_{i,j} = \mathtt{in1}_{i,j} + \mathtt{in2}_{i,j} 
```

---

### subtract
Subtract a value from the incoming float/color/vector/matrix

@MX_TABLE_subtract@

`out` shall be computed as the component-wise subtraction of `in2` from `in1`

```math
\mathtt{out}_{i,j} = \mathtt{in2}_{i,j} - \mathtt{in1}_{i,j} 
```

---

### multiply
Multiply two values together. Scalar and vector types multiply component-wise, while matrices multiply with the standard matrix product.

@MX_TABLE_multiply@

For scalar and vector types, `out` shall be computed as the component-wise multiplication of `in1` with `in2`

```math
\mathtt{out}_{i} = \mathtt{in1}_{i} * \mathtt{in2}_{i} 
```

For matrix types, `out` shall be computed as the matrix inner product.

---

### divide
Divide one value by another. Scalar and vector types divide component-wise, while for matrices `in1` is multiplied with the inverse of `in2`.

@MX_TABLE_divide@

For scalar and vector types, `out` shall be computed as the component-wise division of `in2` by `in1`

```math
\mathtt{out}_{i} = \mathtt{in1}_{i} / \mathtt{in2}_{i} 
```

For matrix types, `out` shall be computed as the matrix inner product of `in1` and the inverse of `in2`.

```math
\mathtt{out} = \mathtt{in}_1 \mathtt{in}_2^{-1}
```

---

### modulo
The remaining fraction after dividing an incoming float/color/vector by a value and subtracting the integer portion. Modulo always returns a non-negative result.

@MX_TABLE_modulo@

`out` shall be computed as the absolute value of the component-wise modulo of `in1` and `in2`
```math
\mathtt{out}_{i} = \lvert \mathtt{in1}_{i} \space mod \space \mathtt{in2}_{i} \rvert
```

---

### fract
Returns the fractional part of the floating-point input.

@MX_TABLE_fract@

`out` shall be computed component-wise as the fractional part of `in1`.
```math
\begin{align}
x &= \lvert \mathtt{in}_{i} \rvert \\
\mathtt{out}_{i} &= x - \lfloor x \rfloor \\
\end{align}
```

---

### invert
subtract the incoming float, color, or vector from `amount` in all channels, outputting: `amount - in`.

@MX_TABLE_invert@

`out` shall be computed as `in` subtracted from `amount`:
```math
\mathtt{out}_{i} = \mathtt{amount}_{i} - \mathtt{in}_{i}
```

---

### absval
The per-channel absolute value of the incoming float/color/vector.

@MX_TABLE_absval@
                                                                  
`out` shall be computed as the channel-wise absolute value of `in`:
```math
\mathtt{out}_{i} = \lvert \mathtt{in}_{i} \rvert
```

---

### floor
The per-channel nearest integer value less than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the floor(float) also has a variant outputting an integer type.


@MX_TABLE_floor@

`out` shall be computed as the channel-wise floor of `in`:
```math
\mathtt{out}_{i} = \lfloor \mathtt{in}_{i} \rfloor
```

---

### ceil
The per-channel nearest integer value greater than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the ceil(float) also has a variant outputting an integer type.

@MX_TABLE_ceil@

`out` shall be computed as the channel-wise ceil of `in`:

```math
\mathtt{out}_{i} = \lceil \mathtt{in}_{i} \rceil
```

---

### round
Round each channel of the incoming float/color/vector values to the nearest integer value.

@MX_TABLE_round@
                                                                 
Round each channel of `in` to the nearest whole number, storing the result in the corresponding channel of `out`. Rounding shall be calculated using IEEE 754 round-to-nearest-ties-away-from-zero mode.

```math
\mathtt{out}_{i} = \lfloor \mathtt{in}_{i} \rceil
```

---

### power
Raise incoming float/color values to the specified exponent, commonly used for "gamma" adjustment.

@MX_TABLE_power@

`out` shall be computed as the component-wise raising of `in1` to the `in2`th power:

```math
\mathtt{out}_{i} = \mathtt{in1}_{i}^{\mathtt{in2}_{i}}
```

---

### safepower
Raise incoming float/color values to the specified exponent. Negative "in1" values will result in negative output values.

@MX_TABLE_power@

`out` shall be computed as the component-wise raising of `in1` to the `in2`th power:

```math
\mathtt{out}_{i} = (sgn \space \mathtt{in1}_{i}) \mathtt{in1}_{i}^{\lvert \mathtt{in2}_{i} \rvert}
```

---

### sin
The sine of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_sin@

`out` shall be computed as the component-wise sin of `in`. `in` shall be in radians.

```math
\mathtt{out}_{i} = \sin{\mathtt{in}_{i}}
```
---

### cos
The cosine of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_cos@

`out` shall be computed as the component-wise cosine of `in`. `in` shall be in radians:

```math
\mathtt{out}_{i} = \cos{\mathtt{in}_{i}}
```
---

### tan
The tangent of the incoming value, which is expected to be expressed in radians.

@MX_TABLE_tan@

`out` shall be computed as the component-wise tangent of `in`. `in` shall be in radians:

```math
\mathtt{out}_{i} = \tan{\mathtt{in}_{i}}
```
---

### asin
The arcsine of the incoming value. The output will be expressed in radians.

@MX_TABLE_asin@

`out` shall be computed as the component-wise arcsine of `in`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \sin^{-1}{\mathtt{in}_{i}}
```
---

### acos
The arccosine of the incoming value. The output will be expressed in radians.

@MX_TABLE_acos@
                                                                            
`out` shall be computed as the component-wise arccosine of `in`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \cos^{-1}{\mathtt{in}_{i}}
```
---

### atan2
the arctangent of the expression (`iny`/`inx`). The output will be expressed in radians.

@MX_TABLE_atan2@

> [!WARNING]
> Need to define all the rules about 0 values?

`out` shall be computed as the component-wise arctangent of `iny` divided by `inx`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \tan^{-1}{\frac{\mathtt{iny}_{i}}{\mathtt{inx}_{i}}}
```

---

### sqrt
The square root of the incoming value. 

@MX_TABLE_sqrt@

`out` shall be computed as the component-wise square root of `in`.

```math
\mathtt{out}_{i} = \sqrt{\max(\mathtt{in}_{i}, 0)}
```

---

### ln
The natural logarithm of the incoming value. 

@MX_TABLE_ln@
                                                                                    
`out` shall be computed as the component-wise natural logarithm of `in`.

```math
\mathtt{out}_{i} = \ln{\mathtt{in}_{i}}
```

---

### exp
$e$ to the power of the incoming value.

@MX_TABLE_exp@
                                                                                    
`out` shall be computed as the component-wise raising of the base of natural logarithms, $e$, to the power `in`.

```math
\mathtt{out}_{i} = e^{\mathtt{in}_{i}}
```

---

### sign
The per-channel sign of the incoming float/color/vector value: -1 for negative, +1 for positive, or 0 for zero.

@MX_TABLE_sign@
                                                                  
`out` shall be computed as the channel-wise sign of `in`:
```math
\mathtt{out}_{i} = sgn \space \mathtt{in}_{i}
```

---

### clamp
Clamp incoming values per-channel to a specified range of float/color/vector values.

@MX_TABLE_clamp@

`out` shall be computed as the component-wise clamping of `in` between `low` and `high`. That is:

```math
\mathtt{out}_{i} = \max(\mathtt{low}_{i}, \min(\mathtt{high}_{i}, {\mathtt{in}_{i}}))
```
---

### min
Select the minimum of the two incoming values

@MX_TABLE_min@

`out` shall be computed as the component-wise minimum of `in1` and `in2`:
```math
\mathtt{out}_{i} = \min(\mathtt{in1}_{i}, \mathtt{in2}_{i})
```

---

### max
Select the maximum of the two incoming values

@MX_TABLE_max@

`out` shall be computed as the component-wise maximum of `in1` and `in2`:
```math
\mathtt{out}_{i} = \max(\mathtt{in1}_{i}, \mathtt{in2}_{i})
```

---

### normalize
Output the incoming vectorN stream normalized. 

@MX_TABLE_normalize@
                                                                                    
`out` shall be computed as `in` divided by the euclidean norm of `in`:

```math
\mathtt{out} = \frac{\mathtt{in}}{\lVert \mathtt{in} \rVert}
```

---

### magnitude
Output the float magnitude (vector length) of the incoming vectorN stream; cannot be used on float or colorN streams. Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.

@MX_TABLE_magnitude@
                                                                                    
`out` shall be computed as the euclidean norm of `in`:

```math
\mathtt{out} = \lVert \mathtt{in} \rVert = \sqrt{\sum_{i=0}^{N-1} {\mathtt{in}_i}^2}
```

---

### distance
Measures the distance between two points in 2D, 3D, or 4D.

@MX_TABLE_distance@

`out` shall be computed as the magnitude of the vector pointing from `in1` to `in2`, that is the euclidean norm of `in2 - in1`:

```math
\mathtt{out} = \lVert \mathtt{in2} - \mathtt{in1} \rVert
```

---

### dotproduct
Output the (float) dot product of two incoming vectorN streams; cannot be used on float or colorN streams.

@MX_TABLE_dotproduct@

`out` shall be computed as the dot (inner) product of `in1` and `in2`

```math
\mathtt{out} = \mathtt{in1} \cdot \mathtt{in2}
```

---

### crossproduct
Output the (vector3) cross product of two incoming vector3 streams; cannot be used on any other stream type. A disabled crossproduct node passes through the value of `in1` unchanged.

@MX_TABLE_crossproduct@

`out` shall be computed as the cross product of `in1` and `in2`:

```math
\mathtt{out} = \mathtt{in1} \times \mathtt{in2}
```
If the node is disabled, `out` shall be `in1`.

---

### transformpoint
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformpoint@


`in` shall be reinterpreted as a 4-dimensional row vector, _p_, by the addition of a homogeneous coordinate with a value of 1. _p_ shall be multipled with the matrix _M_.

The matrix, _M_, shall be computed by the renderer as the matrix necessary to transform from `fromspace` to `tospace`. If either of the named spaces are unknown to the renderer, or no suitable matrix can be computed, _M_ shall be the identity matrix.

_p'_ shall be computed as multiplication of _p_ with _M_.

`out` shall be computed from _p'_ by dividing the first three coordinates by the homogeneous coordinate:

```math
\begin{align}
p &= \lbrack \mathtt{in}_{0}, \mathtt{in}_{1}, \mathtt{in}_{2}, 1 \rbrack \\
p' &= p \cdot M \\
\mathtt{out} &= \frac{\lbrack p'_{0}, p'_{1}, p'_{2} \rbrack }{p'_{3}}
\end{align}
```

> [!WARNING]
> What did we decide about common vs world?

---

### transformvector
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformvector@

`in` shall be reinterpreted as a 4-dimensional row vector, _v_, by the addition of a homogeneous coordinate with a value of 0. _v_ shall be multipled with the matrix _M_.

The matrix, _M_, shall be computed by the renderer as the matrix necessary to transform from `fromspace` to `tospace`. If either of the named spaces are unknown to the renderer, or no suitable matrix can be computed, _M_ shall be the identity matrix.

_v'_ shall be computed as multiplication of _v_ with _M_.

`out` shall be computed from _v'_ by removing the homogeneous coordinate.

```math
\begin{align}

\vec{v} &= [\mathtt{in}_{0}, \mathtt{in}_{1}, \mathtt{in}_{2}, 1] \notag \\
\vec{v}' &= \vec{v} \cdot M \notag \\
\mathtt{out} &= [\vec{v}'_{0}, \vec{v}'_{1}, \vec{v}'_{2}] \notag

\end{align}
```

> [!WARNING]
> What did we decide about common vs world?

---

### transformnormal
Transform the incoming vector3 normal from one specified space to another; cannot be used on any other stream type.

@MX_TABLE_transformnormal@

---

### transformmatrix
Transform the incoming vectorN by the specified matrix.

@MX_TABLE_transformmatrix@

---

### normalmap
Transform a normal vector from the encoded tangent space to world space. The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.

@MX_TABLE_normalmap@

---

---

### creatematrix

Build a 3x3 or 4x4 matrix from three vector3 or four vector3 or vector4 inputs. A matrix44 may also be created from vector3 input values, in which case the fourth value will be set to 0.0 for `in1`-`in3`, and to 1.0 for `in4` when creating the matrix44.

@MX_TABLE_creatematrix@

---

### hextilednormalmap
Transform a normal vector from the encoded tangent space to world space. The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.

@MX_TABLE_hextilednormalmap@

---

### transpose
Transpose the incoming matrix

@MX_TABLE_transpose@
                                                                                   
`out` shall be computed as the transpose of `in`, that is:

```math
\mathrm{out} = \mathrm{in}^T
```

---

### determinant
Output the determinant of the incoming matrix.

@MX_TABLE_determinant@
                                                                                   
`out` shall be computed as the determinant of `in`, that is:

```math
\mathrm{out} = \det(\mathrm{in})
```

---

### invertmatrix
Invert the incoming matrix.

@MX_TABLE_invertmatrix@
                                                               
`out` shall be computed as the matrix inverse of `in`. If `in` is not invertible, every component out `out` shall be set to `NaN`.

---

### rotate2d
Rotate the incoming 2D vector about the origin.

@MX_TABLE_rotate2d@

`out` shall be computed as the right-handed rotation of `in` about the origin by `amount` degrees. That is, `in` shall be multiplied by the matrix:

```math
\begin{bmatrix}
\cos\theta & -\sin\theta \\
\sin\theta & \cos\theta
\end{bmatrix}
```

where:
```math
\theta = \frac{\mathrm{amount} \cdot \pi}{180}
```

---

### rotate3d
Rotate the incoming 3D vector about the specified unit axis vector.

@MX_TABLE_rotate3d@

`out` shall be computed as the conjugation of `in` with a unit quaternion `q`:

```math
\begin{align}
\boldsymbol{q} &= \alpha_0 \sin\frac{\theta}{2} \boldsymbol{i} + \alpha_1 \sin\frac{\theta}{2} \boldsymbol{j} + \alpha_2 \sin\frac{\theta}{2} \boldsymbol{k} + \cos\frac{\theta}{2} \notag \\
\mathtt{out} &= \boldsymbol{q} \space \omega \space \boldsymbol{q^{-1}} \notag
\end{align}
```

where:
```math
\begin{align}
\omega &= \mathtt{in} \notag \\
\theta &= \frac{\mathtt{amount}\cdot\pi}{180} \notag \\
\alpha  &= \mathtt{axis}  \notag \\
\end{align}
```

---

### place2d
Transform incoming 2D texture coordinates from one frame of reference to another.

@MX_TABLE_place2d@


---

### trianglewave
Generate a triangle wave from the given scalar input. The generated wave ranges from zero to one and repeats on integer boundaries.

@MX_TABLE_trianglewave@

`out` shall be computed as a triangle wave over the domain of `in`, with the range [0, 1] and period 1 according to the equation:

```math
\mathtt{out} = 2 \lvert \mathtt{in} - \lfloor \mathtt{in} + \frac{1}{2} \rfloor \rvert
```
> [!WARNING]
> define defaultinput?
                                                                                  
---

### reflect
Reflect the incoming 3D vector about a surface normal vector.

@MX_TABLE_reflect@

`out` shall be computed as the reflection of `in` about `normal`:

```math
\mathtt{out} = \omega_i -2 \lparen \omega_i \cdot N \rparen N
```

where:

```math
\begin{align}

\omega_i &= \mathtt{in} \notag \\ 
N &= \mathtt{normal} \notag

\end{align}
```

---


### refract
Refract the incoming 3D vector through a surface with the given surface normal and relative index of refraction.

@MX_TABLE_refract@

`out` shall be computed as the refraction of `in` through the interface whose surface normal is `normal` and has relative index of refraction `ior`:

```math 
\mathtt{out} = -\eta \omega_i + \lparen \eta \cos\theta_i - \cos\theta_t \rparen N
```

where:
```math
\begin{align}
\omega_i &= \mathtt{in} \notag \\ 
N &= \mathtt{normal} \notag \\ 
\eta &= \mathtt{ior} \notag \\ 
\theta_i &= \cos^{-1}(\omega_i \cdot N) \notag \\
\theta_t &= \sqrt{\eta^{2} \sin^2{\theta_i}} \notag \\
\end{align}
```



## Adjustment Nodes

### contrast
Increase or decrease the contrast of the incoming `in` values using `amount` as a linear slope multiplier.

@MX_TABLE_contrast@

The output is determined as follows:

$out =  (in - pivot) \cdot amount + pivot$

Note that the contrast increases when `amount` $>$ 1, and decreases when `amount` is between 0 and 1. Also `pivot` will not change as contrast is adjusted.

---

### remap
Linearly remap incoming values from one range of values [`inlow`, `inhigh`] to another [`outlow`, `outhigh`].

@MX_TABLE_remap@

The output is determined as follows:

$out = outlow + \left( \dfrac{in - inlow}{inhigh - inlow}  \right) \cdot (outhigh - outlow)$



### range
Remap incoming values from one range of values to another, optionally applying a gamma correction "in the middle". 

@MX_TABLE_range@

This node first remaps the input from [`inlow`, `inhigh`] to [0,1], using the remap node. It then applies gamma correction, using $1/gamma$ as the exponent. Note that gamma values greater than `1.0` make midtones brighter. This gamma corrected result is then remaped from [0,1], to [`outlow`, `outhigh`]. Then based on the value of `doclamp` the final output may be clamped to [`outlow`, `outhigh`].

---

### smoothstep
Output a smooth, hermite-interpolated remapping of input values from [`low`, `high`] to [0,1].


@MX_TABLE_smoothstep@

This remaps the input to [0,1], using a hermite-interpolated or smoothstep remapping.
Note that inputs outside the [`low`, `high`] range are clamped.

$out = \begin{cases}
    1, \ in \geq high\\
    0, \ in \leq low  \\
    remap(low, high, in), \ otherwise
\end{cases}$

---

### luminance
Output a grayscale value containing the luminance of the incoming RGB color in all color channels

@MX_TABLE_luminance@

Output a grayscale value containing the luminance of the incoming RGB color in all color channels, computed using the dot product of the incoming color `in` with the luma coefficients `lumacoeffs` of the working colorspace; the alpha channel is left unchanged if present.

---

### rgbtohsv
Convert an incoming color from RGB to HSV space (with H and S ranging from 0 to 1); the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

@MX_TABLE_rgbtohsv@

See Foley & van Dam 

---

### hsvtorgb
Convert an incoming color from HSV to RGB space; the alpha channel is left unchanged if present. This conversion is not affected by the current color space.

@MX_TABLE_hsvtorgb@

See Foley & van Dam 
---

### hsvadjust
Adjust the hue, saturation and value of an RGB color by converting the input color to HSV, adding amount.x to the hue, multiplying the saturation by amount.y, multiplying the value by amount.z, then converting back to RGB.

@MX_TABLE_hsvadjust@

Note:
- `amount.x` $>1$ rotates hue in the "red to green to blue" direction, 
- `amount.x` = 1.0 is equivalent to a 360 degree (e.g. no-op) rotation. 
- `amount.x` values that are $<$ 0 or $>$ 1 are allowed, the values are wrapped at the 0-1 boundaries. 
- The internal conversions between RGB and HSV spaces are not affected by the current color space. 
- For color4 inputs, the alpha value is unchanged.

---

### saturate
Adjust the saturation of a color, the alpha channel will be unchanged if present.

@MX_TABLE_saturate@

The `amount` value is used to perform linear interpolation between the incomihng color and the grayscale luminance of the input computed using the provided luma coefficients. 

Note that this operation is not equivalent to the saturation adjustment of the `hsvadjust` node, as that operator does not take the working or any other colorspace into account.

---

### colorcorrect
Combines various adjustment nodes into one artist-friendly color correction node. For color4 inputs, the alpha value is unchanged.

@MX_TABLE_colorcorrect@

Note 
- `hue` values that are $<$ 0 or $>$ 1 are allowed, but the values are wrapped at the 0-1 boundaries. 
- the color brightness is increased or decreased by $2^{\large exposure}$.
- the `constrastpivot` value does not change as contrast is adjusted. 


## Compositing Nodes

### mix
Mix between two inputs according to an input mix amount.

@MX_TABLE_mix@

**TODO** determine if we're including the `xxxshader` variants here? It doesn't align with the subscript behavior description below.

Each component of the `out` output should be calculated by linearly interpolating the respective components of the `fg` and `bg` inputs, using the `mix` input as the interpolant.

$$out_i = mix * fg_i + (1.0 - mix) * bg_i$$

---

### premult
Multiply the R or RGB channels of the input by the Alpha channel of the input.

@MX_TABLE_premult@

Each of the `r`, `g` and `b` components of the `out` output should be calculated by multiplying the respective components of the `in` input by the `a` component of the `in` input. The `a` component of `out` should be equal to the `a` component of the `in` input.

$$out_i =
\begin{cases}
in_i * in_a       & \text{if } i \text{ is } r,g,b\\
in_a              & \text{if } i \text{ is } a
\end{cases}$$

</details>

---

### unpremult
Divide the RGB channels of the input by the Alpha channel of the input. If the Alpha value is zero, it is passed through unchanged.

@MX_TABLE_power@

If the `a` component of the `in` input is zero, then each component of `out` should be equal 

Each of the `r`, `g` and `b` components of the `out` output should be calculated by dividing the respective components of the `in` input by the `a` component of the `in` input. The `a` component of `out` should be equal to the `a` component of the `in` input.

**TODO** - what happens if `in.a` is zero?

$$out_i =
\begin{cases}
in_i                    & \text{if } in_a = 0 \\
\frac{in_i}{in_a}       & \text{else if } i \text{ is } r,g,b\\
in_a                    & \text{else if } i \text{ is } a \\
\end{cases}$$


## Blend Nodes

Blend nodes take two 1-4 channel inputs and apply the same operator to all channels (the math for alpha is the same as for R or RGB).  In the Blend Operator table, "F" and "B" refer to any individual channel of the `fg` and `bg` inputs respectively.  Blend nodes support an additional float input `mix`, which is used to mix the original `bg` value (`mix`=0) with the result of the blend operation (`mix`=1, the default).

| Blend Operator   | Each Channel Output                          | Supported Types        |
|------------------|----------------------------------------------|------------------------|
| **`plus`**       | B+F                                          | float, color<em>N</em> |
| **`minus`**      | B-F                                          | float, color<em>N</em> |
| **`difference`** | abs(B-F)                                     | float, color<em>N</em> |
| **`burn`**       | 1-(1-B)/F                                    | float, color<em>N</em> |
| **`dodge`**      | B/(1-F)                                      | float, color<em>N</em> |
| **`screen`**     | 1-(1-F)(1-B)                                 | float, color<em>N</em> |
| **`overlay`**    | 2FB if B&lt;0.5;<br> 1-2(1-F)(1-B) if B>=0.5 | float, color<em>N</em> |


### plus
Add two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_plus@

Each component of the `fg` input must be added to the corresponding component of the `bg` input to create an intermediate result.  

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i=mix*(bg_i + fg_i) + (1.0-mix)*bg_i$$

</details>

---

### minus
Subtract two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_minus@

Each component of the `fg` input must be subtracted from the corresponding component of the `bg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i=mix*(bg_i - fg_i) + (1.0-mix)*bg_i$$

---

### difference
Absolute-value difference of two 1-4 channel inputs, with optional mixing between the bg input and the result.

@MX_TABLE_difference@

Each component of the `fg` input must be subtracted from the corresponding component of the `bg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix* | bg_i - fg_i | + (1.0-mix)*bg_i$$

---

### burn
Take two 1-4 channel inputs and apply the same operator to all channels: 
1-(1-B)/F

@MX_TABLE_burn@

Each component of the `out` output should be set to zero if the respective component of the `fg` input is zero or less. 

Otherwise, each component of the `bg` input is inverted and then divided by the respective component of the `fg` input, and finally inverted again to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i =
\begin{cases}
mix*(1.0 - (\frac{1.0 - bg_i}{fg_i}) + (1.0-mix)*bg_i       & \text{if } fg_i > 0\\
0                                                           & \text{if } fg_i <= 0
\end{cases}$$

---

### dodge
Take two 1-4 channel inputs and apply the same operator to all channels: 
B/(1-F)

@MX_TABLE_dodge@

Each component of the `out` output should be set to zero if the respective component of the `fg` input when inverted is zero or less.

Otherwise, each component of the `bg` input is divided by the respective component of `fg` inverted to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i =
\begin{cases}
mix*\frac{bg_i}{1.0 - fg_i} + (1.0-mix)*bg_i       & \text{if } (1.0 - fg_i) > 0\\
0                                                           & \text{if } (1.0 - fg_i) <= 0
\end{cases}$$

---

### screen
Take two 1-4 channel inputs and apply the same operator to all channels: 
1-(1-F)*(1-B)

@MX_TABLE_screen@

Each respective component of the `fg` input and the `bg` must be inverted and then multiplied together, this product should then inverted again to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix*(1.0 - (1.0 - fg_i) * (1.0 - bg_)) + (1.0-mix)*bg_i$$

---

### overlay
Take two 1-4 channel inputs and apply the same operator to all channels:
2FB if B<0.5;
1-2(1-F)(1-B) if B>=0.5

@MX_TABLE_overlay@

For each component of the `bg` input, if the value is less than 0.5 then both the respective `fg` and `bg` components are multiplied together and then multiplied by two to create an intermediate result. Otherwise, if the component of `bg` is greater than or equal to 0.5, then both the `fg` and `bg` components are inverted, multiplied together and then multiplied by 2. Finally this value is inverted to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i =
\begin{cases}
mix*(2 * fg_i * bg_i) + (1.0-mix)*bg_i                      & \text{ if } bg_i < 0.5 \\
mix*(1 - 2(1 - fg_i)(1 - bg_i)) + (1.0-mix)*bg_i            & \text{ if } bg_i >= 0.5
\end{cases}$$


## Merge Nodes

Merge nodes take two 4-channel (color4) inputs and use the built-in alpha channel(s) to control the 
compositing of the `fg` and `bg` inputs.  In the node descriptions below, `F` and `B` refer to the 
non-alpha channels of the `fg` and `bg` inputs respectively, and `f` and `b` refer to the alpha 
channels of the `fg` and `bg` inputs.  Merge nodes are not defined for 1-channel or 3-channel 
inputs, and cannot be used on vector<em>N</em> streams.  Merge nodes support an optional float 
input `mix`, which can be used to mix the original `bg` value (`mix=0`) with the result of the 
blend operation (`mix=1`, the default).

### disjointover
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
F+B         if f+b<=1
F+B(1-f)/b  if f+b>1
alpha: min(f+b,1)

| **`disjointover`** | F+B if f+b&lt;=1;<br> F+B(1-f)/b if f+b>1 | min(f+b,1) |


@MX_TABLE_disjointover@

If the sum of the `a` components of both the `fg` and `bg` inputs is less than or equal to 1, then each respective component of the `fg` and `bg` inputs is summed to create an intermediate result. Otherwise, if the sum of the `a` components of both the `fg` and `bg` inputs is greater than 1, then each component of the `bg` input is multiplied by the ratio of inverse of the `a` component of the `fg` input to the `a` component of the `bg` input, finally the corresponding component of the `fg` input is added  to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i =
\begin{cases}
mix*(fg_i + bg_i) + (1.0-mix)*bg_i                      & \text{ if } fg_a + bg_a <= 1 \\
mix*(fg_i+bg_i(1-fg_a)/bg_a) + (1.0-mix)*bg_i            & \text{ if } fg_a + bg_a > 1
\end{cases}$$

---

### in
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs:
* RGB = Fb
* Alpha = fb

@MX_TABLE_in@

Each component of the `fg` input must be multiplied by the `a` component of the `bg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix*(fg_i*bg_a) + (1.0-mix) * bg_i$$

---

### mask
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs: 
Bf  (alpha: bf)

@MX_TABLE_mask@

Each component of the `bg` input must be multiplied by the `a` component of the `fg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix*(bg_i*fg_a) + (1.0-mix)*bg_i$$

---

### matte
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs: 
Ff+B(1-f)  (alpha: f+b(1-f))

@MX_TABLE_matte@

Each of the `r`,`g` and `b` components of `fg` and `bg` inputs are linearly interpolated, using the `a` component of the `fg` input as the interpolant to create an intermediate result.  The `a` component of the `fg` input is used as the `a` component of this intermediate result. 

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i =
\begin{cases}
mix*(fg_i*fg_a + bg_i*(1.0-fg_a)) + (1.0-mix)*bg_i       & \text{ if } i \text{ in } r,g,b \\
mix*(fg_i + bg_i*(1.0-fg_a)) + (1.0-mix)*bg_i            & \text{ if } i = a
\end{cases}$$

---

### out
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs: 
F(1-b)  (alpha: f(1-b))

@MX_TABLE_out@

Each component of the `fg` input must be multiplied by the inverted `a` component of the `bg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix * (fg_i*(1.0-bg_a)) + (1.0-mix) * bg_i$$

---

### over
Take two color4 inputs and use the built-in alpha channel(s) to control the compositing of the fg and bg inputs: 
F+B(1-f)  (alpha: f+b(1-f))

@MX_TABLE_over@

Each component of the `bg` input must be multiplied by the inverted `a` component of the `fg` input, and then added to the corresponding component of the `fg` input to create an intermediate result.

Each component of the `out` output should be calculated by linearly interpolating the respective component of the the absolute value of this intermediate result and the respective component of `bg` using the `mix` input as the interpolant.

$$out_i = mix * (fg_i + (bg_i*(1.0-fg_a))) + (1.0-mix) * bg_i$$


## Masking Nodes

Masking nodes take one 1-4 channel input `in` plus a separate float `mask` input and apply the same operator to all channels (if present, the math for alpha is the same as for R or RGB).

### inside
Take one 1-4 channel input "in" and multiply each channel by a separate float "mask" input.

@MX_TABLE_inside@

The components of `out` must be calculated by multiplying the components `in` by the float input `mask`.

$$out_i = in_i * mask$$

---

### outside
Take one 1-4 channel input `in` and multiply each channel by the inverse of a separate float `mask` input.

@MX_TABLE_outside@

The components of `out` must be calculated by multiplying the components `in` by the inverse of the float input `mask`.

$$out_i = in_i * (1.0 - mask)$$


## Conditional Nodes

Conditional nodes are used to compare values of two streams, or to select a value from one of several streams.

### ifgreater

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater than the `value2` input.

@MX_TABLE_ifgreater@

---

### ifgreatereq

Output the value of the `in1` or `in2` stream depending on whether the `value1` input is greater or equal to the `value2` input.

@MX_TABLE_ifgreatereq@

---

### ifequal

@MX_TABLE_ifequal@

---

### switch

Output the value of one of up to ten input streams, according to the value of a selector input `which`. Note that not all inputs need to be connected. The output has the same type as `in1`, with a default value of __zero__.

@MX_TABLE_switch@

The value of `which` determines the output as follows:

$out = \begin{cases}
    in1, \  &which < 1\\
    in2, \ 1 \leq &which < 2 \\
    in3, \ 2 \leq &which < 3 \\
    ... \\
    in10, \ 9 \leq &which < 10 \\
\end{cases}$

For values `which` $\geq 10$ we return __zero__

## Channel Nodes

Channel nodes are used to perform channel manipulations and data type conversions on streams.

### extract

Isolate a single float channel from a __vectorN__ or __colorN__ stream. The output value is of type `float` with a default value of __zero__.

@MX_TABLE_extract@

The valid range for `index` should be clamped to $[0,N)$ in the user interface, where __N__ is the size of the input vector stream. `index` is a uniform, non-varying value. Any `index` values outside of the valid range should result in an error.

---

### separate2

Split the channels of a 2-channel stream into separate float outputs.

@MX_TABLE_separate2@

For the vector2-input `in`, `outx` and `outy` correspond to the x- and y-components of `in`..

---

### separate3

Split the channels of a 3-channel stream into separate float outputs.

@MX_TABLE_separate3@

When the input `in` is a color3, `outr`, `outg`, and `outb` correspond to the r-, g-, and b-components of `in`, respectively.

When the input `in` is a vector3, `outx`, `outy`, and `outz` correspond to the x-, y-, and z-components of `in`, respectively.

---

### separate4

Split the channels of a 4-channel stream into separate float outputs.

@MX_TABLE_separate4@

When the input `in` is a color4, `outr`, `outg`, `outb`, and `outa` correspond to the r-, g-, b-, and alpha components of `in`, respectively.

When the input `in` is a vector4, `outx`, `outy`, `outz`, and `outw` correspond to the x-, y-, z-, and w-components of `in`, respectively.

---

### combine2

Combine the channels from two streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine2@

---

### combine3

Combine the channels from three streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine3@

---

### combine4

Combine the channels from four streams into the same number of channels of a single output stream of a compatible type.

@MX_TABLE_combine4@


## Convolution Nodes

Convolution nodes have one input named `in`, and apply a defined convolution function to the input stream.

### blur

Applies a convolution blur to the input stream.

@MX_TABLE_blur@

#### Filter Types

|Name          |Description           |
|--------------|----------------------|
|"box"         |Linear box filter     |
|"gaussian"    |Gaussian smoothing    |

#### Box Filter

Blurs the input, based on the average of neighboring values:

$$B = sum_{i=1}^sc a_i = a_1 + a_2 + ... + a_sc$$

#### Gaussian Filter

Applies a gaussian filter to the input:

$$G(u,v) = \frac{1}{2 \pi \sigma ^2} e ^{- \frac{u^2 + v^2}{2 \sigma ^2}}$$

---

### heighttonormal

Convert a scalar height map to a tangent-space normal map of type `vector3`. The output normal map is encoded with all channels in the [0-1] range, enabling its storage in unsigned image formats.

@MX_TABLE_heighttonormal@

Let the scalar values be represented as a function $h(u, v)$, where $(u, v)$ are coordinates of the image.

The partial derivatives of the height field are:
$$\frac{\partial h}{\partial u} \text{ and } \frac{\partial h}{\partial v}$$

The normal vector $\vec{n}$ at each point $(x, y)$ can be calculated as:
$$N = \left(-\frac{\partial h}{\partial u}, -\frac{\partial h}{\partial v}, 1\right)$$

To normalize the normal vector, we divide it by its magnitude:
$$N = \frac{1}{\sqrt{1 + \left(\frac{\partial h}{\partial u}\right)^2 + \left(\frac{\partial h}{\partial v}\right)^2}} \left(-\frac{\partial h}{\partial u}, -\frac{\partial h}{\partial v}, 1\right)$$

# Logical Operator Nodes 

## and
logically AND the two input boolean values

@MX_TABLE_and@

Output the boolean value resulting from the logical AND of the input values. 

---

## or
logically Inclusive OR the two input boolean values

@MX_TABLE_or@

Output the boolean value resulting from the logical Inclusive OR of the input values. 

---

## xor
logically Exclusive OR the two input boolean values

@MX_TABLE_xor@

Output the boolean value resulting from the logical Exclusive OR of the input values. 

---

## not
logically NOT the input boolean value

@MX_TABLE_not@

Output the boolean value resulting from the logical NOT of the input value. 

## Organization Nodes

### dot


@MX_TABLE_dot@

A no-op, passes its input through to its output unchanged. Users can use dot nodes to shape edge connection paths or provide documentation checkpoints in node graph layout UI's. Dot nodes may also pass uniform values from <constant> or other nodes with uniform="true" outputs to uniform <input>s and <token>s.