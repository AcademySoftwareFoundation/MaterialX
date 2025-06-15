# MaterialX Version
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 39)
set(MATERIALX_BUILD_VERSION 4)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

# CMake setup
cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
enable_testing()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# JavaScript setup 
option(MATERIALX_BUILD_JS "Build the MaterialX JavaScript package from C++ bindings. Requires the emscripten environment." OFF)
set(MATERIALX_EMSDK_PATH "" CACHE PATH "Path to EMSDK (e.g. 'D:/Projects/emsdk').")
if (MATERIALX_BUILD_JS)
    if (EXISTS "${MATERIALX_EMSDK_PATH}")
        set(EMSDK_PATH ${MATERIALX_EMSDK_PATH})
    elseif (EXISTS $ENV{EMSDK})
        set(EMSDK_PATH $ENV{EMSDK})
    endif()
    if (EMSDK_PATH)
        set(CMAKE_TOOLCHAIN_FILE "${EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    else()
        message("The EMSDK path was not supplied, disabling MATERIALX_BUILD_JS")
        set(MATERIALX_BUILD_JS OFF)
    endif()
endif()

project(MaterialX VERSION ${MATERIALX_LIBRARY_VERSION})

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 3.9 or greater." OFF)
option(MATERIALX_BUILD_VIEWER "Build the MaterialX Viewer." OFF)
option(MATERIALX_BUILD_GRAPH_EDITOR "Build the MaterialX Graph Editor." OFF)
option(MATERIALX_BUILD_DOCS "Create HTML documentation using Doxygen. Requires that Doxygen be installed." OFF)

option(MATERIALX_BUILD_GEN_GLSL "Build the GLSL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_OSL "Build the OSL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_MDL "Build the MDL shader generator back-end." ON)
option(MATERIALX_BUILD_GEN_MSL "Build the MSL shader generator back-end." ON)
option(MATERIALX_BUILD_RENDER "Build the MaterialX Render modules." ON)
option(MATERIALX_BUILD_RENDER_PLATFORMS "Build platform-specific render modules for each shader generator." ON)
option(MATERIALX_BUILD_OIIO "Build OpenImageIO support for MaterialXRender." OFF)
option(MATERIALX_BUILD_OCIO "Build OpenColorIO support for shader generators." OFF)
option(MATERIALX_BUILD_TESTS "Build unit tests." OFF)
option(MATERIALX_BUILD_BENCHMARK_TESTS "Build benchmark tests." OFF)

option(MATERIALX_BUILD_SHARED_LIBS "Build MaterialX libraries as shared rather than static." OFF)
option(MATERIALX_BUILD_DATA_LIBRARY "Build generated products from the MaterialX data library." OFF)
option(MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS "Process the data library files at build time to expand any template elements." ON)
option(MATERIALX_BUILD_BAKE_NAMED_VALUES "Process the data library files at build time to bake out the named values." ON)
option(MATERIALX_BUILD_MONOLITHIC "Build a single monolithic MaterialX library." OFF)
option(MATERIALX_BUILD_USE_CCACHE "Enable the use of ccache to speed up build time, if present." ON)
option(MATERIALX_PYTHON_LTO "Enable link-time optimizations for MaterialX Python." ON)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)
option(MATERIALX_INSTALL_RESOURCES "Install the resources folder when building render modules." ON)
option(MATERIALX_TEST_RENDER "Run rendering tests for MaterialX Render module. GPU required for graphics validation." ON)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)
option(MATERIALX_COVERAGE_ANALYSIS "Build MaterialX libraries with coverage analysis on supporting platforms." OFF)
option(MATERIALX_DYNAMIC_ANALYSIS "Build MaterialX libraries with dynamic analysis on supporting platforms." OFF)

option(MATERIALX_BUILD_IOS "Build MaterialX for iOS. (Deprecated. Set CMAKE_SYSTEM_NAME to iOS instead.)" OFF)
option(MATERIALX_BUILD_APPLE_FRAMEWORK "Build MaterialX as an Apple Framework" ${__build_apple_framework})
if (MATERIALX_BUILD_IOS)
    message(DEPRECATION "The MATERIALX_BUILD_IOS option is deprecated. Set CMAKE_SYSTEM_NAME to iOS instead.")
    set(CMAKE_SYSTEM_NAME iOS)
endif()

list(APPEND CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/cmake/macros)
include(Public)

