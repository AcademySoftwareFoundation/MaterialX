## add
Add a value to the incoming float/color/vector/matrix

|Input     |Description                |Type     |Default  |Accepted Values|
|----------|---------------------------|---------|---------|---------------|
|`in1`     |The primary input stream   |__multi__|__zero__ |               |
|`in2`     |The stream to add to `in1` |__multi__|__zero__ |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|int              |int              |int          |
|int              |float            |float        |
|float            |float            |float        |
|float            |int              |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector2          |int              |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector3          |int              |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|vector4          |int              |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color3           |int              |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |
|color4           |int              |color4       |
|matrix33         |matrix33         |matrix33     |
|matrix33         |float            |matrix33     |
|matrix33         |int              |matrix33     |
|matrix44         |matrix44         |matrix44     |
|matrix44         |float            |matrix44     |
|matrix44         |int              |matrix44     |

#### Behavior
`out` shall be computed as the component-wise addition of `in1` and `in2`

```math
\mathtt{out}_{i,j} = \mathtt{in1}_{i,j} + \mathtt{in2}_{i,j} 
```

</details>

---

## subtract
Subtract a value from the incoming float/color/vector/matrix

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The primary input stream          |__multi__|__zero__ |               |
|`in2`     |The stream to subtract from `in1` |__multi__|__zero__ |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|int              |int              |int          |
|int              |float            |float        |
|float            |float            |float        |
|float            |int              |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector2          |int              |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector3          |int              |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|vector4          |int              |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color3           |int              |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |
|color4           |int              |color4       |
|matrix33         |matrix33         |matrix33     |
|matrix33         |float            |matrix33     |
|matrix33         |int              |matrix33     |
|matrix44         |matrix44         |matrix44     |
|matrix44         |float            |matrix44     |
|matrix44         |int              |matrix44     |

#### Behavior
`out` shall be computed as the component-wise subtraction of `in2` from `in1`

```math
\mathtt{out}_{i,j} = \mathtt{in2}_{i,j} - \mathtt{in1}_{i,j} 
```

</details>

---

## multiply
Multiply two values together. Scalar and vector types multiply component-wise, while matrices multiply with the standard matrix product.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The primary input stream          |__multi__|__one__  |               |
|`in2`     |The stream to multiply with `in1` |__multi__|__one__  |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|int              |int              |int          |
|int              |float            |float        |
|float            |float            |float        |
|float            |int              |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector2          |int              |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector3          |int              |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|vector4          |int              |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color3           |int              |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |
|color4           |int              |color4       |
|matrix33         |matrix33         |matrix33     |
|matrix33         |float            |matrix33     |
|matrix33         |int              |matrix33     |
|matrix44         |matrix44         |matrix44     |
|matrix44         |float            |matrix44     |
|matrix44         |int              |matrix44     |

#### Behavior
For scalar and vector types, `out` shall be computed as the component-wise multiplication of `in1` with `in2`

```math
\mathtt{out}_{i} = \mathtt{in1}_{i} * \mathtt{in2}_{i} 
```

For matrix types, `out` shall be computed as the matrix inner product.

</details>

---

## divide
Divide one value by another. Scalar and vector types divide component-wise, while for matrices `in1` is multiplied with the inverse of `in2`.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The primary input stream          |__multi__|__one__  |               |
|`in2`     |The stream to divide `in1` by     |__multi__|__one__  |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|int              |int              |int          |
|int              |float            |float        |
|float            |float            |float        |
|float            |int              |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector2          |int              |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector3          |int              |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|vector4          |int              |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color3           |int              |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |
|color4           |int              |color4       |
|matrix33         |matrix33         |matrix33     |
|matrix33         |float            |matrix33     |
|matrix33         |int              |matrix33     |
|matrix44         |matrix44         |matrix44     |
|matrix44         |float            |matrix44     |
|matrix44         |int              |matrix44     |

#### Behavior
For scalar and vector types, `out` shall be computed as the component-wise division of `in2` by `in1`

```math
\mathtt{out}_{i} = \mathtt{in1}_{i} / \mathtt{in2}_{i} 
```

