# Front matter

## Input and Output Types

Unless explicitly specified in the Signature and Parameter tables, each node has a single output, named `out`.

Where an input type is specified as __multi__, the exact type is dependent on the specific node overload. See the Signature table for a complete list of all overload types for each node. 

__zero__ is defined to be the additive identity for each type, that is:

|Type       |Value                 |
|-----------|----------------------|
|float      | 0.0                  |
|integer    | 0                    |
|boolean    | false                |
|vector2    | [0.0, 0.0]           |
|vector3    | [0.0, 0.0, 0.0]      |
|vector4    | [0.0, 0.0, 0.0, 0.0] |
|color3     | [0.0, 0.0, 0.0]      |
|color4     | [0.0, 0.0, 0.0, 0.0] |
|matrix33   | [0.0, ... 0.0]       |
|matrix44   | [0.0, ... 0.0]       |
|string     | ""                   |
|filename   | ""                   |
|array      | empty array          |

__one__ is defined to be the multiplicative identity for each type, that is:

|Type       |Value                 |
|-----------|----------------------|
|float      | 1.0                  |
|integer    | 1                    |
|boolean    | true                 |
|vector2    | [1.0, 1.0]           |
|vector3    | [1.0, 1.0, 1.0]      |
|vector4    | [1.0, 1.0, 1.0, 1.0] |
|color3     | [1.0, 1.0, 1.0]      |
|color4     | [1.0, 1.0, 1.0, 1.0] |
|matrix33   | identity matrix      |
|matrix44   | identity matrix      |
|string     | undefined            |
|filename   | undefined            |
|array      | undefined            |


## Value Ranges

The Signature table has a column, "Accepted values" which contains the range of values that the node is expected to generate a correct result for. Values outside of this range generate undefined behavior. 

An empty "Accepted values" field indicates that any value representable in the underlying data type is valid.

## Equations

The Details section of each node definition contains equations where appropriate. In these equations, node inputs are written in $\mathtt{monospaced \space font}$, and may be aliased to single-letter variables where it helps for legibility in the equations, for example:

```math
\begin{align}

\mathtt{out} &= \omega \cdot N \notag \\
\omega &= \mathtt{input} \notag \\
N &= \mathtt{normal} \notag 

\end{align}
```

Many nodes describe component-wise operations where the sample equation must be applied individually to each component of a vector or matrix, or directly to a scalar value. These are represented using subscript notation for the component indices, with indices counting from 0.  

Since many nodes are polymorphic over input types, equations are written for the highest-rank type and when reading for lower-rank types, the higher dimensions should be ignored. For instance, `add` is defined as:

```math
\mathtt{out}_{i,j} = \mathtt{in1}_{i,j} + \mathtt{in2}_{i,j}
```

which is appropriate for the `matrix33` and `matrix44` types, and should be read as:

```math
\mathtt{out}_{i} = \mathtt{in1}_{i} + \mathtt{in2}_{i}
```

for `vectorN` and `colorN` types, and:

```math
\mathtt{out} = \mathtt{in1} + \mathtt{in2}
```

for scalars.


## Example

The below shows a dummy example node. The first section gives the high-level, user-facing description of what the node does and all the inputs to the node, including their defaults and the accepted range of values, if applicable. More exact definitions are provided in the `details` rollout.

The `Signatures` section gives the full signature table, where each row is one overload of the node, and each column specifies the types of each parameter for that overload.

The `Behaviour` section specifies the exact behavior of the node, including equations for calculating the result and a normative description of expected behavior. In equations, node parameters are written in <tt>monospaced font</tt>, and aliased with variables where it helps readability.

### foobar
Gives the foonifaction of `in`. `in` must be greater than or equal to one.

|Input     |Description                                         |Type      |Default    |Accepted Values|
|----------|----------------------------------------------------|----------|-----------|---------------|
|`in`      |The input to foo                                    |__multi__ | __one__   | [1, +inf)     |
|`normal`  |The normal vector through which to foo `in`         |vector3   | __Nworld__|               |
|`ior`     |The relative index of refraction of the surface     |float     | 1         | (0, +inf)     |
|`name`    |A name to give to the foo                           |string    |"bar"      |               |
|`coordsys`|Coordinate system in which to foo                   |string    |"world"    |               |
|`mode`    |Scattering mode of foo                              |string    |"R"        |"R", "T", "TRT"|

<details>

#### Signatures
|`in`            |`normal`         |`ior`            |`name`          |`cordsys`        |`mode`           |`out`            |
|----------------|-----------------|-----------------|----------------|-----------------|-----------------|-----------------|
|float           |vector3          |float            |string          |string           |string           |float            |
|vector3         |vector3          |float            |string          |string           |string           |vector3          |

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
