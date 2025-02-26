# MaterialX Version
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 39)
set(MATERIALX_BUILD_VERSION 3)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

# Cmake setup
cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
enable_testing()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 3.6 or greater." OFF)
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
option(MATERIALX_BUILD_TESTS "Build unit tests." OFF)
option(MATERIALX_BUILD_BENCHMARK_TESTS "Build benchmark tests." OFF)

option(MATERIALX_BUILD_SHARED_LIBS "Build MaterialX libraries as shared rather than static." OFF)
option(MATERIALX_BUILD_DATA_LIBRARY "Build generated products from the MaterialX data library." OFF)
option(MATERIALX_BUILD_MONOLITHIC "Build a single monolithic MaterialX library." OFF)
option(MATERIALX_BUILD_USE_CCACHE "Enable the use of ccache to speed up build time, if present." ON)
option(MATERIALX_PYTHON_LTO "Enable link-time optimizations for MaterialX Python." ON)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)
option(MATERIALX_INSTALL_RESOURCES "Install the resources folder when building render modules." ON)
option(MATERIALX_TEST_RENDER "Run rendering tests for MaterialX Render module. GPU required for graphics validation." ON)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)
option(MATERIALX_COVERAGE_ANALYSIS "Build MaterialX libraries with coverage analysis on supporting platforms." OFF)
option(MATERIALX_DYNAMIC_ANALYSIS "Build MaterialX libraries with dynamic analysis on supporting platforms." OFF)

option(MATERIALX_BUILD_IOS "Build MaterialX for iOS. (Deprecated. Set CMAKE_SYSTEM_NAME instead)" OFF)
option(MATERIALX_BUILD_APPLE_FRAMEWORK "Build MaterialX as an Apple Framework" ${__build_apple_framework})
if (MATERIALX_BUILD_IOS)
    MESSAGE(WARNING "The MATERIALX_BUILD_IOS is deprecated. Set the CMAKE_SYSTEM_NAME to the platform instead")
    set(CMAKE_SYSTEM_NAME iOS)
endif()

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

if(MATERIALX_BUILD_GEN_MSL)
    # All hardware shading languages currently depend on the GLSL shader generator.
    set(MATERIALX_BUILD_GEN_GLSL ON)
endif()

set(MATERIALX_PYTHON_VERSION "" CACHE STRING
    "Python version to be used in building the MaterialX Python package (e.g. '3.9').")
set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH
    "Python executable to be used in building the MaterialX Python package (e.g. 'C:/Python39/python.exe').")
set(MATERIALX_PYTHON_OCIO_DIR "" CACHE PATH
    "Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python (e.g. 'D:/Projects/OpenColorIO-Configs/aces_1.0.3').")
set(MATERIALX_PYTHON_PYBIND11_DIR "" CACHE PATH
    "Path to a folder containing the PyBind11 source to be used in building MaterialX Python.")

set(MATERIALX_OIIO_DIR "" CACHE PATH "Path to the root folder of the OpenImageIO installation.")

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
set (MATERIALX_LIBNAME_SUFFIX "" CACHE STRING "Specify a suffix to all libraries that are built")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_BUILD_GEN_GLSL)
mark_as_advanced(MATERIALX_BUILD_GEN_OSL)
mark_as_advanced(MATERIALX_BUILD_GEN_MDL)
mark_as_advanced(MATERIALX_BUILD_GEN_MSL)
mark_as_advanced(MATERIALX_BUILD_RENDER)
mark_as_advanced(MATERIALX_BUILD_RENDER_PLATFORMS)
mark_as_advanced(MATERIALX_BUILD_OIIO)
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
mark_as_advanced(MATERIALX_PYTHON_OCIO_DIR)
mark_as_advanced(MATERIALX_PYTHON_PYBIND11_DIR)
mark_as_advanced(MATERIALX_OIIO_DIR)
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

# Attempt to configure OSL testing if it can be found by cmake.
# This will not override any explicitly provided oslc and testrender
if(MATERIALX_BUILD_RENDER AND MATERIALX_BUILD_GEN_OSL AND MATERIALX_BUILD_TESTS)
    # We currently only need the actual OSL binaries if we're running tests with Render and GenOSL enabled.
    find_package(OSL QUIET)
    if(OSL_FOUND)
        if(NOT MATERIALX_OSL_BINARY_OSLC)
            set(MATERIALX_OSL_BINARY_OSLC $<TARGET_FILE:OSL::oslc>)
        endif()
        if(NOT MATERIALX_OSL_BINARY_TESTRENDER)
            # currently OSL does not export a cmake target for testrender, but once that's added this can be simplified.
            set(MATERIALX_OSL_BINARY_TESTRENDER $<TARGET_FILE_DIR:OSL::oslc>/testrender)
        endif()
        # NOTE : we do not derive a value for MATERIALX_OSL_INCLUDE_PATH here, as a a cmake installed OSL package
        # should have the shader includes in the expected location.
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
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "Default install path" FORCE)
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

