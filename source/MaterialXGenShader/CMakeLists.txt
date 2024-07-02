file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

set(optionalLibraries "")

if(MATERIALX_BUILD_OCIO)
    find_package(OpenColorIO REQUIRED)
    list(APPEND optionalLibraries OpenColorIO::OpenColorIO)
endif()

mx_add_library(MaterialXGenShader
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    LIBRARIES
        MaterialXFormat
        MaterialXCore
        ${optionalLibraries}
    EXPORT_DEFINE
        MATERIALX_GENSHADER_EXPORTS)

if(MATERIALX_BUILD_OCIO)
    if (NOT MATERIALX_BUILD_MONOLITHIC)
        target_compile_definitions(MaterialXGenShader PUBLIC MATERIALX_BUILD_OCIO)
    else()
        target_compile_definitions(${MATERIALX_MONOLITHIC_TARGET} PUBLIC MATERIALX_BUILD_OCIO)
    endif()
endif()