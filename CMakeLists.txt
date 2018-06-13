project(MaterialX)
set(MATERIALX_MAJOR_VERSION 1)
set(MATERIALX_MINOR_VERSION 36)
set(MATERIALX_BUILD_VERSION 0)
set(MATERIALX_LIBRARY_VERSION ${MATERIALX_MAJOR_VERSION}.${MATERIALX_MINOR_VERSION}.${MATERIALX_BUILD_VERSION})

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_MACOSX_RPATH ON)
include(CheckCXXCompilerFlag)
enable_testing()

option(MATERIALX_BUILD_PYTHON "Build the MaterialX Python package from C++ bindings. Requires Python 2.6 or greater." OFF)
option(MATERIALX_BUILD_DOCS "Create HTML documentation using Doxygen. Requires that Doxygen be installed." OFF)
option(MATERIALX_INSTALL_PYTHON "Install the MaterialX Python package as a third-party library when the install target is built." ON)
option(MATERIALX_WARNINGS_AS_ERRORS "Interpret all compiler warnings as errors." OFF)

set(MATERIALX_PYTHON_EXECUTABLE "" CACHE FILEPATH
    "Path to the Python executable (e.g. 'C:/Python27/python.exe').")
set(MATERIALX_PYTHON_INCLUDE_DIR "" CACHE PATH 
    "Path to the headers of the Python installation (e.g. 'C:/Python27/include').")
set(MATERIALX_PYTHON_LIBRARY "" CACHE FILEPATH
    "Path to the Python library file (e.g. 'C:/Python27/libs/python27.lib').")
set(MATERIALX_PYTHON_OCIO_DIR "" CACHE PATH
    "Path to a folder containing the default OCIO configuration to be packaged with MaterialX Python (e.g. 'D:/Projects/OpenColorIO-Configs/aces_1.0.3').")

mark_as_advanced(MATERIALX_BUILD_DOCS)
mark_as_advanced(MATERIALX_PYTHON_EXECUTABLE)
mark_as_advanced(MATERIALX_PYTHON_INCLUDE_DIR)
mark_as_advanced(MATERIALX_PYTHON_LIBRARY)
mark_as_advanced(MATERIALX_PYTHON_OCIO_DIR)
mark_as_advanced(MATERIALX_INSTALL_PYTHON)
mark_as_advanced(MATERIALX_WARNINGS_AS_ERRORS)

set(PYTHON_INCLUDE_DIR ${MATERIALX_PYTHON_INCLUDE_DIR})
set(PYTHON_LIBRARY ${MATERIALX_PYTHON_LIBRARY})

# Adjust the default installation path
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "Default install path" FORCE)
endif()

# Adjust compiler settings
if(MSVC)
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
    if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
        CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template UNDEFINED_VAR_TEMPLATE_FLAG)
        if(UNDEFINED_VAR_TEMPLATE_FLAG)
            add_compile_options(-Wno-undefined-var-template)
        endif()
    endif()
endif()

# Add subdirectories
add_subdirectory(source/MaterialXCore)
add_subdirectory(source/MaterialXFormat)
add_subdirectory(source/MaterialXTest)
add_subdirectory(documents)

if(MATERIALX_BUILD_PYTHON)
    add_subdirectory(source/PyMaterialX)
    add_subdirectory(python)
endif(MATERIALX_BUILD_PYTHON)
