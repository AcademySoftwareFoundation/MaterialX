#
# TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
# All rights reserved.  See LICENSE.txt for license.
#
# Helper CMake module to Find MaterialX include dirs, libraries and document libraries
#
# Example Usage
#  - Build and Install MaterialX
#  - register the path to this cmake module using CMAKE_MODULE_PATH
#  - use find_package(MaterialX REQUIRED) to locate core components
#  - additonal components can be found using shadergen, bxdf and pytton
#     e.g:  find_package(MaterialX REQUIRED COMPONENTS shadergen)
#           will find inc, lib, corelib and generators
#
# Variables defined:
# MATERIALX_FOUND            True if headers and requested libraries were found
# MATERIALX_BASE_DIR         MaterialX root installation directory
# MATERIALX_INCLUDE_DIRS     MaterialX include directory
# MATERIALX_LIB_DIRS         MaterialX lib directory
# MATERIALX_CORE_LIBS        MaterialX Core libraries and Shader Generation i.e. Core, Format & GenShader
# MATERIALX_GENERATOR_LIBS   MaterialX Generator libraries i.e GenGlsl, GenOsl
# MATERIALX_RENDER_LIBS      MaterialX Render libraries i.e RenderGlsl, RenderOsl
# MATERIALX_STDLIB_DIR       Path to the MaterialX standard library directory
# MATERIALX_PBRLIB_DIR       Path to the MaterialX pbr library directory
# MATERIALX_BXDFLIB_DIR      Path to the Surface shaders library directory e.g. standard surface
# MATERIALX_PYTHON_DIR       Path to MaterialX Python library
# MATERIALX_RESOURCES_DIR    Path to MaterialX Resources (sample data, mtlx etc)
#

# make core variables required
set (MATERIALX_REQUIRED_VARS
        MATERIALX_INCLUDE_DIRS
        MATERIALX_LIB_DIRS
        MATERIALX_STDLIB_DIR
        MATERIALX_CORE_LIBS
    )

# make shadergen required if requested
if ("shadergen" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS
        MATERIALX_GENERATOR_LIBS
        MATERIALX_RENDER_LIBS)
endif()

# make bxdf surfacce shaders required if requested
if ("bxdf" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS
        MATERIALX_PBRLIB_DIR
        MATERIALX_BXDFLIB_DIR)
endif()


if ("python" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS
        MATERIALX_PYTHON_DIR
        MATERIALX_RESOURCES_DIR)
endif()



# Locate MaterialX base directory based on ENV var
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

# Path to surface shader library
find_path(MATERIALX_BXDFLIB_DIR
    standard_surface.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/bxdf"
    DOC
        "MaterialX Surface shaders Libraries Path"
)

# Path to python library
find_path(MATERIALX_PYTHON_DIR
    setup.py
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "python"
    DOC
        "MaterialX Python Bindings Path"
)

# Path to MaterialX Resources
find_path(MATERIALX_RESOURCES_DIR
    README.md
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "resources"
    DOC
        "MaterialX Resources path"
)



# Core Libraries
foreach(MATERIALX_LIB
    Core
    Format
    GenShader
    Render
    Contrib)
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

# Target render Libraries
foreach(MATERIALXRENDER_LIB
    Glsl
    Osl
    Hw)

    # Locate the render library
    find_library(MATERIALX_RENDER_${MATERIALXRENDER_LIB}_LIBRARY
            MaterialXRender${MATERIALXRENDER_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_RENDER_${MATERIALXRENDER_LIB}_LIBRARY)
        list(APPEND MATERIALX_RENDER_LIBS ${MATERIALX_GEN_${MATERIALXRENDER_LIB}_LIBRARY})
    endif ()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MaterialX
    REQUIRED_VARS ${MATERIALX_REQUIRED_VARS}
)
