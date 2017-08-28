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

add_library(MayaExporter SHARED 
    ${_source}
    ${_headers}
    ${_node_translator_source}
    ${_node_translator_headers}
)

set_target_properties(
    MayaExporter
    PROPERTIES 
    OUTPUT_NAME MaterialXExport
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION ${MAYA_EXPORTER_VERSION}
    SOVERSION ${SOVERSION})

set_target_properties(
    MayaExporter
    PROPERTIES
    PREFIX ""
    SUFFIX ".mll")

include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../source
)
DEPEND_MAYAAPI_INCLUDE()

target_link_libraries(
    MayaExporter
    MaterialXCore
    MaterialXFormat
    ${CMAKE_DL_LIBS}
)
DEPEND_MAYAAPI_LINK(MayaExporter)

install(TARGETS MayaExporter DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MayaExporter/plug-ins)
install(FILES ${_scripts} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MayaExporter/scripts)
install(FILES ${_data} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MayaExporter/data)
install(FILES ${_mods} DESTINATION ${CMAKE_INSTALL_PREFIX}/contrib/MayaExporter)
