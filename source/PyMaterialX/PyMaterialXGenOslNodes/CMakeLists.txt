file(GLOB pymaterialxgenoslnodes_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pymaterialxgenoslnodes_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

pybind11_add_module(PyMaterialXGenOslNodes SHARED ${PYBIND11_MODULE_FLAGS} ${pymaterialxgenoslnodes_source} ${pymaterialxgenoslnodes_headers})

if(APPLE)
    set_target_properties(PyMaterialXGenOslNodes PROPERTIES CXX_VISIBILITY_PRESET "default")
endif()

set_target_properties(
    PyMaterialXGenOslNodes
    PROPERTIES
    OUTPUT_NAME PyMaterialXGenOslNodes
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_UP_TWO_RPATH}"
    DEBUG_POSTFIX "${MATERIALX_PYTHON_DEBUG_POSTFIX}")

target_link_libraries(
    PyMaterialXGenOslNodes
    PUBLIC PyMaterialXGenShader
           MaterialXGenOslNodes
    PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS PyMaterialXGenOslNodes
        DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}")
