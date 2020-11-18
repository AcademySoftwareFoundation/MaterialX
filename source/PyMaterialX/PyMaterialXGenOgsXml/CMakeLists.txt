file(GLOB pymaterialxgenogsxml_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenogsxml_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenOgsXml SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenogsxml_source} ${pymaterialxgenogsxml_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenOgsXml PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenOgsXml
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenOgsXml
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenOgsXml
    PUBLIC PyMaterialXGenGlsl
           PyMaterialXRender
           MaterialXGenOgsXml
           MaterialXRender
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenOgsXml
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX")
