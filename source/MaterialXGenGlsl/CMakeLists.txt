file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")
list(FILTER materialx_headers EXCLUDE REGEX "wgsl/converter/") # internal WGSL helpers; not installed

mx_add_library(MaterialXGenGlsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenHw
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENGLSL_EXPORTS)
