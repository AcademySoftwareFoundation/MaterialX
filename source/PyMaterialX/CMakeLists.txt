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

if (MATERIALX_BUILD_DOCS)
    # Ensure Doxygen docs are generated, then extract docs for pybind11 bindings.
    set(PYBIND_DOCS_ARGS -d ${CMAKE_BINARY_DIR}/documents/doxygen_xml -p ${CMAKE_SOURCE_DIR}/source/PyMaterialX)
    if(MATERIALX_PYTHON_FORCE_REPLACE_DOCS)
        list(APPEND PYBIND_DOCS_ARGS --force)
    endif()
    add_custom_target(PyBindDocs
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/Scripts/pybind_docs.py ${PYBIND_DOCS_ARGS}
        COMMENT "Generating PyMaterialX binding docs from Doxygen XML"
        VERBATIM
    )

    # Run MaterialXDocs before attempting to generate pybind docs
    add_dependencies(PyBindDocs MaterialXDocs)

    # Make pybind modules depend on the generated docs so the binding docs
    # are integrated prior to building the Python extension modules.
    foreach(_py_mod IN ITEMS PyMaterialXCore PyMaterialXFormat PyMaterialXGenShader
                              PyMaterialXGenGlsl PyMaterialXGenMsl PyMaterialXGenMdl
                              PyMaterialXGenOsl PyMaterialXRender PyMaterialXRenderGlsl
                              PyMaterialXRenderOsl PyMaterialXRenderMsl)
        if(TARGET ${_py_mod})
            add_dependencies(${_py_mod} PyBindDocs)
        endif()
    endforeach()
endif()