For matrix types, `out` shall be computed as the matrix inner product of `in1` and the inverse of `in2`.

```math
\mathtt{out} = \mathtt{in}_1 \mathtt{in}_2^{-1}
```

</details>

---

## modulo
The remaining fraction after dividing an incoming float/color/vector by a value and subtracting the integer portion. Modulo always returns a non-negative result.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The primary input stream          |__multi__|__one__  |               |
|`in2`     |The stream to divide `in1` by.    |__multi__|__one__  | `in2` != 0    |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|float            |float            |float        |
|float            |int              |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector2          |int              |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector3          |int              |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|vector4          |int              |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color3           |int              |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |
|color4           |int              |color4       |

#### Behavior
`out` shall be computed as the absolute value of the component-wise modulo of `in1` and `in2`
```math
\mathtt{out}_{i} = \lvert \mathtt{in1}_{i} \space mod \space \mathtt{in2}_{i} \rvert
```

</details>

---

## invert
subtract the incoming float, color, or vector from `amount` in all channels, outputting: `amount - in`.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in`      |The primary input stream          |__multi__|__zero__ |               |
|`amount`  |The value to subtract `in` from   |__multi__|__one__  |               |

<details>

#### Signatures
|`in`             |`amount`         |`out`        |
|-----------------|-----------------|-------------|
|float            |float            |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |

#### Behavior
`out` shall be computed as `in` subtracted from `amount`:
```math
\mathtt{out}_{i} = \mathtt{amount}_{i} - \mathtt{in}_{i}
```

</details>

---

## absval
The per-channel absolute value of the incoming float/color/vector.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in`      |The primary input stream          |__multi__|__zero__ |               |
                                                                  
<details>

#### Signatures
|`in`             |`out`        |
|-----------------|-------------|
|float            |float        |
|vector2          |vector2      |
|vector3          |vector3      |
|vector4          |vector4      |
|color3           |color3       |
|color4           |color4       |

#### Behavior
`out` shall be computed as the channel-wise absolute value of `in`:
```math
\mathtt{out}_{i} = \lvert \mathtt{in}_{i} \rvert
```

</details>

---

## sign
The per-channel sign of the incoming float/color/vector value: -1 for negative, +1 for positive, or 0 for zero.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in`      |The primary input stream          |__multi__|__zero__ |               |
                                                                  
<details>

#### Signatures
|`in`             |`out`        |
|-----------------|-------------|
|float            |float        |
|vector2          |vector2      |
|vector3          |vector3      |
|vector4          |vector4      |
|color3           |color3       |
|color4           |color4       |

#### Behavior
`out` shall be computed as the channel-wise sign of `in`:
```math
\mathtt{out}_{i} = sgn \space \mathtt{in}_{i}
```

</details>

---

## floor
The per-channel nearest integer value less than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the floor(float) also has a variant outputting an integer type.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in`      |The primary input stream          |__multi__|__zero__ |               |
                                                                 
<details>

#### Signatures
|`in`             |`out`        |
|-----------------|-------------|
|float            |float        |
|float            |int          |
|vector2          |vector2      |
|vector3          |vector3      |
|vector4          |vector4      |
|color3           |color3       |
|color4           |color4       |

#### Behavior
`out` shall be computed as the channel-wise floor of `in`:
```math
\mathtt{out}_{i} = \lfloor \mathtt{in}_{i} \rfloor
```

</details>

---

