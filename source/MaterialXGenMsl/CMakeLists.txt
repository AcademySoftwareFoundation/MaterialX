file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXGenMsl ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_GENMSL_EXPORTS)

set_target_properties(
    MaterialXGenMsl PROPERTIES
    OUTPUT_NAME MaterialXGenMsl${MATERIALX_LIBNAME_SUFFIX}
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    INSTALL_RPATH "${MATERIALX_SAME_DIR_RPATH}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXGenMsl
    MaterialXGenShader
    MaterialXCore
    ${CMAKE_DL_LIBS})

target_include_directories(MaterialXGenMsl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS})

install(TARGETS MaterialXGenMsl
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXGenMsl/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGenMsl.pdb"
    DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
