#
# TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
# All rights reserved.  See LICENSE.txt for license.
#
# Helper CMake module to Find MaterialX include dirs, libraries and document libraries
# Based on https://github.com/PixarAnimationStudios/USD/blob/master/cmake/modules/FindMaterialX.cmake
#
#
# Example Usage
#  - Build and Install MaterialX 
#  - register the path to this cmake module using CMAKE_MODULE_PATH
#  - use find_package(MaterialX REQUIRED)
#
# Variables defined:
# MATERIALX_FOUND            True if headers and requested libraries were found
# MATERIALX_BASE_DIR         MaterialX root installation directory
# MATERIALX_INCLUDE_DIRS     MaterialX include directory
# MATERIALX_LIB_DIRS         MaterialX lib directory
# MATERIALX_CORE_LIBS        MaterialX Core libraries and ShaderX i.e. Core, Format & GenShader
# MATERIALX_GENERATOR_LIBS   MaterialX Generator libraries i.e GenGlsl, GenOsl
# MATERIALX_STDLIB_DIR       Path to the MaterialX standard library directory
# MATERIALX_PBRLIB_DIR       Path to the MaterialX pbr library directory
#

find_path(MATERIALX_BASE_DIR
    NAMES
        include/MaterialXCore/Library.h
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
    )

find_path(MATERIALX_INCLUDE_DIRS 
    MaterialXCore/Library.h
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        include
    DOC
        "MaterialX Header Path"
)

set(MATERIALX_CORE_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}MaterialXCore${CMAKE_STATIC_LIBRARY_SUFFIX})
find_path(MATERIALX_LIB_DIRS 
    "${MATERIALX_CORE_LIB_NAME}"
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        lib
    DOC
        "MaterialX Library Path"
)

# Path to stdlib library
find_path(MATERIALX_STDLIB_DIR 
    stdlib_defs.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/stdlib"
    DOC
        "MaterialX Standard Libraries Path"
)

# Path to pbr library
find_path(MATERIALX_PBRLIB_DIR 
    pbrlib_defs.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/pbrlib"
    DOC
        "MaterialX PBR Libraries Path"
)

# Path to document library
#get_filename_component(MATERIALX_MTLXDOC_DIR ${MATERIALX_STDLIB_DIR} DIRECTORY)

# Core Libraries
foreach(MATERIALX_LIB
    Core
    Format
    Shader)
    find_library(MATERIALX_${MATERIALX_LIB}_LIBRARY
            MaterialX${MATERIALX_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_${MATERIALX_LIB}_LIBRARY)
        list(APPEND MATERIALX_CORE_LIBS ${MATERIALX_${MATERIALX_LIB}_LIBRARY})
    endif ()
endforeach()

# Target generator Libraries 
foreach(MATERIALXGEN_LIB
    Glsl
    Osl
    OgsFx
    Arnold)

    # Locate the genetrator library
    find_library(MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY
            MaterialXGen${MATERIALXGEN_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY)
        list(APPEND MATERIALX_GENERATOR_LIBS ${MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY})
    endif ()
endforeach()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MaterialX
    REQUIRED_VARS
        MATERIALX_BASE_DIR
        MATERIALX_INCLUDE_DIRS
        MATERIALX_LIB_DIRS
        MATERIALX_STDLIB_DIR
        MATERIALX_PBRLIB_DIR
        MATERIALX_CORE_LIBS
        MATERIALX_GENERATOR_LIBS
)