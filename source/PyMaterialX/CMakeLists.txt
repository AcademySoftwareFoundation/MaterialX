set(PYBIND11_INCLUDE_DIR "${MATERIALX_PYTHON_PYBIND11_DIR}/include")
set(PYBIND11_COMMON_H "${PYBIND11_INCLUDE_DIR}/pybind11/detail/common.h")
set(PYBIND11_PYTHON_VERSION ${MATERIALX_PYTHON_VERSION})
set(PYTHON_EXECUTABLE ${MATERIALX_PYTHON_EXECUTABLE})

if(NOT EXISTS "${PYBIND11_COMMON_H}")
message(FATAL_ERROR "PyBind11 source file not found: ${PYBIND11_COMMON_H}")
endif()

file(STRINGS "${PYBIND11_COMMON_H}" pybind11_version_defines
     REGEX "#define PYBIND11_VERSION_(MAJOR|MINOR|PATCH) ")
foreach(ver ${pybind11_version_defines})
  if (ver MATCHES "#define PYBIND11_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
    set(PYBIND11_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
  endif()
endforeach()
set(PYBIND11_VERSION ${PYBIND11_VERSION_MAJOR}.${PYBIND11_VERSION_MINOR}.${PYBIND11_VERSION_PATCH})
message(STATUS "Using PyBind11 v${PYBIND11_VERSION}")

include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PYBIND11_INCLUDE_DIR})

list(APPEND CMAKE_MODULE_PATH "${MATERIALX_PYTHON_PYBIND11_DIR}/tools")
include(pybind11Tools)

if(NOT MATERIALX_PYTHON_LTO)
    set(PYBIND11_MODULE_FLAGS "NO_EXTRAS")
endif()

add_subdirectory(PyMaterialXCore)
add_subdirectory(PyMaterialXFormat)
add_subdirectory(PyMaterialXGenShader)
add_subdirectory(PyMaterialXGenGlsl)
add_subdirectory(PyMaterialXGenOsl)

if (MATERIALX_BUILD_GEN_OGSFX)
    add_subdirectory(PyMaterialXGenOgsFx)
endif()
if (MATERIALX_BUILD_GEN_ARNOLD)
    add_subdirectory(PyMaterialXGenArnold)
endif()
