file(GLOB pymaterialxgenogsfx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenogsfx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenOgsFx SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenogsfx_source} ${pymaterialxgenogsfx_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenOgsFx PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenOgsFx
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenOgsFx
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX "${MATERIALX_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenOgsFx
    PUBLIC PyMaterialXGenGlsl
           MaterialXGenOgsFx
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenOgsFx
        DESTINATION "${CMAKE_INSTALL_PREFIX}/python/MaterialX")
