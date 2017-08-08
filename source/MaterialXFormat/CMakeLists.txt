include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB pugixml_source "${CMAKE_CURRENT_SOURCE_DIR}/PugiXML/*.cpp")
file(GLOB pugixml_headers "${CMAKE_CURRENT_SOURCE_DIR}/PugiXML/*.hpp")

source_group("Source Files\\PugiXml" FILES ${pugixml_source})
source_group("Header Files\\PugiXml" FILES ${pugixml_headers})

add_library(MaterialXFormat STATIC ${materialx_source} ${materialx_headers} ${pugixml_source} ${pugixml_headers})

set_target_properties(
    MaterialXFormat PROPERTIES
    OUTPUT_NAME MaterialXFormat
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXFormat
    MaterialXCore
    ${CMAKE_DL_LIBS}
)

install(TARGETS MaterialXFormat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(FILES ${materialx_headers}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXFormat/)
