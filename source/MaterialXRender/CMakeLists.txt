file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_inlined "${CMAKE_CURRENT_SOURCE_DIR}/*.inl")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(NOT MATERIALX_BUILD_OIIO)
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/OiioImageLoader.cpp")
endif()

mx_add_library(MaterialXRender
    SOURCE_FILES
        ${materialx_source} ${materialx_inlined}
    HEADER_FILES
        ${materialx_headers}
    LIBRARIES
        MaterialXGenShader
    EXPORT_DEFINE
        MATERIALX_RENDER_EXPORTS)

if(UNIX)
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-unused-function)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-stringop-overflow)
    endif()
endif()

if(MATERIALX_BUILD_OIIO)
    set(OPENIMAGEIO_ROOT_DIR ${MATERIALX_OIIO_DIR})
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/OpenImageIO")
    find_package(OpenImageIO CONFIG REQUIRED)
    target_link_libraries(${TARGET_NAME} OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)
endif()
