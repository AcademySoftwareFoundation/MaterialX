include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB pymaterialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB pybind11_headers "${CMAKE_CURRENT_SOURCE_DIR}/PyBind11/*.h")

source_group("Header Files\\PyBind11" FILES ${pybind11_headers})

add_subdirectory(PyBind11)
pybind11_add_module(PyMaterialX SHARED ${pymaterialx_source} ${pymaterialx_headers} ${pybind11_headers})

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
