file(GLOB pymaterialxgenessl_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenessl_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenEssl SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenessl_source} ${pymaterialxgenessl_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenessl PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenEssl
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenEssl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenEssl
    PUBLIC PyMaterialXGenShader
           MaterialXGenEssl
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenEssl
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX")
