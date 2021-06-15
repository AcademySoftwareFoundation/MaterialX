file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

set(MATERIALX_GLSLANG_DIR "" CACHE STRING "Location of glslang.")
set(MATERIALX_SPIRV_CROSS_DIR "" CACHE STRING "Location of SPIRV-Cross.")

# Please note that glslang modifies the default install directory so it's best
# to set it explicitly with CMAKE_INSTALL_PREFIX in the build script.
if (IS_DIRECTORY ${MATERIALX_GLSLANG_DIR})
    add_subdirectory(${MATERIALX_GLSLANG_DIR} glslang)
endif()
if (NOT TARGET glslang)
    message(FATAL_ERROR "glslang was not found - required for compilation")
endif()
if(WIN32)
    # This is unfortunate but glslang forces our
    # platform toolset to be v110, which we may not even have
    # installed, undo anything glslang has done to it.
    set(CMAKE_GENERATOR_TOOLSET "${OLD_PLATFORM_TOOLSET}" CACHE STRING
        "Platform Toolset" FORCE)
endif()

if (IS_DIRECTORY ${MATERIALX_SPIRV_CROSS_DIR})
    add_subdirectory(${MATERIALX_SPIRV_CROSS_DIR} spirv-cross)
endif()
if (NOT TARGET spirv-cross-core)
    message(FATAL_ERROR "SPIRV-Cross was not found - required for compilation")
endif()

add_library(MaterialXCross STATIC 
    ${materialx_source} 
    ${materialx_header}
)

set_target_properties(
    MaterialXCross PROPERTIES
    OUTPUT_NAME MaterialXCross
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXCross
    glslang OSDependent OGLCompiler HLSL SPIRV
    spirv-cross-core spirv-cross-hlsl
    ${CMAKE_DL_LIBS})

target_include_directories(MaterialXCross
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
        ${glslang_SOURCE_DIR}
        ${MATERIALX_SPIRV_CROSS_DIR}
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS}
)

