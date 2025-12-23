include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR})

# Apply Python version and location requests from the user.
set(PYBIND11_PYTHON_VERSION ${MATERIALX_PYTHON_VERSION})
set(PYTHON_EXECUTABLE ${MATERIALX_PYTHON_EXECUTABLE})

# First look for a PyBind11 package via CMake.
if(MATERIALX_PYTHON_PYBIND11_DIR STREQUAL "")
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        include_directories(${PYBIND11_INCLUDE_DIR})
    else()
        set(MATERIALX_PYTHON_PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/PyBind11")
    endif()
endif()

# Then look for PyBind11 at its native or user-specified location.
if(NOT pybind11_FOUND)
    add_subdirectory(${MATERIALX_PYTHON_PYBIND11_DIR})
    include_directories(${PYBIND11_INCLUDE_DIR})
endif()

if(NOT MATERIALX_PYTHON_LTO)
    set(PYBIND11_MODULE_FLAGS "NO_EXTRAS")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template UNDEFINED_VAR_TEMPLATE_FLAG)
    if(UNDEFINED_VAR_TEMPLATE_FLAG)
        add_compile_options(-Wno-undefined-var-template)
    endif()
endif()

function(mx_add_python_module MODULE_NAME)
    set(options)
    set(oneValueArgs SOURCE_EXTENSION)
    set(multiValueArgs
        LIBRARIES
        EXCLUDE_FILES)
    cmake_parse_arguments(args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    # Default to .cpp source files if not specified
    if (NOT args_SOURCE_EXTENSION)
        set(args_SOURCE_EXTENSION "cpp")
    endif()

    # Gather source and header files
    file(GLOB module_source "${CMAKE_CURRENT_SOURCE_DIR}/*.${args_SOURCE_EXTENSION}")
    file(GLOB module_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    # Exclude specific files if requested
    if (args_EXCLUDE_FILES)
        foreach(exclude_file ${args_EXCLUDE_FILES})
            list(REMOVE_ITEM module_source "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
        endforeach()
    endif()

    # Create the pybind11 module
    pybind11_add_module(${MODULE_NAME} SHARED ${PYBIND11_MODULE_FLAGS} ${module_source} ${module_headers})

    # Apple-specific visibility setting
    if (APPLE)
        set_target_properties(${MODULE_NAME} PROPERTIES CXX_VISIBILITY_PRESET "default")
    endif()

    # Set common target properties
    set_target_properties(
        ${MODULE_NAME} PROPERTIES
        OUTPUT_NAME ${MODULE_NAME}
        COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
        LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
        INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
        DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

    # Link libraries
    target_link_libraries(${MODULE_NAME}
        PUBLIC ${args_LIBRARIES}
        PRIVATE ${CMAKE_DL_LIBS})

    # Install target
    install(TARGETS ${MODULE_NAME}
            DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
endfunction()

add_subdirectory(PyMaterialXCore)
add_subdirectory(PyMaterialXFormat)
if (MATERIALX_BUILD_GEN_GLSL OR MATERIALX_BUILD_GEN_OSL OR MATERIALX_BUILD_GEN_MDL OR MATERIALX_BUILD_GEN_MSL OR MATERIALX_BUILD_GEN_SLANG)
    add_subdirectory(PyMaterialXGenShader)
    if (MATERIALX_BUILD_GEN_GLSL)
        add_subdirectory(PyMaterialXGenGlsl)
    endif()
    if (MATERIALX_BUILD_GEN_MSL)
        add_subdirectory(PyMaterialXGenMsl)
    endif()
    if (MATERIALX_BUILD_GEN_OSL)
        add_subdirectory(PyMaterialXGenOsl)
    endif()
    if (MATERIALX_BUILD_GEN_MDL)
        add_subdirectory(PyMaterialXGenMdl)
    endif()
    if (MATERIALX_BUILD_GEN_SLANG)
        add_subdirectory(PyMaterialXGenSlang)
    endif()
endif()
if (MATERIALX_BUILD_RENDER)
    add_subdirectory(PyMaterialXRender)
    if (MATERIALX_BUILD_GEN_GLSL)
        add_subdirectory(PyMaterialXRenderGlsl)
    endif()
    if (MATERIALX_BUILD_GEN_OSL)
        add_subdirectory(PyMaterialXRenderOsl)
    endif()
    if (APPLE AND MATERIALX_BUILD_GEN_MSL)
        add_subdirectory(PyMaterialXRenderMsl)
    endif()
endif()