# Apple ecosystem cross-compilation
# https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-ios-tvos-visionos-or-watchos
set(MATERIALX_BUILD_APPLE_EMBEDDED OFF)
set(__build_apple_framework OFF)
if (CMAKE_SYSTEM_NAME MATCHES "iOS" OR CMAKE_SYSTEM_NAME MATCHES "tvOS" OR CMAKE_SYSTEM_NAME MATCHES "visionOS" OR CMAKE_SYSTEM_NAME MATCHES "watchOS")
    set(MATERIALX_BUILD_APPLE_EMBEDDED ON)
    set(__build_apple_framework ${MATERIALX_BUILD_SHARED_LIBS})
    # TARGET_OS_IPHONE refers to all IPHONE derived platforms
    # https://chaosinmotion.com/2021/08/02/things-to-remember-compiler-conditionals-for-macos-ios-etc/
    add_definitions(-DTARGET_OS_IPHONE=1)
    set(MATERIALX_BUILD_MONOLITHIC ON)
    set(MATERIALX_BUILD_PYTHON OFF)
    set(MATERIALX_BUILD_VIEWER OFF)
    set(MATERIALX_BUILD_GRAPH_EDITOR OFF)
    set(MATERIALX_BUILD_GEN_GLSL OFF)
    set(MATERIALX_BUILD_GEN_OSL OFF)
    set(MATERIALX_BUILD_GEN_MDL OFF)
    set(MATERIALX_BUILD_TESTS OFF)
endif()

# Apple framework handling
if(APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-" CACHE STRING "The Codesigning identity needed to sign compiled objects")
endif()
if (MATERIALX_BUILD_APPLE_FRAMEWORK)
    add_definitions(-DBUILD_APPLE_FRAMEWORK)
    set(MATERIALX_BUILD_MONOLITHIC ON)
    set(MATERIALX_BUILD_PYTHON OFF)
    set(MATERIALX_BUILD_VIEWER OFF)
    set(MATERIALX_BUILD_GRAPH_EDITOR OFF)
    set(MATERIALX_BUILD_TESTS OFF)
    set(MATERIALX_BUILD_SHARED_LIBS ON)
endif()

if (MATERIALX_BUILD_JS)
    set(MATERIALX_BUILD_RENDER OFF)
    set(MATERIALX_BUILD_TESTS OFF)
endif()

# All hardware shading languages currently depend on the GLSL shader generator.
if(MATERIALX_BUILD_GEN_MSL)
    set(MATERIALX_BUILD_GEN_GLSL ON)
endif()

set(MATERIALX_PYTHON_VERSION "" CACHE STRING
    "Python version to be used in building the MaterialX Python package (e.g. '3.9').")
set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH
    "Python executable to be used in building the MaterialX Python package (e.g. 'C:/Python39/python.exe').")
set(MATERIALX_PYTHON_PYBIND11_DIR "" CACHE PATH
    "Path to a folder containing the PyBind11 source to be used in building MaterialX Python.")

# Settings to define installation layout
set(MATERIALX_INSTALL_INCLUDE_PATH "include" CACHE STRING "Install header include path (e.g. 'inc', 'include').")
set(MATERIALX_INSTALL_LIB_PATH "lib" CACHE STRING "Install lib path (e.g. 'libs', 'lib').")
set(MATERIALX_INSTALL_STDLIB_PATH "libraries" CACHE STRING "Install path for mtlx std libs (e.g. 'libraries').")

# Helpers for OSL validation
set(MATERIALX_OSL_BINARY_OSLC "" CACHE FILEPATH "Full path to the OSL compiler binary.")
set(MATERIALX_OSL_BINARY_TESTRENDER "" CACHE FILEPATH "Full path to the OSL test render binary.")
set(MATERIALX_OSL_INCLUDE_PATH "" CACHE PATH "Full path to OSL shader includes (e.g. 'stdosl.h').")

set(MATERIALX_PYTHON_FOLDER_NAME "python/MaterialX" CACHE INTERNAL "Folder name to user for installing the Python library.")

if(SKBUILD)
    set(MATERIALX_PYTHON_FOLDER_NAME "MaterialX")
endif()

# Helpers for MDL validation
if (MATERIALX_BUILD_GEN_MDL)
    set(MATERIALX_MDLC_EXECUTABLE "" CACHE FILEPATH "Full path to the mdlc binary.")
    set(MATERIALX_MDL_RENDER_EXECUTABLE "" CACHE FILEPATH "Full path to the mdl renderer binary.")
    set(MATERIALX_MDL_RENDER_ARGUMENTS "" CACHE STRING "Custom arguments for renderer.")
    set(MATERIALX_MDL_MODULE_PATHS "" CACHE FILEPATH "Comma separated list of MDL module paths.")
    set(MATERIALX_INSTALL_MDL_MODULE_PATH ${MATERIALX_INSTALL_STDLIB_PATH} CACHE FILEPATH "Install path for mdl module.")
