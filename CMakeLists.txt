option(MATERIALX_BUILD_JS "Build the MaterialX Javascript package from C++ bindings. Requires the emscripten environment." OFF)
set(MATERIALX_EMSDK_PATH "" CACHE PATH "Path to EMSDK (e.g. 'C:/emsdk').")

if (MATERIALX_BUILD_JS)
    # Set the the emsdk path
    if (EXISTS "${MATERIALX_EMSDK_PATH}")        
        set(EMSDK_PATH ${MATERIALX_EMSDK_PATH})
    elseif (EXISTS $ENV{EMSDK})
        set(EMSDK_PATH $ENV{EMSDK})
    endif()

    if (EMSDK_PATH)
        # NOTE: Setting the CMAKE_TOOLCHAIN_FILE needs to happen before calling project()
        set(CMAKE_TOOLCHAIN_FILE "${MATERIALX_EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    else()
        message("The EMSDK path was not supplied, disabling MATERIALX_BUILD_JS")
        set(MATERIALX_BUILD_JS OFF)
    endif()
endif()

project(MaterialX)
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 38)
set(MATERIALX_BUILD_VERSION 0)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
enable_testing()

option(MATERIALX_BUILD_DOCS "Create HTML documentation using Doxygen. Requires that Doxygen be installed." OFF)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)

# Shading language generation options
option(MATERIALX_BUILD_GEN_GLSL "Build the GLSL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_OSL "Build the OSL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_MDL "Build the MDL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_OGSXML "Build the OGSXML shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_OGSFX "Build the OGSFX shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_ARNOLD "Build the Arnold OSL shader generator back-end." ON)

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 2.7 or greater." OFF)
option(MATERIALX_BUILD_RUNTIME "Build the MaterialX Runtime data model library." OFF)
option(MATERIALX_BUILD_RENDER "Build the MaterialX Render modules." ON)
option(MATERIALX_BUILD_CROSS "Build MaterialXCross folder." OFF)

if(MATERIALX_DISABLE_BUILD_RENDER)
    message(WARNING "MATERIALX_DISABLE_BUILD_RENDER is deprecated. Please use MATERIALX_BUILD_RENDER instead")
    set(MATERIALX_BUILD_RENDER OFF)
endif()

option(MATERIALX_PYTHON_LTO "Enable link-time optimizations for MaterialX Python." ON)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)

option(MATERIALX_TEST_RENDER "Run rendering tests for MaterialX Render module. GPU required for graphics validation." ON)
option(MATERIALX_BUILD_OIIO "Build OpenImageIO support for MaterialXRender." OFF)
option(MATERIALX_BUILD_VIEWER "Build the MaterialX Viewer." ON)
option(MATERIALX_BUILD_CONTRIB "Build contribution folder." ON)
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
    set(MATERIALX_BUILD_CONTRIB OFF)
    set(MATERIALX_BUILD_OIIO OFF)
endif()

if (MATERIALX_BUILD_GEN_ARNOLD)
    set(MATERIALX_BUILD_GEN_OSL ON)
endif()

if (MATERIALX_BUILD_GEN_OGSFX)
    set(MATERIALX_BUILD_GEN_GLSL ON)
endif()

if (MATERIALX_BUILD_GEN_OGSXML)
    set(MATERIALX_BUILD_GEN_GLSL ON)
endif()

if (MATERIALX_BUILD_GEN_GLSL OR MATERIALX_BUILD_GEN_OSL OR MATERIALX_BUILD_GEN_OGSXML OR MATERIALX_BUILD_GEN_OGSFX OR MATERIALX_BUILD_GEN_ARNOLD)
    set(MATERIALX_BUILD_GEN ON)
else()
    set(MATERIALX_BUILD_GEN OFF)
endif()

set(MATERIALX_PYTHON_VERSION "" CACHE STRING
    "Python version to be used in building the MaterialX Python package (e.g. '2.7').")
set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH
    "Python executable to be used in building the MaterialX Python package (e.g. 'C:/Python27/python.exe').")
set(MATERIALX_PYTHON_OCIO_DIR "" CACHE PATH
    "Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python (e.g. 'D:/Projects/OpenColorIO-Configs/aces_1.0.3').")
set(MATERIALX_PYTHON_PYBIND11_DIR "" CACHE PATH
    "Path to a folder containing the PyBind11 source to be used in building MaterialX Python.")

