# MaterialX Data Libraries

The following is the layout of the definitions and implementations provided as part of the core libraries.

These libraries can be used to build shading networks which can be accepted by code generators to produce shader code.

The following core "targets": GLSL, MDL and OSL.

## Standard Library
- [stdlib](stdlib)
    - [stdlib_defs.mtlx](stdlib/stdlib_defs.mtlx) : Single node nodedef definitions file.
    - [stdlib_ng.mtlx](stdlib/stdlib_ng.mtlx) :  Node graph definitions file.
    - [genglsl](stdlib/genglsl): GLSL language support.
        - lib : Shader utility files.
        - [stdlib_genglsl_impl.mtlx](stdlib/genglsl/stdlib_genglsl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genglsl_cm_impl.mtlx](stdlib/genglsl/stdlib_genglsl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genglsl_unit_impl.mtlx](stdlib/genosl/stdlib_genglsl_unit_impl.mtlx) : Real world unit support implementations.
        - GLSL implementation files.
    - [genosl](stdlib/genosl): OSL language support.
        - lib: Shader utility files.
        - [stdlib_genosl_impl.mtlx](stdlib/genosl/stdlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genosl_cm_impl.mtlx](stdlib/genosl/stdlib_genosl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genosl_unit_impl.mtlx](stdlib/genosl/stdlib_genosl_unit_impl.mtlx) : Real world unit support implementations.
        -  OSL implementation files.
    - [osl](stdlib/osl): OSL reference implementation
        -  OSL implementation files.
    - [genmdl](stdlib/genmdl): MDL language support.
        - [stdlib_genmdl_impl.mtlx](stdlib/genosl/stdlib_genmdl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genmdl_cm_impl.mtlx](stdlib/genosl/stdlib_genmdl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genmdl_unit_impl.mtlx](stdlib/genosl/stdlib_genmdl_unit_impl.mtlx) : Real world unit support implementations.

## Physically-Based Shading Library
- [pbrlib](pbrlib)
    - [pbrlib_defs.mtlx](pbrlib/pbrlib_defs.mtlx) : Single node definitions file.
    - [pbrlib_ng.mtlx](pbrlib/pbrlib_ng.mtlx) : Node graph definitions file.
    - [genglsl](pbrlib/genglsl) : GLSL language support
        - lib : Shader utility files.
        - [pbrlib_genglsl_impl.mtlx](pbrlib/genglsl/pbrlib_genglsl_impl.mtlx) : Mapping from definitions to implementations
        - GLSL implementation files.
    - [genosl](pbrlib/genosl) : OSL language support
        - lib : Utilities
        - [pbrlib_genosl_impl.mtlx](pbrlib/genosl/pbrlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        - OSL implementation files.
    - [genmdl](pbrlib/genmdl) : MDL language support
        - [pbrlib_genmdl_impl.mtlx](pbrlib/genosl/pbrlib_genmdl_impl.mtlx) : Mapping from definitions to implementations.
        - Note: MDL implementation files are in a "package" folder found under
        [source/MaterialXGenMdl/mdl/materialx](../source/MaterialXGenMdl/mdl/materialx)

## Target Definitions
- Each target implementation requires a target definition for definition / implementation correspondence to work.
- [targets](targets) is the folder holding documents which declare these definitions.
- There are definition files for the following core targets:
  - OSL : `genosl`
  - Desktop GLSL : `genglsl`
  - MDL : `genmdl`
- Any additional target files should be added under this folder and loaded in as required.

### Target Support
- GLSL target support is for version 4.0 or higher.
- OSL target support is for version 1.9.10 or higher.
- MDL target support is for version 1.6.
- "Default" color management support includes OSL,  GLSL, and MDL implementations for the following non-LUT transforms:
    - lin_rec709, gamma18, gamma22, gamma24, acescg, g22_acescg, srgb_texture
- Basic GLSL `lightshader` node definitions and implementations are provided for the following light types:
    - point, directional, spot
- Code generation does not currently support:
    - `ambientocclusion` node.
    - `arrayappend` node.
    - `curveadjust` node.
    - `displacementshader` and `volumeshader` nodes and associated operations (`add`, `multiply`, `mix`) for GLSL targets.
