project(MaterialX)
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 36)
set(MATERIALX_BUILD_VERSION 4)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
enable_testing()

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 2.7 or greater." OFF)
option(MATERIALX_BUILD_VIEWER "Build the MaterialX Viewer." OFF)
option(MATERIALX_BUILD_DOCS "Create HTML documentation using Doxygen. Requires that Doxygen be installed." OFF)
option(MATERIALX_PYTHON_LTO "Enable link-time optimizations for MaterialX Python." ON)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)

set(MATERIALX_PYTHON_VERSION "" CACHE STRING
    "Python version to be used in building the MaterialX Python package (e.g. '2.7').")
set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH
    "Python executable to be used in building the MaterialX Python package (e.g. 'C:/Python27/python.exe').")
set(MATERIALX_PYTHON_OCIO_DIR "" CACHE PATH
    "Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python (e.g. 'D:/Projects/OpenColorIO-Configs/aces_1.0.3').")
set(MATERIALX_PYTHON_PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/PyMaterialX/PyBind11" CACHE PATH
    "Path to a folder containing the PyBind11 source. Defaults to the included PyBind11 source, which has been extended to support Python 2.6.")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_PYTHON_LTO)
mark_as_advanced(MATERIALX_INSTALL_PYTHON)
mark_as_advanced(MATERIALX_WARNINGS_AS_ERRORS)
mark_as_advanced(MATERIALX_PYTHON_VERSION)
mark_as_advanced(MATERIALX_PYTHON_EXECUTABLE)
mark_as_advanced(MATERIALX_PYTHON_OCIO_DIR)
mark_as_advanced(MATERIALX_PYTHON_PYBIND11_DIR)

# Validation options for compiling and rendering generated shaders
option(MATERIALX_TEST_RENDER "Run rendering tests for MaterialX Render module. GPU required for graphics validation." OFF)
mark_as_advanced(MATERIALX_TEST_RENDER)
option(MATERIALX_BUILD_OIIO "Build OpenImageIO support for MaterialXRender." OFF)
mark_as_advanced(MATERIALX_BUILD_OIIO)
set(MATERIALX_OIIO_DIR "" CACHE PATH "Path to the root folder of the OpenImageIO installation.")
mark_as_advanced(MATERIALX_OIIO_DIR)

# Helpers for OSL validation
set(MATERIALX_OSLC_EXECUTABLE "" CACHE FILEPATH "Full path to the oslc binary.")
mark_as_advanced(MATERIALX_OSLC_EXECUTABLE)
add_definitions(-DMATERIALX_OSLC_EXECUTABLE=\"${MATERIALX_OSLC_EXECUTABLE}\")
set(MATERIALX_TESTRENDER_EXECUTABLE "" CACHE FILEPATH "Full path to the testrender binary.")
mark_as_advanced(MATERIALX_TESTRENDER_EXECUTABLE)
add_definitions(-DMATERIALX_TESTRENDER_EXECUTABLE=\"${MATERIALX_TESTRENDER_EXECUTABLE}\")
set(MATERIALX_OSL_INCLUDE_PATH "" CACHE PATH "Full path to osl include paths. e.g. location of stdosl.h")
mark_as_advanced(MATERIALX_OSL_INCLUDE_PATH)
add_definitions(-DMATERIALX_OSL_INCLUDE_PATH=\"${MATERIALX_OSL_INCLUDE_PATH}\")

# Adjust the default installation path
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "Default install path" FORCE)
endif()

# Adjust compiler settings
if(MSVC)
    add_compile_options(/MP)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        add_compile_options(/W4)
    endif()
    if(MATERIALX_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wno-missing-braces)
    if(MATERIALX_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# Shared functions
function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# Add core subdirectories
add_subdirectory(source/MaterialXCore)
add_subdirectory(source/MaterialXFormat)

# Add shader generation subdirectories
add_subdirectory(source/MaterialXGenShader)
add_subdirectory(source/MaterialXGenOsl)
add_subdirectory(source/MaterialXGenGlsl)

# Add render module subdirectories
if(MATERIALX_BUILD_OIIO)
    add_definitions(-DMATERIALX_BUILD_OIIO)
    add_definitions(-DOPENIMAGEIO_ROOT_DIR=\"${OPENIMAGEIO_ROOT_DIR}\")
endif()
add_subdirectory(source/MaterialXRender)
add_subdirectory(source/MaterialXRenderOsl)
add_subdirectory(source/MaterialXRenderHw)
add_subdirectory(source/MaterialXRenderGlsl)

# Add viewer subdirectory
if(MATERIALX_BUILD_VIEWER)
    add_subdirectory(source/MaterialXView)
endif()

# Add test subdirectory
if(MATERIALX_TEST_RENDER)
    add_definitions(-DMATERIALX_TEST_RENDER)
endif()
add_subdirectory(source/MaterialXTest)

# Add resource and library subdirectories
add_subdirectory(libraries)
add_subdirectory(resources)

if(MATERIALX_BUILD_PYTHON)
    add_subdirectory(source/PyMaterialX)
    add_subdirectory(python)
endif()
if(MATERIALX_BUILD_DOCS)
    add_subdirectory(documents)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.6.2")
    if(MATERIALX_BUILD_VIEWER)
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXView)
    else()
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXTest)
    endif()
endif()

# Install MaterialX find_package module
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}" MESSAGE_NEVER)
