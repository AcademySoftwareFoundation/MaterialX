file(GLOB pymaterialxgenoslnetwork_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenoslnetwork_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenOslNetwork SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenoslnetwork_source} ${pymaterialxgenoslnetwork_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenOslNetwork PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenOslNetwork
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenOslNetwork
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenOslNetwork
    PUBLIC PyMaterialXGenShader
           MaterialXGenOslNetwork
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenOslNetwork
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
