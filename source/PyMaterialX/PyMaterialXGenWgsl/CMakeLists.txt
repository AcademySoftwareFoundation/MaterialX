file(GLOB pymaterialxgenwgsl_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenwgsl_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenWgsl SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenwgsl_source} ${pymaterialxgenwgsl_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenWgsl PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenWgsl
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenWgsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenWgsl
    PUBLIC PyMaterialXGenShader
           MaterialXGenWgsl
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenWgsl
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