endif()

# Namespace
set(MATERIALX_NAMESPACE_SUFFIX "" CACHE STRING "Add a suffix to the main MaterialX C++ namespace: Options include dev, staging, <YOURFACILITY> etc.")
if(MATERIALX_NAMESPACE_SUFFIX STREQUAL "")
    set(MATERIALX_NAMESPACE "MaterialX_v${MATERIALX_MAJOR_VERSION}_${MATERIALX_MINOR_VERSION}_${MATERIALX_BUILD_VERSION}")
else()
    set(MATERIALX_NAMESPACE "MaterialX_${MATERIALX_NAMESPACE_SUFFIX}_v${MATERIALX_MAJOR_VERSION}_${MATERIALX_MINOR_VERSION}_${MATERIALX_BUILD_VERSION}")
endif()
message(STATUS "Setting namespace to '${MATERIALX_NAMESPACE}'")

# Library name custom suffix
# This helps an application that needs to ship a dynamic library MaterialX ensure
# that it has a unique name that won't conflict with one elsewhere on the system.
set(MATERIALX_LIBNAME_SUFFIX "" CACHE STRING "Specify a suffix to all libraries that are built")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_BUILD_GEN_GLSL)
mark_as_advanced(MATERIALX_BUILD_GEN_OSL)
mark_as_advanced(MATERIALX_BUILD_GEN_MDL)
mark_as_advanced(MATERIALX_BUILD_GEN_MSL)
mark_as_advanced(MATERIALX_BUILD_RENDER)
mark_as_advanced(MATERIALX_BUILD_RENDER_PLATFORMS)
mark_as_advanced(MATERIALX_BUILD_OIIO)
mark_as_advanced(MATERIALX_BUILD_OCIO)
mark_as_advanced(MATERIALX_BUILD_BENCHMARK_TESTS)
mark_as_advanced(MATERIALX_BUILD_SHARED_LIBS)
mark_as_advanced(MATERIALX_BUILD_DATA_LIBRARY)
mark_as_advanced(MATERIALX_BUILD_MONOLITHIC)
mark_as_advanced(MATERIALX_BUILD_USE_CCACHE)
mark_as_advanced(MATERIALX_NAMESPACE_SUFFIX)
mark_as_advanced(MATERIALX_LIBNAME_SUFFIX)
mark_as_advanced(MATERIALX_PYTHON_LTO)
mark_as_advanced(MATERIALX_INSTALL_PYTHON)
mark_as_advanced(MATERIALX_INSTALL_RESOURCES)
mark_as_advanced(MATERIALX_TEST_RENDER)
mark_as_advanced(MATERIALX_WARNINGS_AS_ERRORS)
mark_as_advanced(MATERIALX_COVERAGE_ANALYSIS)
mark_as_advanced(MATERIALX_DYNAMIC_ANALYSIS)
mark_as_advanced(MATERIALX_PYTHON_VERSION)
mark_as_advanced(MATERIALX_PYTHON_EXECUTABLE)
mark_as_advanced(MATERIALX_PYTHON_PYBIND11_DIR)
mark_as_advanced(MATERIALX_OSL_BINARY_OSLC)
mark_as_advanced(MATERIALX_OSL_BINARY_TESTRENDER)
mark_as_advanced(MATERIALX_OSL_INCLUDE_PATH)
mark_as_advanced(MATERIALX_INSTALL_INCLUDE_PATH)
mark_as_advanced(MATERIALX_INSTALL_LIB_PATH)
mark_as_advanced(MATERIALX_INSTALL_STDLIB_PATH)
mark_as_advanced(MATERIALX_BUILD_JS)
mark_as_advanced(MATERIALX_EMSDK_PATH)
mark_as_advanced(MATERIALX_BUILD_IOS)
mark_as_advanced(MATERIALX_BUILD_APPLE_FRAMEWORK)
if (MATERIALX_BUILD_GEN_MDL)
    mark_as_advanced(MATERIALX_MDLC_EXECUTABLE)
    mark_as_advanced(MATERIALX_MDL_RENDER_EXECUTABLE)
    mark_as_advanced(MATERIALX_MDL_RENDER_ARGUMENTS)
    mark_as_advanced(MATERIALX_MDL_MODULE_PATHS)
    mark_as_advanced(MATERIALX_INSTALL_MDL_MODULE_PATH)