function(mx_add_library MATERIALX_MODULE_NAME)
    set(options ADD_OBJECTIVE_C_CODE)
    set(oneValueArgs EXPORT_DEFINE)
    set(multiValueArgs
            SOURCE_FILES
            HEADER_FILES
            INLINED_FILES
            MTLX_MODULES)
    cmake_parse_arguments(args
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    if (APPLE AND args_ADD_OBJECTIVE_C_CODE)
        file(GLOB_RECURSE materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m*")
        set_source_files_properties(${materialx_source_oc} PROPERTIES
                COMPILE_FLAGS "-x objective-c++")
        set(args_SOURCE_FILES ${args_SOURCE_FILES} ${materialx_source_oc})
    endif()

    assign_source_group("Source Files" ${args_SOURCE_FILES})
    assign_source_group("Source Files" ${args_INLINED_FILES})
    assign_source_group("Header Files" ${args_HEADER_FILES})

    if (NOT MATERIALX_BUILD_MONOLITHIC)
        set(TARGET_NAME ${MATERIALX_MODULE_NAME})
        add_library(${TARGET_NAME})

        # Create version resource
        if(MATERIALX_BUILD_SHARED_LIBS AND MSVC)
            configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/MaterialXVersion.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
            target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        endif()

        target_link_libraries(${TARGET_NAME}
            PUBLIC
                ${args_MTLX_MODULES}
                ${CMAKE_DL_LIBS})

        target_include_directories(${TARGET_NAME}
                PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
                $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
                PRIVATE
                ${EXTERNAL_INCLUDE_DIRS})

        set_target_properties(
                ${TARGET_NAME} PROPERTIES
                OUTPUT_NAME ${MATERIALX_MODULE_NAME}${MATERIALX_LIBNAME_SUFFIX}
                COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
                LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
                INSTALL_RPATH "${MATERIALX_SAME_DIR_RPATH}"
                VERSION "${MATERIALX_LIBRARY_VERSION}"
                SOVERSION "${MATERIALX_MAJOR_VERSION}")
    else()
        set(TARGET_NAME ${MATERIALX_MONOLITHIC_TARGET})

        add_library(${MATERIALX_MODULE_NAME} ALIAS ${MATERIALX_MONOLITHIC_TARGET})

        # Store the aliased MaterialX modules name to create cmake export aliases later.
        set_property(GLOBAL APPEND PROPERTY MATERIALX_MODULES ${MATERIALX_MODULE_NAME})
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    target_sources(${TARGET_NAME}
        PRIVATE
            ${args_SOURCE_FILES}
        PUBLIC
            FILE_SET
                mxHeaders
            TYPE
                HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/..
                ${CMAKE_CURRENT_BINARY_DIR}/..
            FILES
                ${args_HEADER_FILES}
                ${args_INLINED_FILES})

    target_include_directories(${TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

    target_compile_definitions(${TARGET_NAME} PRIVATE "-D${args_EXPORT_DEFINE}")

    if(NOT SKBUILD)
        if(NOT MATERIALX_BUILD_MONOLITHIC)
            install(TARGETS ${MATERIALX_MODULE_NAME}
                    EXPORT MaterialX
                    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                    RUNTIME DESTINATION bin
                    FILE_SET mxHeaders DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH})
        endif()

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${MATERIALX_MODULE_NAME}.pdb"
                DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
    endif()

    # pass TARGET_NAME back to call site - so caller can modify the build target
    set(TARGET_NAME ${TARGET_NAME} PARENT_SCOPE)
endfunction()

# Propagate shared library setting to NanoGUI:
if(MATERIALX_BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "ON")
else()
    set(BUILD_SHARED_LIBS "OFF")
endif()

# Build a monolithic target - needs to be added before the other build targets that may be included.
if (MATERIALX_BUILD_MONOLITHIC)
    set(MATERIALX_MONOLITHIC_TARGET MaterialX)
    add_subdirectory(source)
endif()

# Add core subdirectories
add_subdirectory(source/MaterialXCore)
add_subdirectory(source/MaterialXFormat)

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

if (MATERIALX_BUILD_MONOLITHIC)
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

if(MATERIALX_BUILD_VIEWER)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXView)
elseif(MATERIALX_BUILD_GRAPH_EDITOR)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXGraphEditor)
elseif(MATERIALX_BUILD_TESTS)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MaterialXTest)
endif()

# Install root-level documents
if(NOT SKBUILD)
    install(FILES LICENSE CHANGELOG.md README.md THIRD-PARTY.md
            DESTINATION .)

    set(MATERIALX_GEN_CONFIG_PATH "${MATERIALX_INSTALL_LIB_PATH}/cmake/${CMAKE_PROJECT_NAME}")

    include(CMakePackageConfigHelpers)

    if (MATERIALX_BUILD_MONOLITHIC)
        # export aliases for the MaterialX modules built in this monolithic build to be
        # less disruptive to downstream probjects.
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
                                ${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake
                                INSTALL_DESTINATION "${MATERIALX_GEN_CONFIG_PATH}"
                                PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_PROJECT_NAME)
    write_basic_package_version_file(${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
                                    VERSION ${MATERIALX_LIBRARY_VERSION}
                                    COMPATIBILITY AnyNewerVersion)

    # Install the auto-generated CMake configuration files:

    install(EXPORT MaterialX
            DESTINATION "${MATERIALX_GEN_CONFIG_PATH}"
            FILE ${CMAKE_PROJECT_NAME}Targets.cmake)

    install(FILES "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
            "${CMAKE_BINARY_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake"
            DESTINATION "${MATERIALX_GEN_CONFIG_PATH}")
endif()
