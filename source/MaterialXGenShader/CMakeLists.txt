file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXGenShader
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXFormat
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENSHADER_EXPORTS)

if(MATERIALX_BUILD_OCIO)
    find_package(OpenColorIO REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenColorIO::OpenColorIO)
    target_compile_definitions(${TARGET_NAME} PUBLIC MATERIALX_BUILD_OCIO)
endif()
