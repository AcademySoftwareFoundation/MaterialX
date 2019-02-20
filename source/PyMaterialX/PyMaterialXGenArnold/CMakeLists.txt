file(GLOB pymaterialxgenarnold_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenarnold_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenArnold SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenarnold_source} ${pymaterialxgenarnold_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenArnold PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenArnold
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenArnold
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    PyMaterialXGenArnold
    PUBLIC MaterialXFormat
           MaterialXGenArnold
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenArnold
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX")