## ceil
The per-channel nearest integer value greater than or equal to the incoming float/color/vector. The output remains in floating point per-channel, i.e. the same type as the input, except that the ceil(float) also has a variant outputting an integer type.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in`      |The primary input stream          |__multi__|__zero__ |               |
                                                                  
<details>

#### Signatures
|`in`             |`out`        |
|-----------------|-------------|
|float            |float        |
|float            |int          |
|vector2          |vector2      |
|vector3          |vector3      |
|vector4          |vector4      |
|color3           |color3       |
|color4           |color4       |

#### Behavior
`out` shall be computed as the channel-wise ceil of `in`:

```math
\mathtt{out}_{i} = \lceil \mathtt{in}_{i} \rceil
```

</details>

---

## round
Round each channel of the incoming float/color/vector values to the nearest integer value.

|Input     |Description                   |Type     |Default |Accepted Values|
|----------|------------------------------|---------|--------|---------------|
|`in`      |The input stream to be rounded|__multi__|__zero__|               |
                                                             
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|float            |int              |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |
|color3           |color4           |
|color4           |color4           |

#### Behavior
Round each channel of `in` to the nearest whole number, storing the result in the corresponding channel of `out`. Rounding shall be calculated using IEEE 754 round-to-nearest-ties-away-from-zero mode.

```math
\mathtt{out}_{i} = \lfloor \mathtt{in}_{i} \rceil
```
</details>

---

## power
Raise incoming float/color values to the specified exponent, commonly used for "gamma" adjustment.

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The primary input stream          |__multi__|__one__  |               |
|`in2`     |The exponent to raise `in1` to    |__multi__|__one__  |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|float            |float            |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |

#### Behavior
`out` shall be computed as the component-wise raising of `in1` to the `in2`th power:

```math
\mathtt{out}_{i} = \mathtt{in1}_{i}^{\mathtt{in2}_{i}}
```

</details>

---

## sin
The sine of the incoming value, which is expected to be expressed in radians.

|Input     |Description                             |Type     |Default  |Accepted Values|
|----------|----------------------------------------|---------|---------|---------------|
|`in`      |The input stream to have its sine taken |__multi__|__zero__ |               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise sin of `in`. `in` shall be in radians.

```math
\mathtt{out}_{i} = \sin{\mathtt{in}_{i}}
```
</details>

---

## cos
The cosine of the incoming value, which is expected to be expressed in radians.

|Input|Description                               |Type     |Default  |Accepted Values|
|-----|------------------------------------------|---------|---------|---------------|
|`in` |The input stream to have its cosine taken |__multi__|__zero__ |               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise cosine of `in`. `in` shall be in radians:

```math
\mathtt{out}_{i} = \cos{\mathtt{in}_{i}}
```
</details>

---

## tan
The tangent of the incoming value, which is expected to be expressed in radians.

|Input|Description                                |Type     |Default  |Accepted Values|
|-----|-------------------------------------------|---------|---------|---------------|
|`in` |The input stream to have its tangent taken |__multi__|__zero__ |               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise tangent of `in`. `in` shall be in radians:

```math
\mathtt{out}_{i} = \tan{\mathtt{in}_{i}}
```
</details>

---

## asin
The arcsine of the incoming value. The output will be expressed in radians.

|Input|Description                                |Type     |Default |Accepted Values    |
|-----|-------------------------------------------|---------|--------|-------------------|
|`in` |The input stream to have its arcsine taken |__multi__|__zero__|[-__one__, __one__]|

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise arcsine of `in`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \sin^{-1}{\mathtt{in}_{i}}
```
</details>

---

## acos
The arccosine of the incoming value. The output will be expressed in radians.

|Input     |Description                                  |Type     |Default |Accepted Values    |
|----------|---------------------------------------------|---------|--------|-------------------|
|`in`      |The input stream to have its arccosine taken |__multi__|__zero__|[-__one__, __one__]|
                                                                            
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise arccosine of `in`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \cos^{-1}{\mathtt{in}_{i}}
```
</details>

---

## atan
The arctangent of the incoming value. The output will be expressed in radians.

|Input     |Description                                   |Type     |Default |Accepted Values    |
|----------|----------------------------------------------|---------|--------|-------------------|
|`in`      |The input stream to have its arctangent taken |__multi__|__zero__|                   |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise arctangent of `in`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \tan^{-1}{\mathtt{in}_{i}}
```
</details>

---

## atan2
the arctangent of the expression (`iny`/`inx`). The output will be expressed in radians.

|Input     |Description                                                |Type     |Default |Accepted Values|
|----------|-----------------------------------------------------------|---------|--------|---------------|
|`iny`      |The input stream numerator the arctangent expression      |__multi__|__zero__|               |
|`inx`      |The input stream denominator to the arctangent expression |__multi__|__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise arctangent of `iny` / `inx`. `out` shall be in radians.

