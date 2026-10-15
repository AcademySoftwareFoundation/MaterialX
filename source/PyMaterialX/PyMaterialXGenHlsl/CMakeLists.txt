file(GLOB pymaterialxgenhlsl_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenhlsl_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenHlsl SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenhlsl_source} ${pymaterialxgenhlsl_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenHlsl PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenHlsl
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenHlsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenHlsl
    PUBLIC PyMaterialXGenShader
           MaterialXGenHlsl
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenHlsl
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