endif()

if (MATERIALX_BUILD_USE_CCACHE)
    # Setup CCache for C/C++ compilation
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()

# Allow the OSL CMake package to provide binary locations for render tests.
# This will not override explicitly provided oslc, testrender, and include paths.
if(MATERIALX_BUILD_RENDER AND MATERIALX_BUILD_GEN_OSL AND MATERIALX_BUILD_TESTS)
    find_package(OSL QUIET)
    if(OSL_FOUND)
        if(NOT MATERIALX_OSL_BINARY_OSLC)
            set(MATERIALX_OSL_BINARY_OSLC $<TARGET_FILE:OSL::oslc>)
        endif()
        if(NOT MATERIALX_OSL_BINARY_TESTRENDER)
            # OSL does not yet export a CMake target for testrender.
            set(MATERIALX_OSL_BINARY_TESTRENDER $<TARGET_FILE_DIR:OSL::oslc>/testrender)
        endif()
    endif()
endif()

# Add global definitions
if(MATERIALX_BUILD_OIIO)
    add_definitions(-DMATERIALX_BUILD_OIIO)
endif()
if(MATERIALX_TEST_RENDER)
    add_definitions(-DMATERIALX_TEST_RENDER)
endif()
if (MATERIALX_BUILD_BENCHMARK_TESTS)
    add_definitions(-DMATERIALX_BUILD_BENCHMARK_TESTS)
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
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/installed" CACHE PATH "Default install path" FORCE)
endif()

# Set the installation path for shared libraries
if(APPLE)
    set(RPATH_RELATIVE_SYMBOL "@loader_path")
else()
    set(RPATH_RELATIVE_SYMBOL "$ORIGIN")
endif()

# The following include relative RPATHS, allowing shared libraries to be relocated, as well as
# absolute RPATHS for backwards compatibility.

# For linking to libraries in the same dir - ie, things in "MATX/lib" to each other
set(MATERIALX_SAME_DIR_RPATH "${RPATH_RELATIVE_SYMBOL};${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_LIB_PATH}")
# For linking to libraries where source is one directory deep, ie: "MATX/bin/../lib"
set(MATERIALX_UP_ONE_RPATH "${RPATH_RELATIVE_SYMBOL}/../${MATERIALX_INSTALL_LIB_PATH};${MATERIALX_SAME_DIR_RPATH}")
# For linking to libraries where source is two directories deep, ie: "MATX/python/MaterialX/../../lib"
set(MATERIALX_UP_TWO_RPATH "${RPATH_RELATIVE_SYMBOL}/../../${MATERIALX_INSTALL_LIB_PATH};${MATERIALX_SAME_DIR_RPATH}")
if(SKBUILD)
    # When building the Python wheels, we don't want to set any RPATH because
    # we want to wheel to be self-contained. We don't want any interference from
    # external paths.
    set(MATERIALX_UP_TWO_RPATH "${RPATH_RELATIVE_SYMBOL}")
endif()

# Adjust compiler settings
if(MSVC)
    add_compile_options(/MP)
    if(MATERIALX_BUILD_MONOLITHIC)
        add_compile_options(/bigobj)
    endif()
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
    if(MATERIALX_COVERAGE_ANALYSIS)
        add_compile_options(--coverage -O0)
        add_link_options(--coverage)
    endif()
    if(MATERIALX_DYNAMIC_ANALYSIS)
        set(DYNAMIC_ANALYSIS_OPTIONS -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all)
        add_compile_options(${DYNAMIC_ANALYSIS_OPTIONS})
        add_link_options(${DYNAMIC_ANALYSIS_OPTIONS})
    endif()
    if(MATERIALX_BUILD_JS)
        add_compile_options(-fexceptions)
    endif()
endif()

# Propagate shared library setting to NanoGUI
if(MATERIALX_BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "ON")
else()
    set(BUILD_SHARED_LIBS "OFF")
endif()

# If we're baking the named "Value:" entries - then we have to also expand any
# template elements
if (MATERIALX_BUILD_BAKE_NAMED_VALUES)
    set(MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS ON)
endif()

if (MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS OR MATERIALX_BUILD_BAKE_NAMED_VALUES)
    set(MATERIALX_BUILD_DATA_LIBRARY ON)
