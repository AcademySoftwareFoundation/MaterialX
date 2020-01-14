include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_executable(MaterialXTest ${materialx_source} ${materialx_headers})

add_test(NAME MaterialXTest
	COMMAND MaterialXTest
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(MATERIALX_BUILD_OIIO AND OPENIMAGEIO_ROOT_DIR)
    add_custom_command(TARGET MaterialXTest POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${OPENIMAGEIO_ROOT_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../libraries ${CMAKE_CURRENT_BINARY_DIR}/libraries)     
install(DIRECTORY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libraries/stdlib/reference/osl)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/stdlib/osl/"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libraries/stdlib/reference/osl 
    FILES_MATCHING PATTERN "*.h")
add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources)

set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

set(LIBS
    MaterialXCore
    MaterialXFormat
    MaterialXGenShader
    MaterialXGenOsl
    MaterialXGenGlsl
    MaterialXRender
    MaterialXRenderOsl
    MaterialXRenderHw
    MaterialXRenderGlsl)

target_link_libraries(
    MaterialXTest
    ${LIBS}
    ${CMAKE_DL_LIBS})
