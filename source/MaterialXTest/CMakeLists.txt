include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_executable(MaterialXTest ${materialx_source} ${materialx_headers})

# Discover all tests and allow them to be run in parallel (ctest -j20):
foreach(src_file ${materialx_source})
    file(STRINGS ${src_file} matched_lines REGEX "TEST_CASE")
    foreach(matched_line ${matched_lines})
        string(REGEX REPLACE "(TEST_CASE[( \"]+)" "" test_name ${matched_line})
        string(REGEX REPLACE "(\".*)" "" test_name ${test_name})
        string(REGEX REPLACE "[^A-Za-z0-9_]+" "_" test_safe_name ${test_name})
        add_test(NAME "MaterialXTest_${test_safe_name}"
            COMMAND MaterialXTest ${test_name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endforeach()
endforeach()

if(MATERIALX_BUILD_OIIO AND OPENIMAGEIO_ROOT_DIR)
    add_custom_command(TARGET MaterialXTest POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${OPENIMAGEIO_ROOT_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../libraries ${CMAKE_CURRENT_BINARY_DIR}/libraries)
add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources)

set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}"
    DEBUG_POSTFIX "${MATERIALX_DEBUG_POSTFIX}")

set(LIBS
    MaterialXCore
    MaterialXFormat
    MaterialXRuntime
    MaterialXGenShader
    MaterialXGenOsl
    MaterialXGenGlsl
    MaterialXRender
    MaterialXRenderOsl
    MaterialXRenderHw
    MaterialXRenderGlsl
)

# Add language specific generators
if (MATERIALX_BUILD_GEN_ARNOLD)
    LIST(APPEND LIBS "MaterialXGenArnold")
endif()
if (MATERIALX_BUILD_GEN_OGSFX)
    LIST(APPEND LIBS "MaterialXGenOgsFx")
endif()
if (MATERIALX_BUILD_GEN_OGSXML)
    LIST(APPEND LIBS "MaterialXGenOgsXml")
endif()

# Add contrib libraries
if (MATERIALX_BUILD_CONTRIB)
    LIST(APPEND LIBS "MaterialXContrib")
endif()

target_link_libraries(
    MaterialXTest
    ${LIBS}
    ${CMAKE_DL_LIBS})
