file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_inlined "${CMAKE_CURRENT_SOURCE_DIR}/*.inl")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXRender
    SOURCE_FILES
        ${materialx_source}
    INLINED_FILES
        ${materialx_inlined}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenShader
        MaterialXGenHw # TODO - should this dependency be moved to RenderHw - currently needed for GeometryHandler and LightHandler
    EXPORT_DEFINE
        MATERIALX_RENDER_EXPORTS)

if(UNIX)
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-unused-function)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-stringop-overflow)
    endif()
endif()

if(MATERIALX_BUILD_OIIO)
    find_package(OpenImageIO CONFIG REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)
    target_compile_definitions(${TARGET_NAME} PUBLIC MATERIALX_BUILD_OIIO)
endif()