```math
\mathtt{out}_{i} = \tan^{-1}{\frac{\mathtt{iny}_{i}}{\mathtt{inx}_{i}}}
```
</details>

---

## sqrt
The square root of the incoming value. 

|Input     |Description                                    |Type     |Default |Accepted Values       |
|----------|-----------------------------------------------|---------|--------|----------------------|
|`in`      |The input stream to have its square root taken |__multi__|__zero__| [__zero__, __+inf__) |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise square root of `in`.

```math
\mathtt{out}_{i} = \sqrt{\max(\mathtt{in}_{i}, 0)}
```
</details>

---

## ln
The natural logarithm of the incoming value. 

|Input     |Description                                          |Type     |Default |Accepted Values       |
|----------|-----------------------------------------------------|---------|--------|----------------------|
|`in`      |The input stream to have its natural logarithm taken |__multi__|__one__| (__zero__, __+inf__) |
                                                                                    
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise natural logarithm of `in`.

```math
\mathtt{out}_{i} = \ln{\mathtt{in}_{i}}
```
</details>

---

## exp
*e* to the power of the incoming value.

|Input     |Description                                          |Type     |Default |Accepted Values|
|----------|-----------------------------------------------------|---------|--------|---------------|
|`in`      |The input stream to have its natural logarithm taken |__multi__|__zero__|               |
                                                                                    
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |
|vector2          |vector2          |
|vector3          |vector3          |
|vector4          |vector4          |

#### Behavior
`out` shall be computed as the component-wise raising of the base of natural logarithms to the power `in`.

```math
\mathtt{out}_{i} = e^{\mathtt{in}_{i}}
```
</details>

---

## clamp
Clamp incoming values per-channel to a specified range of float/color/vector values.

|Input     |Description                                                           |Type     |Default |Accepted Values|
|----------|----------------------------------------------------------------------|---------|--------|---------------|
|`in`      |The input stream to be clamped                                        |__multi__|__zero__|               |
|`low`     |Any value of `in` lower than this value will be set to this value     |__multi__|__zero__|               |
|`high`    |Any value of `in` higher than this value will be set to this value    |__multi__|__zero__|               |

<details>

#### Signatures
|`in`             |`low`            |`high`           |`out`            |
|-----------------|-----------------|-----------------|-----------------|
|float            |float            |float            |float            |
|vector2          |vector2          |vector2          |vector2          |
|vector2          |float            |float            |vector2          |
|vector3          |vector3          |vector3          |vector3          |
|vector3          |float            |float            |vector3          |
|vector4          |vector4          |vector4          |vector4          |
|vector4          |float            |float            |vector4          |

#### Behavior
`out` shall be computed as the component-wise clamping of `in` between `low` and `high`. That is:

```math
\mathtt{out}_{i} = \max(\mathtt{low}_{i}, \min(\mathtt{high}_{i}, {\mathtt{in}_{i}}))
```
</details>

---

## trianglewave
Generate a triangle wave from the given scalar input. The generated wave ranges from zero to one and repeats on integer boundaries.

|Input     |Description                                        |Type     |Default |Accepted Values|
|----------|---------------------------------------------------|---------|--------|---------------|
|`in`      |The input stream to generate the triangle wave from|__multi__|__zero__|               |
                                                                                  
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|float            |float            |

#### Behavior
`out` shall be computed as a triangle wave over the domain of `in`, with the range [0, 1] and period 1 according to the equation:

```math
\mathtt{out} = 2 \lvert \mathtt{in} - \lfloor \mathtt{in} + \frac{1}{2} \rfloor \rvert
```
</details>

---

## min
Select the minimum of the two incoming values

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The first input stream            |__multi__|__zero__ |               |
|`in2`     |The second input stream           |__multi__|__zero__ |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|float            |float            |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |

#### Behavior
`out` shall be computed as the component-wise minimum of `in1` and `in2`:
```math
\mathtt{out}_{i} = \min(\mathtt{in1}_{i}, \mathtt{in2}_{i})
```

</details>

---

