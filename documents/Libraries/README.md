# Library Structure

The following is the layout of the definitions and implementations provided as part of the core libraries.

## Standard Library
-   [stdlib](stdlib)
    - [stdlib_defs.mtlx](stdlib/stdlib_defs.mtlx) : Single node nodedef definitions file.
    - [stdlib_ng.mtlx](stdlib/stdlib_ng.mtlx) :  Node graph definitions file.
    -   [genglsl](stdlib/genglsl): GLSL language support
        -   lib : Shader utility files.
        - [stdlib_genglsl_impl.mtlx](stdlib/genglsl/stdlib_genglsl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genglsl_cm_impl.mtlx](stdlib/genglsl/stdlib_genglsl_cm_impl.mtlx) : Minimal set of "default" color management implementations.
        -   GLSL implementation files
        -   [ogsfx](stdlib/genglsl/ogsfx): OGSFX support
            - [stdlib_genglsl_ogsfx_impl.mtlx](stdlib/genglsl/ogsfx/stdlib_genglsl_ogsfx_impl.mtlx) : Mapping from definitions to implementations
            -   OGSFX implementation files
    -   [genosl](stdlib/genosl): OSL language support
        -   lib: Shader utility files.
        - [stdlib_genosl_impl.mtlx](stdlib/genosl/stdlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        - [stdlib_genosl_cm_impl.mtlx](stdlib/genosl/stdlib_genosl_cm_impl.mtlx) : Minimal set of "default" color management implementations
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
        -   [ogsfx](pbrlib/genglsl/ogsfx) : OGSFX support
            - OGSFX implementation files.
    -   [genosl](pbrlib/genosl) : OSL language support
        -   lib : Utilities
        - [pbrlib_genosl_impl.mtlx](pbrlib/genosl/pbrlib_genosl_impl.mtlx) : Mapping from definitions to implementations
        -   OSL implementation files.

Note that there is no reference implementation for the PBR library.
