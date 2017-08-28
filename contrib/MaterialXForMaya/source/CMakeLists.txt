file(GLOB _source  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB _headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB _node_translator_source  "${CMAKE_CURRENT_SOURCE_DIR}/NodeTranslators/*.cpp")
file(GLOB _node_translator_headers "${CMAKE_CURRENT_SOURCE_DIR}/NodeTranslators/*.h")

file(GLOB _scripts "${CMAKE_CURRENT_SOURCE_DIR}/*.mel")
file(GLOB _mods "${CMAKE_CURRENT_SOURCE_DIR}/*.mod")
file(GLOB _data "${CMAKE_CURRENT_SOURCE_DIR}/../data/*.*")

source_group("Header Files" FILES ${_headers})
source_group("Source Files" FILES ${_source})
source_group("Header Files\\NodeTranslators" FILES ${_node_translator_headers})
source_group("Source Files\\NodeTranslators" FILES ${_node_translator_source})

add_library(MaterialXForMaya SHARED 
    ${_source}
    ${_headers}
    ${_node_translator_source}
    ${_node_translator_headers}
)

set_target_properties(
    MaterialXForMaya
    PROPERTIES 
    OUTPUT_NAME MaterialX
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION ${MXTOOLS_VERSION}
    SOVERSION ${SOVERSION})

set_target_properties(
    MaterialXForMaya
    PROPERTIES
    PREFIX ""
    SUFFIX ".mll")

include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${MATERIALX_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../source
)
DEPEND_MAYAAPI_INCLUDE()

target_link_libraries(
    MaterialXForMaya
    ${MATERIALX_DIR}/lib/MaterialXCore.lib
    ${MATERIALX_DIR}/lib/MaterialXFormat.lib
    ${CMAKE_DL_LIBS}
)
DEPEND_MAYAAPI_LINK(MaterialXForMaya)

install(TARGETS MaterialXForMaya DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MaterialXForMaya/plug-ins)
install(FILES ${_scripts} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MaterialXForMaya/scripts)
install(FILES ${_data} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MaterialXForMaya/data)
install(FILES ${_mods} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MaterialXForMaya)