set(MATERIALX_OIIO_DIR "" CACHE PATH "Path to the root folder of the OpenImageIO installation.")
set(MATERIALX_DEBUG_POSTFIX "" CACHE STRING "Postfix to append to the target file name for debug configuration (e.g. 'd').")

# Helpers for OSL validation
set(MATERIALX_OSLC_EXECUTABLE "" CACHE FILEPATH "Full path to the oslc binary.")
set(MATERIALX_TESTRENDER_EXECUTABLE "" CACHE FILEPATH "Full path to the testrender binary.")
set(MATERIALX_OSL_INCLUDE_PATH "" CACHE PATH "Full path to osl include paths. e.g. location of stdosl.h")

# Helpers for MDL validation
if (MATERIALX_BUILD_GEN_MDL)
    set(MATERIALX_MDLC_EXECUTABLE "" CACHE FILEPATH "Full path to the mdlc binary.")
    set(MATERIALX_MDL_RENDER_EXECUTABLE "" CACHE FILEPATH "Full path to the mdl renderer binary.")
    set(MATERIALX_MDL_RENDER_ARGUMENTS "" CACHE STRING "Custom arguments for renderer.")
    set(MATERIALX_MDL_MODULE_PATHS "" CACHE FILEPATH "Comma separated list of MDL module paths.")
    set(MATERIALX_INSTALL_MDL_MODULE_PATH ${MATERIALX_INSTALL_STDLIB_PATH} CACHE FILEPATH "Install path for mdl module.")
endif()

# Settings to define installation layout
set(MATERIALX_INSTALL_INCLUDE_PATH "include" CACHE STRING "Install header include path (e.g. 'inc', 'include').")
set(MATERIALX_INSTALL_LIB_PATH "lib" CACHE STRING "Install lib path (e.g. 'libs', 'lib').")
set(MATERIALX_INSTALL_STDLIB_PATH "libraries" CACHE STRING "Install path for mtlx std libs (e.g. 'libraries').")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_WARNINGS_AS_ERRORS)
mark_as_advanced(MATERIALX_BUILD_GEN_OGSXML)
mark_as_advanced(MATERIALX_BUILD_GEN_OGSFX)
mark_as_advanced(MATERIALX_BUILD_GEN_ARNOLD)
mark_as_advanced(MATERIALX_BUILD_GEN)
mark_as_advanced(MATERIALX_BUILD_RENDER)
mark_as_advanced(MATERIALX_BUILD_CROSS)
mark_as_advanced(MATERIALX_PYTHON_LTO)
mark_as_advanced(MATERIALX_INSTALL_PYTHON)
mark_as_advanced(MATERIALX_TEST_RENDER)
mark_as_advanced(MATERIALX_BUILD_CONTRIB)
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
mark_as_advanced(MATERIALX_INSTALL_INCLUDE_PATH)
mark_as_advanced(MATERIALX_INSTALL_LIB_PATH)
mark_as_advanced(MATERIALX_INSTALL_STDLIB_PATH)
if (MATERIALX_BUILD_GEN_MDL)
    mark_as_advanced(MATERIALX_MDLC_EXECUTABLE)
    mark_as_advanced(MATERIALX_MDL_RENDER_EXECUTABLE)
    mark_as_advanced(MATERIALX_MDL_RENDER_ARGUMENTS)
    mark_as_advanced(MATERIALX_MDL_MODULE_PATHS)
    mark_as_advanced(MATERIALX_INSTALL_MDL_MODULE_PATH)
endif()

