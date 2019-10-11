include_directories(    
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB pugixml_source "${CMAKE_CURRENT_SOURCE_DIR}/../MaterialXFormat/PugiXML/*.cpp")
file(GLOB pugixml_headers "${CMAKE_CURRENT_SOURCE_DIR}/../MaterialXFormat/PugiXML/*.hpp")
file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/External/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Handlers/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/External/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Handlers/*.h")

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

add_library(MaterialXContrib STATIC 
    ${materialx_source} 
    ${materialx_header}
    ${pugixml_source} 
    ${pugixml_headers})

set_target_properties(
    MaterialXContrib PROPERTIES
    OUTPUT_NAME MaterialXContrib
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX ${MATERIALX_DEBUG_POSTFIX})

target_link_libraries(
    MaterialXContrib
    MaterialXRender
    ${CMAKE_DL_LIBS})

install(TARGETS MaterialXContrib
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/External" "${CMAKE_CURRENT_SOURCE_DIR}/Handlers"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXContrib/
    FILES_MATCHING PATTERN "*.h*"
    PATTERN "MaterialXMaya/*.h*" EXCLUDE)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXContrib.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)

add_subdirectory(MaterialXMaya)
