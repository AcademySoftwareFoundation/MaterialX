file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.c*")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_package(OpenGL REQUIRED)
elseif(UNIX)
    find_package(X11 REQUIRED)
    # Note - can't just require the Xt component because FindX11 in cmake 3.1
    # doesn't support it
    if(NOT X11_Xt_FOUND)
        message(FATAL_ERROR "Error in building MaterialXRenderGlsl: Xt was not found")
    endif()

    if(SKBUILD)
        set(OpenGL_GL_PREFERENCE LEGACY)
    endif()

    find_package(OpenGL REQUIRED)
endif()

mx_add_library(MaterialXRenderGlsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXRenderHw
        MaterialXGenGlsl
    EXPORT_DEFINE
        MATERIALX_RENDERGLSL_EXPORTS
    ADD_OBJECTIVE_C_CODE)

if(APPLE)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DGL_SILENCE_DEPRECATION)
elseif(UNIX)
    #TODO - public or private here?
    target_include_directories(${TARGET_NAME} PRIVATE ${X11_INCLUDE_DIR})
endif()

# Disable OpenGL deprecation warnings on Clang.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

if(MATERIALX_BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_NAME} PUBLIC GLAD_GLAPI_EXPORT PRIVATE GLAD_GLAPI_EXPORT_BUILD)
endif()

if(WIN32)
    if(MSVC)
        target_link_libraries(${TARGET_NAME}
            PUBLIC
                Opengl32)
    elseif(MINGW)
        target_link_libraries(${TARGET_NAME}
                PUBLIC
                Opengl32
                gdi32)
    endif()
elseif(APPLE)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            "-framework OpenGL"
            "-framework Foundation"
            "-framework Cocoa"
            "-framework Metal")
elseif(UNIX)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            OpenGL::GL
            X11::X11
            X11::Xt)
endif()
