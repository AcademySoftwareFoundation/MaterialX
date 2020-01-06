include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXRenderOsl STATIC ${materialx_source} ${materialx_headers})

set_target_properties(
    MaterialXRenderOsl PROPERTIES
    OUTPUT_NAME MaterialXRenderOsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXRenderOsl
    MaterialXRender
    ${CMAKE_DL_LIBS})

install(TARGETS MaterialXRenderOsl
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXRenderOsl/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderOsl.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)
