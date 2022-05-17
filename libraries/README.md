# MaterialX Data Libraries

This folder contains the standard data libraries for MaterialX, providing declarations and graph definitions for the MaterialX nodes, and source code for all supported shader generators.

## Standard Pattern Library
- [stdlib](stdlib)
    - [stdlib_defs.mtlx](stdlib/stdlib_defs.mtlx) : Nodedef declarations.
    - [stdlib_ng.mtlx](stdlib/stdlib_ng.mtlx) : Nodegraph definitions.
    - [genglsl](stdlib/genglsl): GLSL language support.
        - lib : Shader utility files.
        - [stdlib_genglsl_impl.mtlx](stdlib/genglsl/stdlib_genglsl_impl.mtlx) : Mapping from declarations to implementations.
        - [stdlib_genglsl_cm_impl.mtlx](stdlib/genglsl/stdlib_genglsl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genglsl_unit_impl.mtlx](stdlib/genosl/stdlib_genglsl_unit_impl.mtlx) : Real world unit support implementations.
        - GLSL implementation files.
    - [genosl](stdlib/genosl): OSL language support.
        - lib: Shader utility files.
        - [stdlib_genosl_impl.mtlx](stdlib/genosl/stdlib_genosl_impl.mtlx) : Mapping from declarations to implementations.
        - [stdlib_genosl_cm_impl.mtlx](stdlib/genosl/stdlib_genosl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genosl_unit_impl.mtlx](stdlib/genosl/stdlib_genosl_unit_impl.mtlx) : Real world unit support implementations.
        -  OSL implementation files.
    - [genmdl](stdlib/genmdl): MDL language support.
        - [stdlib_genmdl_impl.mtlx](stdlib/genmdl/stdlib_genmdl_impl.mtlx) : Mapping from declarations to implementations.
        - [stdlib_genmdl_cm_impl.mtlx](stdlib/genmdl/stdlib_genmdl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        - [stdlib_genmdl_unit_impl.mtlx](stdlib/genmdl/stdlib_genmdl_unit_impl.mtlx) : Real world unit support implementations.

## Physically Based Shading Library
- [pbrlib](pbrlib)
    - [pbrlib_defs.mtlx](pbrlib/pbrlib_defs.mtlx) : Nodedef declarations.
    - [pbrlib_ng.mtlx](pbrlib/pbrlib_ng.mtlx) : Nodegraph definitions.
    - [genglsl](pbrlib/genglsl) : GLSL language support
        - lib : Shader utility files.
        - [pbrlib_genglsl_impl.mtlx](pbrlib/genglsl/pbrlib_genglsl_impl.mtlx) : Mapping from declarations to implementations.
        - GLSL implementation files.
    - [genosl](pbrlib/genosl) : OSL language support
        - lib : Utilities
        - [pbrlib_genosl_impl.mtlx](pbrlib/genosl/pbrlib_genosl_impl.mtlx) : Mapping from declarations to implementations.
        - OSL implementation files.
    - [genmdl](pbrlib/genmdl) : MDL language support
        - [pbrlib_genmdl_impl.mtlx](pbrlib/genmdl/pbrlib_genmdl_impl.mtlx) : Mapping from declarations to implementations.
        - Note: MDL implementation files are in a "package" folder found under [source/MaterialXGenMdl/mdl/materialx](../source/MaterialXGenMdl/mdl/materialx)

## BxDF Graph Library
- [bxdf](bxdf)
    - [standard_surface.mtlx](bxdf/standard_surface.mtlx) : Graph definition of the [Autodesk Standard Surface](https://autodesk.github.io/standard-surface/) shading model.
    - [usd_preview_surface.mtlx](bxdf/usd_preview_surface.mtlx) : Graph definition of the [UsdPreviewSurface](https://graphics.pixar.com/usd/docs/UsdPreviewSurface-Proposal.html) shading model.
    - [lama](bxdf/lama) : Graph definitions of the [MaterialX Lama](https://rmanwiki.pixar.com/display/REN24/MaterialX+Lama) node set.

## Target Definitions
- Each target implementation requires a target definition for declaration / implementation correspondence to work.
- The [targets](targets) folder contains definition files for the following core targets:
  - GLSL : `genglsl`
  - OSL : `genosl`
  - MDL : `genmdl`
- Any additional target files should be added under this folder and loaded in as required.

### Target Support
- GLSL target support is for version 4.0 or higher.
- OSL target support is for version 1.9.10 or higher.
- MDL target support is for version 1.6.
- "Default" color management support includes OSL, GLSL, and MDL implementations for the following color spaces:
    - lin_rec709, g18_rec709, g22_rec709, rec709_display, acescg (lin_ap1), g22_ap1, srgb_texture
- Basic GLSL `lightshader` node definitions and implementations are provided for the following light types:
    - point, directional, spot
- Code generation does not currently support:
    - `ambientocclusion` node.
    - `arrayappend` node.
    - `curveadjust` node.
    - `displacementshader` and `volumeshader` nodes and associated operations (`add`, `multiply`, `mix`) for GLSL targets.