## max
Select the maximum of the two incoming values

|Input     |Description                       |Type     |Default  |Accepted Values|
|----------|----------------------------------|---------|---------|---------------|
|`in1`     |The first input stream            |__multi__|__zero__ |               |
|`in2`     |The second input stream           |__multi__|__zero__ |               |

<details>

#### Signatures
|`in1`            |`in2`            |`out`        |
|-----------------|-----------------|-------------|
|float            |float            |float        |
|vector2          |vector2          |vector2      |
|vector2          |float            |vector2      |
|vector3          |vector3          |vector3      |
|vector3          |float            |vector3      |
|vector4          |vector4          |vector4      |
|vector4          |float            |vector4      |
|color3           |color3           |color3       |
|color3           |float            |color3       |
|color4           |color4           |color4       |
|color4           |float            |color4       |

#### Behavior
`out` shall be computed as the component-wise maximum of `in1` and `in2`:
```math
\mathtt{out}_{i} = \max(\mathtt{in1}_{i}, \mathtt{in2}_{i})
```

</details>

---

## magnitude
Output the float magnitude (vector length) of the incoming vectorN stream; cannot be used on float or colorN streams. Note: the fourth channel in vector4 streams is not treated any differently, e.g. not as a homogeneous "w" value.

|Input     |Description                                          |Type     |Default |Accepted Values|
|----------|-----------------------------------------------------|---------|--------|---------------|
|`in`      |The input stream for which to calculate the magnitude|__multi__|__zero__|               |
                                                                                    
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector2          |float            |
|vector3          |float            |
|vector4          |float            |

#### Behavior
`out` shall be computed as the euclidean norm of `in`:

```math
\mathtt{out} = \lVert \mathtt{in} \rVert = \sqrt{\sum_{i=0}^{N-1} {\mathtt{in}_i}^2}
```

</details>

---

## distance
Measures the distance between two points in 2D, 3D, or 4D.

|Input     |Description                    |Type     |Default |Accepted Values|
|----------|-------------------------------|---------|--------|---------------|
|`in1`     |The first input vector stream  |__multi__|__zero__|               |
|`in2`     |The second input vector stream |__multi__|__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector2          |float            |
|vector3          |float            |
|vector4          |float            |

#### Behavior
`out` shall be computed as the magnitude of the vector pointing from `in1` to `in2`, that is the euclidean norm of `in2 - in1`:

```math
\mathtt{out} = \lVert \mathtt{in2} - \mathtt{in1} \rVert
```

</details>

---

## dotproduct
Output the (float) dot product of two incoming vectorN streams; cannot be used on float or colorN streams.

|Input     |Description                    |Type     |Default |Accepted Values|
|----------|-------------------------------|---------|--------|---------------|
|`in1`     |The first input vector stream  |__multi__|__zero__|               |
|`in2`     |The second input vector stream |__multi__|__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector2          |float            |
|vector3          |float            |
|vector4          |float            |

#### Behavior
`out` shall be computed as the dot (inner) product of `in1` and `in2`

```math
\mathtt{out} = \mathtt{in1} \cdot \mathtt{in2}
```

</details>

---

## crossproduct
Output the (vector3) cross product of two incoming vector3 streams; cannot be used on any other stream type. A disabled crossproduct node passes through the value of in1 unchanged.

|Input     |Description                     |Type   |Default |Accepted Values|
|----------|--------------------------------|-------|--------|---------------|
|`in1`     |The first input vector3 stream  |vector3|__zero__|               |
|`in2`     |The second input vector3 stream |vector3|__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector3          |vector3          |

#### Behavior
`out` shall be computed as the cross product of `in1` and `in2`:

```math
\mathtt{out} = \mathtt{in1} \times \mathtt{in2}
```
If the node is disabled, `out` shall be `in1`.

</details>

---

## transformpoint
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

|Input      |Description                                                                                                                                                    |Type   |Default |Accepted Values|
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|---------------|
|`in`       |The input vector3 stream                                                                                                                                       |vector3|__zero__|               |
|`fromspace`|The name of a vector space understood by the rendering target to transform the in point from; may be empty to specify the renderer's working or "common" space.|string |__zero__|               |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` point to.                                                    |string |__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector3          |vector3          |

