file(GLOB pymaterialxrenderhlsl_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxrenderhlsl_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXRenderHlsl SHARED ${PYBIND11_MODULE_FLAGS}
    ${pymaterialxrenderhlsl_source} ${pymaterialxrenderhlsl_headers})

set_target_properties(
    PyMaterialXRenderHlsl
    PROPERTIES
    OUTPUT_NAME PyMaterialXRenderHlsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXRenderHlsl
    PUBLIC PyMaterialXRender
           MaterialXRenderHlsl
           PyMaterialXGenHlsl
           MaterialXGenHlsl
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXRenderHlsl
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
