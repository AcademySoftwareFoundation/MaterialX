set(PYBIND11_INCLUDE_DIR "${MATERIALX_PYTHON_PYBIND11_DIR}/include")
set(PYBIND11_TOOLS_DIR "${MATERIALX_PYTHON_PYBIND11_DIR}/tools")

file(STRINGS "${PYBIND11_INCLUDE_DIR}/pybind11/detail/common.h" pybind11_version_defines
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
    ${PYBIND11_INCLUDE_DIR}
)

file(GLOB pymaterialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

list(APPEND CMAKE_MODULE_PATH "${PYBIND11_TOOLS_DIR}")
include(pybind11Tools)
pybind11_add_module(PyMaterialX SHARED ${pymaterialx_source} ${pymaterialx_headers})

set_target_properties(
    PyMaterialX
    PROPERTIES
    OUTPUT_NAME PyMaterialX
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    PyMaterialX
    PUBLIC MaterialXFormat
    PRIVATE ${CMAKE_DL_LIBS}
)

install(TARGETS PyMaterialX
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX"
)
