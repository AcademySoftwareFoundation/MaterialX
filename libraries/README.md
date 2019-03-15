# Library Structure

The following is the layout of the definitions and implementations provided as part of the core libraries.

- `osl` denotes a reference implementation for OSL.
- `genosl` denotes "generated" OSL code support. The language specifier is `genosl` and the target is `vanilla`.
- `genglsl` denotes "generated" GLSL code  support. The language specifier is `genglsl` with no specific target specifier.

## Standard Library
-   [stdlib](stdlib)
    - [stdlib_defs.mtlx](stdlib/stdlib_defs.mtlx) : Single node nodedef definitions file.
    - [stdlib_ng.mtlx](stdlib/stdlib_ng.mtlx) :  Node graph definitions file.
    -   [genglsl](stdlib/genglsl): GLSL language support.
        -   lib : Shader utility files.
        - [stdlib_genglsl_impl.mtlx](stdlib/genglsl/stdlib_genglsl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genglsl_cm_impl.mtlx](stdlib/genglsl/stdlib_genglsl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        -   GLSL implementation files.
    -   [genosl](stdlib/genosl): OSL language support.
        -   lib: Shader utility files.
        - [stdlib_genosl_impl.mtlx](stdlib/genosl/stdlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genosl_cm_impl.mtlx](stdlib/genosl/stdlib_genosl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        -  OSL implementation files.
    -   [osl](stdlib/osl): OSL reference implementation
        -  OSL implementation files.

## PBR library
-   [pbrlib](pbrlib)
    - [pbrlib_defs.mtlx](pbrlib/pbrlib_defs.mtlx) : Single node definitions file.
    - [pbrlib_ng.mtlx](pbrlib/pbrlib_ng.mtlx) : Node graph definitions file.
    -   [genglsl](pbrlib/genglsl) : GLSL language support
        - lib : Shader utility files.
        - [pbrlib_genglsl_impl.mtlx](pbrlib/genglsl/pbrlib_genglsl_impl.mtlx) : Mapping from definitions to implementations
        -   GLSL implementation files.
    -   [genosl](pbrlib/genosl) : OSL language support
        -   lib : Utilities
        - [pbrlib_genosl_impl.mtlx](pbrlib/genosl/pbrlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        -   OSL implementation files.

## Support Notes:
- GLSL language support is for version 4.0 or higher.
- "default" color management support includes OSL and GLSL implementations for the following non-LUT transforms:
    - lin_rec709, gamma18, gamma22, gamma24, acescg, srgb_texture
- Basic GLSL `lightshader` node definitions and implementations are provided for the following light types:
    - point, directional, spot
- Code generation is not currently supported for:
    - `noise2d`, `noise3d`, `cellnoise2d`, `cellnoise3d`, `fractal3d` nodes for `genglsl`
    - `ambientocclusion` node for: `genosl` and `genglsl`.
    - `arrayappend` node for: `genosl` and `genglsl`.
    - `curveadjust` node for: `genosl` and `genglsl`.
    - `displacementshader` and `volumeshader` nodes and associated operations (`add`, `multiply`, `mix`) for: `genosl` and `genglsl`.
    - `add`, `multiply`, and `mix surfaceshader` node operations for `genglsl`.