# Add global definitions
add_definitions(-DMATERIALX_OSLC_EXECUTABLE=\"${MATERIALX_OSLC_EXECUTABLE}\")
add_definitions(-DMATERIALX_TESTRENDER_EXECUTABLE=\"${MATERIALX_TESTRENDER_EXECUTABLE}\")
add_definitions(-DMATERIALX_OSL_INCLUDE_PATH=\"${MATERIALX_OSL_INCLUDE_PATH}\")
if(MATERIALX_BUILD_OIIO)
    add_definitions(-DMATERIALX_BUILD_OIIO)
endif()
if(MATERIALX_TEST_RENDER)
    add_definitions(-DMATERIALX_TEST_RENDER)
endif()

if (MATERIALX_BUILD_GEN_MDL)
    add_definitions(-DMATERIALX_MDLC_EXECUTABLE=\"${MATERIALX_MDLC_EXECUTABLE}\")
    add_definitions(-DMATERIALX_MDL_RENDER_EXECUTABLE=\"${MATERIALX_MDL_RENDER_EXECUTABLE}\")
    add_definitions(-DMATERIALX_MDL_RENDER_ARGUMENTS=\"${MATERIALX_MDL_RENDER_ARGUMENTS}\")
    add_definitions(-DMATERIALX_MDL_MODULE_PATHS=\"${MATERIALX_MDL_MODULE_PATHS}\")
    add_definitions(-DMATERIALX_INSTALL_MDL_MODULE_PATH=\"${MATERIALX_INSTALL_MDL_MODULE_PATH}\")
endif()

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

    # Use /Z7 to avoid having pdbs
    if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/Zi")
        string(REGEX REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
    endif()
    if(CMAKE_CXX_FLAGS_RELEASE MATCHES "/Zi")
        string(REGEX REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Z7")
    endif()
    if(CMAKE_CXX_FLAGS_RELWITHDEBINFO MATCHES "/Zi")
        string(REGEX REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    else()
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")
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
        add_definitions(-DMATERIALX_BUILD_GEN_GLSL)
        add_subdirectory(source/MaterialXGenGlsl)
    endif()
    if (MATERIALX_BUILD_GEN_OSL)
        add_definitions(-DMATERIALX_BUILD_GEN_OSL)
        add_subdirectory(source/MaterialXGenOsl)
    endif()
    if (MATERIALX_BUILD_GEN_MDL)
        add_definitions(-DMATERIALX_BUILD_GEN_MDL)
        add_subdirectory(source/MaterialXGenMdl)
    endif()
    if (MATERIALX_BUILD_GEN_OGSXML)
        add_definitions(-DMATERIALX_BUILD_GEN_OGSXML)
        add_subdirectory(source/MaterialXGenOgsXml)
    endif()
    if (MATERIALX_BUILD_GEN_OGSFX)
        add_definitions(-DMATERIALX_BUILD_GEN_OGSFX)
        add_subdirectory(source/MaterialXGenOgsFx)
    endif()
    if (MATERIALX_BUILD_GEN_ARNOLD)
        add_definitions(-DMATERIALX_BUILD_GEN_ARNOLD)
        add_subdirectory(source/MaterialXGenArnold)
    endif()
endif()

if (MATERIALX_BUILD_CROSS)
    add_definitions(-DMATERIALX_BUILD_CROSS)
    add_subdirectory(source/MaterialXCross)
endif()

# Add render module subdirectories
if (MATERIALX_BUILD_RENDER)
    if(MATERIALX_BUILD_OIIO)
        add_definitions(-DMATERIALX_BUILD_OIIO)
    endif()
    add_subdirectory(source/MaterialXRender)
    add_subdirectory(source/MaterialXRenderOsl)
    add_subdirectory(source/MaterialXRenderHw)
    add_subdirectory(source/MaterialXRenderGlsl)
    add_subdirectory(resources)
    if (MATERIALX_BUILD_CONTRIB)
        add_definitions(-DMATERIALX_BUILD_CONTRIB)
        add_subdirectory(source/MaterialXContrib)
    endif()

    if(MATERIALX_TEST_RENDER)
        add_definitions(-DMATERIALX_TEST_RENDER)
    endif()

    # Add viewer subdirectory
    if(MATERIALX_BUILD_VIEWER)
        add_subdirectory(source/MaterialXView)
    endif()
endif()

# Add runtime subdirectory
if(MATERIALX_BUILD_RUNTIME)
    add_definitions(-DMATERIALX_BUILD_RUNTIME)
    add_subdirectory(source/MaterialXRuntime)
endif()

if(MATERIALX_BUILD_PYTHON)
    add_subdirectory(source/PyMaterialX)
    add_subdirectory(python)
endif()

if(MATERIALX_BUILD_JS)
    add_subdirectory(source/JsMaterialX)
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
        if (NOT MATERIALX_DISABLE_BUILD_RENDER)
            set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXTest)
        endif()
    endif()
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/MaterialXConfigVersion.cmake
    VERSION ${CMAKE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Package for using the developer build
export(
    EXPORT MaterialX
    FILE ${CMAKE_BINARY_DIR}/cmake/MaterialXConfig.cmake
    )

# Package for using the installed build
install(
    EXPORT      MaterialX
    DESTINATION cmake
    FILE        MaterialXConfig.cmake)
install(
    FILES "${CMAKE_BINARY_DIR}/cmake/MaterialXConfigVersion.cmake"
    DESTINATION cmake)