endif()

# Build a monolithic target - needs to be added before the other build targets that may be included.
if (MATERIALX_BUILD_MONOLITHIC)
    set(MATERIALX_MONOLITHIC_TARGET MaterialX)
    add_subdirectory(source)
endif()

message("XXX : MATERIALX_BUILD_BAKE_NAMED_VALUES     : ${MATERIALX_BUILD_BAKE_NAMED_VALUES}")
message("XXX : MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS : ${MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS}")
message("XXX : MATERIALX_BUILD_DATA_LIBRARY          : ${MATERIALX_BUILD_DATA_LIBRARY}")

# Add core subdirectories
add_subdirectory(source/MaterialXCore)
add_subdirectory(source/MaterialXFormat)
if (MATERIALX_BUILD_EXPAND_TEMPLATE_ELEMS OR MATERIALX_BUILD_BAKE_NAMED_VALUES)
    if (CMAKE_CROSSCOMPILING)
        set(_MaterialXBuildLibrary "${CMAKE_BINARY_DIR}/BUILD_TOOLS/MaterialXBuildLibrary")

        add_custom_command(
                OUTPUT ${_MaterialXBuildLibrary}
                COMMAND ${CMAKE_COMMAND}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/BUILD_TOOLS
                -DMATERIALX_MAJOR_VERSION=${MATERIALX_MAJOR_VERSION}
                -DMATERIALX_MINOR_VERSION=${MATERIALX_MINOR_VERSION}
                -DMATERIALX_BUILD_VERSION=${MATERIALX_BUILD_VERSION}
                -DMATERIALX_NAMESPACE=${MATERIALX_NAMESPACE}
                -DMATERIALX_BUILD_BAKE_NAMED_VALUES=ON
                -B"BUILD_TOOLS"
                -H"${CMAKE_SOURCE_DIR}/source/MaterialXBuildTools"
                COMMAND ${CMAKE_COMMAND} --build BUILD_TOOLS
                DEPENDS MaterialXCore MaterialXFormat
        )
        add_custom_target(MaterialXBuildLibrary ALL
        DEPENDS ${_MaterialXBuildLibrary})
    else()
        set(_MaterialXBuildLibrary MaterialXBuildLibrary)
        add_subdirectory(source/MaterialXBuildTools/buildLibrary)
        add_subdirectory(source/MaterialXBuildTools/buildDocs)
        add_subdirectory(dev_spec)
    endif()
endif()

# Add shader generation subdirectories
add_subdirectory(source/MaterialXGenShader)
if(MATERIALX_BUILD_GEN_GLSL OR MATERIALX_BUILD_GEN_OSL OR MATERIALX_BUILD_GEN_MDL OR MATERIALX_BUILD_GEN_MSL)
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
    if (MATERIALX_BUILD_GEN_MSL)
        add_definitions(-DMATERIALX_BUILD_GEN_MSL)
        add_subdirectory(source/MaterialXGenMsl)
    endif()
    add_subdirectory(libraries)
endif()

# Add rendering and viewer subdirectories
if(MATERIALX_BUILD_RENDER)
    add_subdirectory(source/MaterialXRender)
    if(MATERIALX_BUILD_RENDER_PLATFORMS)
        set(MATERIALX_BUILD_RENDER_HW OFF)
        if(MATERIALX_BUILD_GEN_GLSL AND NOT MATERIALX_BUILD_APPLE_EMBEDDED)
            set(MATERIALX_BUILD_RENDER_HW ON)
            add_subdirectory(source/MaterialXRenderGlsl)
        endif()
        if(MATERIALX_BUILD_GEN_MSL AND APPLE)
            set(MATERIALX_BUILD_RENDER_HW ON)
            add_subdirectory(source/MaterialXRenderMsl)
        endif()
        if(MATERIALX_BUILD_RENDER_HW)
            add_subdirectory(source/MaterialXRenderHw)
        endif()
        if(MATERIALX_BUILD_GEN_OSL)
            add_subdirectory(source/MaterialXRenderOsl)
        endif()
    endif()
    if(MATERIALX_BUILD_VIEWER)
        add_subdirectory(source/MaterialXView)
    endif()
    if(MATERIALX_BUILD_GRAPH_EDITOR)
        add_subdirectory(source/MaterialXGraphEditor)
    endif()
    if(MATERIALX_INSTALL_RESOURCES AND NOT SKBUILD)
        add_subdirectory(resources)
    endif()
endif()

