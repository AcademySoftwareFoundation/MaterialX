file(GLOB pymaterialxgenslang_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenslang_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenSlang SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenslang_source} ${pymaterialxgenslang_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenSlang PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenSlang
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenSlang
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenSlang
    PUBLIC PyMaterialXGenShader
           MaterialXGenSlang
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenSlang
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
