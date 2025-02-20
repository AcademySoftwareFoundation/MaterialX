file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.m*")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(APPLE)
    if(NOT MATERIALX_BUILD_APPLE_EMBEDDED)
        find_library(COCOA_FRAMEWORK Cocoa)
        find_package(OpenGL REQUIRED)
    endif()
elseif(UNIX)
    find_package(X11 REQUIRED)
    # Note - can't just require the Xt component because FindX11 in cmake 3.1
    # doesn't support it
    if(NOT X11_Xt_FOUND)
        message(FATAL_ERROR "Error in building MaterialXRenderMsl: Xt was not found")
    endif()

    find_package(OpenGL REQUIRED)
endif()

mx_add_library(MaterialXRenderMsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXRenderHw
        MaterialXGenMsl
    EXPORT_DEFINE
        MATERIALX_RENDERMSL_EXPORTS
    ADD_OBJECTIVE_C_CODE)

if(APPLE)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DGL_SILENCE_DEPRECATION)
elseif(UNIX)
    # public or private here?
    target_include_directories(${TARGET_NAME} PRIVATE ${X11_INCLUDE_DIR})
endif()

# Disable deprecation warnings on Clang.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

if(MSVC)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            Opengl32)
elseif(APPLE)
    if(NOT MATERIALX_BUILD_APPLE_EMBEDDED)
        target_link_libraries(${TARGET_NAME}
                PUBLIC
                "-framework Cocoa"
                "-framework OpenGL")
    endif()
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            "-framework Foundation"
            "-framework Metal"
            "-framework MetalPerformanceShaders")
elseif(UNIX)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            OpenGL::GL
            X11::X11
            X11::Xt)
endif()