# Add test subdirectory
if(MATERIALX_BUILD_TESTS)
    add_subdirectory(source/MaterialXTest)
endif()

# Add Python subdirectories
if(MATERIALX_BUILD_PYTHON)
    add_subdirectory(source/PyMaterialX)
    add_subdirectory(python)
endif()

if(MATERIALX_BUILD_DOCS)
    add_subdirectory(documents)
endif()

if(MATERIALX_BUILD_JS)
    add_subdirectory(source/JsMaterialX)
endif()

if(MATERIALX_BUILD_MONOLITHIC)
    # MaterialX monolithic build target needs to be installed after any other included
    # modules to ensure the correct files are in mxHeaders
    if(NOT SKBUILD)
        install(TARGETS ${MATERIALX_MONOLITHIC_TARGET}
                EXPORT MaterialX
                ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                RUNTIME DESTINATION bin
                FILE_SET mxHeaders DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH})

        # Note : we don't install the headers etc. here, and rely on each separate modules CMakeLists.txt
        # to do that installation, thus we respect the build options configuration, and only install
        # the headers for the modules we've built in to the monolithic build.

        # Finally do the framework build if requested
        # This uses a zsh script since zsh is guaranteed to exist on systems
        if(MATERIALX_BUILD_APPLE_FRAMEWORK)
            # Conform cmake formats to zsh expected formats
            set(__embedded_build "false")
            if (MATERIALX_BUILD_APPLE_EMBEDDED)
                set(__embedded_build "true")
            endif()

            # Install the Info.plist and shell script
            math(EXPR CFBUNDLEVERSION "${MATERIALX_MAJOR_VERSION} * 10000 + ${MATERIALX_MINOR_VERSION} * 100 + ${MATERIALX_BUILD_VERSION}")
            configure_file(cmake/modules/Info.plist.in "${PROJECT_BINARY_DIR}/Info.plist" @ONLY)
            configure_file(cmake/modules/AppleFrameworkBuild.zsh.in "${PROJECT_BINARY_DIR}/AppleFrameworkBuild.zsh" @ONLY)

            # Run the shell script for the primary configuration
            install(CODE "execute_process(COMMAND zsh ${PROJECT_BINARY_DIR}/AppleFrameworkBuild.zsh )")
        endif()
    endif()
endif()

# Set Visual Studio startup projects
if(MATERIALX_BUILD_VIEWER)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXView)
elseif(MATERIALX_BUILD_GRAPH_EDITOR)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXGraphEditor)
elseif(MATERIALX_BUILD_TESTS)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXTest)
endif()

# Install root-level documents
if(NOT SKBUILD)
    install(FILES LICENSE CHANGELOG.md README.md THIRD-PARTY.md DESTINATION .)

    set(MATERIALX_GEN_CONFIG_PATH "${MATERIALX_INSTALL_LIB_PATH}/cmake/${CMAKE_PROJECT_NAME}")

    include(CMakePackageConfigHelpers)

    if (MATERIALX_BUILD_MONOLITHIC)
        # Export aliases for the MaterialX modules built in this monolithic build
        # to be less disruptive to downstream projects.
        get_property(MATERIALX_MODULES GLOBAL PROPERTY MATERIALX_MODULES)
        set(EXPORT_ALIASES "# Aliased targets for the the monolithic build\n")
        foreach (MODULE ${MATERIALX_MODULES})
            string(APPEND EXPORT_ALIASES "add_library(${MODULE} ALIAS MaterialX)\n")
        endforeach ()

        if (NOT MATERIALX_MODULES)
            message(FATAL_ERROR "Building MaterialX as a monolithic library, but did not find any libraries to alias.")
        endif()
    endif()

    configure_package_config_file(cmake/modules/MaterialXConfig.cmake.in
                                  ${PROJECT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake
                                  INSTALL_DESTINATION "${MATERIALX_GEN_CONFIG_PATH}"
                                  PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_PROJECT_NAME)
    write_basic_package_version_file(${PROJECT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
                                     VERSION ${MATERIALX_LIBRARY_VERSION}
                                     COMPATIBILITY AnyNewerVersion)

    # Install auto-generated CMake configuration files
    install(EXPORT MaterialX
            DESTINATION "${MATERIALX_GEN_CONFIG_PATH}"
            FILE ${CMAKE_PROJECT_NAME}Targets.cmake)
    install(FILES "${PROJECT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
            "${PROJECT_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake"
            DESTINATION "${MATERIALX_GEN_CONFIG_PATH}")
endif()