#### Behavior
`in` shall be reinterpreted as a 4-dimensional row vector, $p$, by the addition of a homogeneous coordinate with a value of 1. $p$ shall be multipled with the matrix $M$.

The matrix, $M$, shall be computed by the renderer as the matrix necessary to transform from `fromspace` to `tospace`. If either of the named spaces are unknown to the renderer, or no suitable matrix can be computed, $M$ shall be the identity matrix.

$p'$ shall be computed as multiplication of $p$ with $M$.

`out` shall be computed from $p'$ by dividing the first three coordinates by the homogeneous coordinate:

```math
\begin{align}
p &= \lbrack \mathtt{in}_{0}, \mathtt{in}_{1}, \mathtt{in}_{2}, 1 \rbrack \notag \\
p' &= p \cdot M \notag \\
\mathtt{out} &= \frac{\lbrack p'_{0}, p'_{1}, p'_{2} \rbrack }{p'_{3}} \notag
\end{align}
```

</details>

---

## transformvector
Transform the incoming vector3 coordinate from one specified space to another; cannot be used on any other stream type.

|Input      |Description                                                                                                                                                    |Type   |Default |Accepted Values|
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|---------------|
|`in`       |The input vector3 stream                                                                                                                                       |vector3|__zero__|               |
|`fromspace`|The name of a vector space understood by the rendering target to transform the in point from; may be empty to specify the renderer's working or "common" space.|string |__zero__|               |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` point to.                                                    |string |__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector3          |vector3          |

#### Behavior
`in` shall be reinterpreted as a 4-dimensional row vector, $v$, by the addition of a homogeneous coordinate with a value of 0. $v$ shall be multipled with the matrix $M$.

The matrix, $M$, shall be computed by the renderer as the matrix necessary to transform from `fromspace` to `tospace`. If either of the named spaces are unknown to the renderer, or no suitable matrix can be computed, $M$ shall be the identity matrix.

$v'$ shall be computed as multiplication of $v$ with $M$.

`out` shall be computed from $v'$ by removing the homogeneous coordinate.

```math
\begin{align}

\vec{v} &= [\mathtt{in}_{0}, \mathtt{in}_{1}, \mathtt{in}_{2}, 1] \notag \\
\vec{v}' &= \vec{v} \cdot M \notag \\
\mathtt{out} &= [\vec{v}'_{0}, \vec{v}'_{1}, \vec{v}'_{2}] \notag

\end{align}
```
</details>

---

## transformnormal
Transform the incoming vector3 normal from one specified space to another; cannot be used on any other stream type.

|Input      |Description                                                                                                                                                    |Type   |Default |Accepted Values|
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|-------|--------|---------------|
|`in`       |The input vector3 stream                                                                                                                                       |vector3|__zero__|               |
|`fromspace`|The name of a vector space understood by the rendering target to transform the in point from; may be empty to specify the renderer's working or "common" space.|string |__zero__|               |
|`tospace`  |The name of a vector space understood by the rendering target for the space to transform the `in` point to.                                                    |string |__zero__|               |

<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|vector3          |vector3          |

#### Behavior
`in` shall be reinterpreted as a 4-dimensional row vector, $n$, by the addition of a homogeneous coordinate with a value of 0. $n$ shall be multipled by the inverse transpose of the matrix $M$.

The matrix, $M$, shall be computed by the renderer as the matrix necessary to transform from `fromspace` to `tospace`. If either of the named spaces are unknown to the renderer, or no suitable matrix can be computed, $M$ shall be the identity matrix.

$n'$ shall be computed as multiplication of $n$ with the inverse transpose of $M$.

`out` shall be computed from $n'$ by removing the homogeneous coordinate.

```math
\begin{align}

\vec{n} &= [\mathtt{in}_{0}, \mathtt{in}_{1}, \mathtt{in}_{2}, 0] \notag \\
\vec{n}' &= \vec{n} \cdot (M^T)^{-1} \notag \\
\mathtt{out} &= [\vec{n}'_{0}, \vec{n}'_{1}, \vec{n}'_{2}] \notag

\end{align}
```
</details>

