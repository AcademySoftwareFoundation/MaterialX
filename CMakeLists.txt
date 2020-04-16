project(MaterialX)
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 37)
set(MATERIALX_BUILD_VERSION 1)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
enable_testing()

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 2.7 or greater." OFF)
option(MATERIALX_BUILD_VIEWER "Build the MaterialX Viewer." OFF)
option(MATERIALX_BUILD_DOCS "Create HTML documentation using Doxygen. Requires that Doxygen be installed." OFF)

option(MATERIALX_BUILD_GEN_GLSL "Build the GLSL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_OSL "Build the OSL shader generator back-end." ON)
option(MATERIALX_BUILD_RENDER "Build the MaterialX Render modules." ON)

option(MATERIALX_PYTHON_LTO "Enable link-time optimizations for MaterialX Python." ON)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)

option(MATERIALX_TEST_RENDER "Run rendering tests for MaterialX Render module. GPU required for graphics validation." ON)
option(MATERIALX_BUILD_OIIO "Build OpenImageIO support for MaterialXRender." OFF)
option(MATERIALX_BUILD_TESTS "Build unit tests." ON)

if (NOT MATERIALX_BUILD_PYTHON)
    set(MATERIALX_PYTHON_LTO OFF)
    set(MATERIALX_INSTALL_PYTHON OFF)
endif()

if (MATERIALX_BUILD_RENDER)
    set(MATERIALX_BUILD_OSL ON)
    set(MATERIALX_BUILD_GLSL ON)
else()
    set(MATERIALX_TEST_RENDER OFF)
    set(MATERIALX_BUILD_VIEWER OFF)
    set(MATERIALX_BUILD_OIIO OFF)
endif()

if (MATERIALX_BUILD_GEN_GLSL OR MATERIALX_BUILD_GEN_OSL)
    set(MATERIALX_BUILD_GEN ON)
else()
    set(MATERIALX_BUILD_GEN OFF)
endif()

set(MATERIALX_DEBUG_POSTFIX "" CACHE STRING "Postfix to append to the target file name for debug configuration (e.g. 'd').")
set(MATERIALX_PYTHON_VERSION "" CACHE STRING "Python version to be used in building the MaterialX Python package (e.g. '2.7').")
set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH "Python executable to be used in building the MaterialX Python package (e.g. 'C:/Python27/python.exe').")
set(MATERIALX_PYTHON_OCIO_DIR "" CACHE PATH "Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python (e.g. 'D:/Projects/OpenColorIO-Configs/aces_1.0.3').")
set(MATERIALX_PYTHON_PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/PyMaterialX/PyBind11" CACHE PATH "Path to a folder containing the PyBind11 source. Defaults to the included PyBind11 source, which has been extended to support Python 2.6.")
set(MATERIALX_OIIO_DIR "" CACHE PATH "Path to the root folder of the OpenImageIO installation.")

# Settings to define installation layout
set(MATERIALX_INSTALL_INCLUDE_PATH "include" CACHE STRING "Install header include path (e.g. 'inc', 'include').")
set(MATERIALX_INSTALL_LIB_PATH "lib" CACHE STRING "Install lib path (e.g. 'libs', 'lib').")
set(MATERIALX_INSTALL_STDLIB_PATH "libraries" CACHE STRING "Install path for mtlx std libs (e.g. 'libraries').")

# Helpers for OSL validation
set(MATERIALX_OSLC_EXECUTABLE "" CACHE FILEPATH "Full path to the oslc binary.")
set(MATERIALX_TESTRENDER_EXECUTABLE "" CACHE FILEPATH "Full path to the testrender binary.")
set(MATERIALX_OSL_INCLUDE_PATH "" CACHE PATH "Full path to osl include paths. e.g. location of stdosl.h")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_WARNINGS_AS_ERRORS)
mark_as_advanced(MATERIALX_BUILD_GEN)
mark_as_advanced(MATERIALX_BUILD_RENDER)
mark_as_advanced(MATERIALX_PYTHON_LTO)
mark_as_advanced(MATERIALX_INSTALL_PYTHON)
mark_as_advanced(MATERIALX_TEST_RENDER)
mark_as_advanced(MATERIALX_BUILD_OIIO)
mark_as_advanced(MATERIALX_DEBUG_POSTFIX)
mark_as_advanced(MATERIALX_PYTHON_VERSION)
mark_as_advanced(MATERIALX_PYTHON_EXECUTABLE)
mark_as_advanced(MATERIALX_PYTHON_OCIO_DIR)
mark_as_advanced(MATERIALX_PYTHON_PYBIND11_DIR)
mark_as_advanced(MATERIALX_OIIO_DIR)
mark_as_advanced(MATERIALX_OSLC_EXECUTABLE)
mark_as_advanced(MATERIALX_TESTRENDER_EXECUTABLE)
mark_as_advanced(MATERIALX_OSL_INCLUDE_PATH)

add_definitions(-DMATERIALX_OSLC_EXECUTABLE=\"${MATERIALX_OSLC_EXECUTABLE}\")
add_definitions(-DMATERIALX_TESTRENDER_EXECUTABLE=\"${MATERIALX_TESTRENDER_EXECUTABLE}\")
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
if (MATERIALX_BUILD_GEN)
    # TODO: split up libraries to only install those relevant to enabled generators
    add_subdirectory(libraries)
    add_subdirectory(source/MaterialXGenShader)
    if (MATERIALX_BUILD_GEN_GLSL)
        add_subdirectory(source/MaterialXGenGlsl)
    endif()
    if (MATERIALX_BUILD_GEN_OSL)
        add_subdirectory(source/MaterialXGenOsl)
    endif()
endif()

# Add render module subdirectories
if (MATERIALX_BUILD_RENDER)
    if(MATERIALX_BUILD_OIIO)
        add_definitions(-DMATERIALX_BUILD_OIIO)
        add_definitions(-DOPENIMAGEIO_ROOT_DIR=\"${OPENIMAGEIO_ROOT_DIR}\")
    endif()
    add_subdirectory(source/MaterialXRender)
    add_subdirectory(source/MaterialXRenderOsl)
    add_subdirectory(source/MaterialXRenderHw)
    add_subdirectory(source/MaterialXRenderGlsl)
    add_subdirectory(resources)

    if(MATERIALX_TEST_RENDER)
        add_definitions(-DMATERIALX_TEST_RENDER)
    endif()

    # Add viewer subdirectory
    if(MATERIALX_BUILD_VIEWER)
        add_subdirectory(source/MaterialXView)
    endif()
endif()

if(MATERIALX_BUILD_PYTHON)
    add_subdirectory(source/PyMaterialX)
    add_subdirectory(python)
endif()

if(MATERIALX_BUILD_DOCS)
    add_subdirectory(documents)
endif()

if(MATERIALX_BUILD_TESTS)
    add_subdirectory(source/MaterialXTest)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.6.2")
    if(MATERIALX_BUILD_VIEWER)
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXView)
    else()
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXTest)
    endif()
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/cmake/MaterialXConfigVersion.cmake
                                 VERSION ${CMAKE_VERSION}
                                 COMPATIBILITY AnyNewerVersion)

# Package for using the developer build
export(EXPORT MaterialX
       FILE ${CMAKE_BINARY_DIR}/cmake/MaterialXConfig.cmake)

# Package for using the installed build
install(EXPORT MaterialX
        DESTINATION cmake
        FILE MaterialXConfig.cmake)
install(FILES "${CMAKE_BINARY_DIR}/cmake/MaterialXConfigVersion.cmake"
        DESTINATION cmake)
