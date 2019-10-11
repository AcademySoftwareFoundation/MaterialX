include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXGenGlsl STATIC ${materialx_source} ${materialx_headers})

set_target_properties(
    MaterialXGenGlsl PROPERTIES
    OUTPUT_NAME MaterialXGenGlsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX ${MATERIALX_DEBUG_POSTFIX})

target_link_libraries(
    MaterialXGenGlsl
    MaterialXGenShader
    MaterialXCore
    ${CMAKE_DL_LIBS})

install(TARGETS MaterialXGenGlsl
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXGenGlsl/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGenGlsl.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)