---

## transformmatrix
Transform the incoming vectorN by the specified matrix.

|Input     |Description                                                                                                                                                                                                     |Type     |Default |Accepted Values|
|----------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|--------|---------------|
|`in`      |The input vector stream. If needed, an additional 1.0 component will be temporarily appended to the in vector to make it match the dimension of the transforming mat matrix, then removed after transformation.|vector3  |__zero__|               |
|`mat`     |the matrix used to transform the vector; a vector2 in can be transformed by a matrix33, a vector3 by a matrix33 or a matrix44, and a vector4 by a matrix44                                                      |__multi__|__one__ |               |

<details>

#### Signatures
|`in`             |`mat`            |`out`            |
|-----------------|-----------------|-----------------|
|vector2          |matrix33         |vector2          |
|vector3          |matrix33         |vector3          |
|vector3          |matrix44         |vector3          |
|vector4          |matrix44         |vector4          |

#### Behavior

</details>

---

## normalmap
Transform a normal vector from the encoded tangent space to world space. The input normal vector is assumed to be encoded with all channels in the [0-1] range, as would commonly be output from a normal map.

|Input      |Description                                                                       |Type     |Default        |Accepted Values|
|-----------|----------------------------------------------------------------------------------|---------|---------------|---------------|
|`in`       |The input vector3 stream.                                                         |vector3  |(0.5, 0.5, 1.0)|               |
|`scale`    |A scaling factor to apply to the incoming vector                                  |__multi__|(0.5, 0.5, 1.0)|               |
|`normal`   |surface normal; defaults to the current world-space normal.                       |vector3  |__one__        |               |
|`tangent`  |surface tangent vector; defaults to the current world-space tangent vector.       |vector3  |__one__        |               |
|`bitangent`|surface bitangent vector; defaults to the current world-space bitangent vector.   |vector3  |__one__        |               |

<details>

#### Signatures
|`in`             |`scale`          |`normal`         |`tangent`        |`bitangent`      |`out`            |
|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|
|vector3          |float            |vector3          |vector3          |vector3          |vector3          |
|vector3          |vector3          |vector3          |vector3          |vector3          |vector3          |

#### Behavior
</details>

---

## creatematrix
Build a 3x3 or 4x4 matrix from three vector3 or four vector3 or vector4 inputs. A matrix44 may also be created from vector3 input values, in which case the fourth value will be set to 0.0 for in1-in3, and to 1.0 for in4 when creating the matrix44.

|Input     |Description                                |Type               |Default             |Accepted Values|
|----------|-------------------------------------------|-------------------|--------------------|---------------|
|`in1`     |The vector for the first row of the matrix |vector3 or vector4 |(1,0,0) or (1,0,0,0)|               |
|`in2`     |The vector for the second row of the matrix|vector3 or vector4 |(1,0,0) or (1,0,0,0)|               |
|`in3`     |The vector for the third row of the matrix |vector3 or vector4 |(1,0,0) or (1,0,0,0)|               |
|`in4`     |The vector for the fourth row of the matrix|vector3 or vector4 |(1,0,0) or (1,0,0,0)|               |

<details>

#### Signatures
|`in1`            |`in2`            |`in3`            |`in4`            |`out`            |
|-----------------|-----------------|-----------------|-----------------|-----------------|
|vector3          |vector3          |vector3          |vector3          |matrix33         |
|vector3          |vector3          |vector3          |vector3          |matrix44         |
|vector4          |vector4          |vector4          |vector4          |matrix44         |

#### Behavior

</details>

---

## transpose
Transpose the incoming matrix

|Input     |Description                                          |Type     |Default|Accepted Values|
|----------|-----------------------------------------------------|---------|-------|---------------|
|`in`      |The input matrix                                     |__multi__|__one__|               |
                                                                                   
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|matrix33         |matrix33         |
|matrix44         |matrix44         |

#### Behavior
`out` shall be computed as the transpose of `in`, that is:

```math
\mathrm{out} = \mathrm{in}^T
```

</details>

---

## determinant
Output the determinant of the incoming matrix.

|Input     |Description                                          |Type     |Default|Accepted Values|
|----------|-----------------------------------------------------|---------|-------|---------------|
|`in`      |The input matrix                                     |__multi__|__one__|               |
                                                                                   
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|matrix33         |matrix33         |
|matrix44         |matrix44         |

#### Behavior
`out` shall be computed as the determinant of `in`, that is:

```math
\mathrm{out} = \det(\mathrm{in})
```

</details>

---

## invertmatrix
Invert the incoming matrix.

|Input     |Description                     |Type     |Default |Accepted Values |
|----------|--------------------------------|---------|--------|----------------|
|`in`      |The input matrix                |__multi__|__one__ |                |
                                                               
<details>

#### Signatures
|`in`             |`out`            |
|-----------------|-----------------|
|matrix33         |matrix33         |
|matrix44         |matrix44         |

#### Behavior
`out` shall be computed as the matrix inverse of `in`. If `in` is not invertible, every component out `out` shall be set to `NaN`.

</details>

---

## rotate2d
Rotate the incoming 2D vector about the origin.

|Input     |Description                                                                         |Type   |Default   |Accepted Values|
|----------|------------------------------------------------------------------------------------|-------|----------|---------------|
|`in`      |The input vector to rotate                                                          |vector2|(0, 0)    |               |
|`amount`  |The angle to rotate, specified in degrees. Positive values rotate counter-clockwise |float  |0         |               |

<details>

#### Signatures
|`in`            |`amount`         |`out`            |
|----------------|-----------------|-----------------|
|vector2         |float            |vector2          |

#### Behavior
`out` shall be computed as the right-handed rotation of `in` about the origin by `amount` degrees. That is, `in` shall be multiplied by the matrix:

```math
\begin{bmatrix}
\cos\theta & -\sin\theta \\
\sin\theta & \cos\theta
\end{bmatrix}
```

where:
```math
\theta = \mathrm{amount}
```

</details>

---

## rotate3d
Rotate the incoming 3D vector about the specified unit axis vector.

|Input     |Description                                                                        |Type    |Default   |Accepted Values|
|----------|-----------------------------------------------------------------------------------|--------|----------|---------------|
|`in`      |The input vector to rotate                                                         |vector3 |(0, 0, 0) |               |
|`amount`  |The angle to rotate, specified in degrees. Positive values rotate counter-clockwise|float   |0         |               |
|`axis`    |The unit axis vector to rotate `in` around                                         |vector3 |(0, 1, 0) |               |

<details>

#### Signatures
|`in`            |`amount`         |`out`            |
|----------------|-----------------|-----------------|
|vector2         |float            |vector2          |

#### Behavior
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
\theta &= \mathtt{amount} \notag \\
\alpha  &= \mathtt{axis}  \notag \\
\end{align}
```

</details>

---

## reflect
Reflect the incoming 3D vector about a surface normal vector.

|Input     |Description                                          |Type   |Default    |Accepted Values|
|----------|-----------------------------------------------------|-------|-----------|---------------|
|`in`      |The input vector to reflect                          |vector3| (1, 0, 0) |               |
|`normal`  |The normal vector about which to reflect `in`        |vector3| __Nworld__|               |

<details>

#### Signatures
|`in`            |`normal`         |`out`            |
|----------------|-----------------|-----------------|
|vector3         |vector3          |vector3          |

#### Behavior
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

</details>

---


## refract
Refract the incoming 3D vector through a surface with the given surface normal and relative index of refraction.

|Input     |Description                                          |Type   |Default    |Accepted Values|
|----------|-----------------------------------------------------|-------|-----------|---------------|
|`in`      |The input vector to refract                          |vector3| (1, 0, 0) |               |
|`normal`  |The normal vector through which to refract `in`      |vector3| __Nworld__|               |
|`ior`     |The relative index of refraction of the surface      |float  | 1         |               |

<details>

#### Signatures
|`in`            |`normal`         |`out`            |
|----------------|-----------------|-----------------|
|vector3         |vector3          |vector3          |

#### Behavior
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

</details>

---

