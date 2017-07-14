include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE catch_headers "${CMAKE_CURRENT_SOURCE_DIR}/Catch/*.hpp")

source_group("Header Files\\Catch" FILES ${catch_headers})

add_executable(MaterialXTest ${materialx_source} ${catch_headers})

add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/Libraries ${CMAKE_CURRENT_BINARY_DIR}/documents/Libraries)

add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/Examples ${CMAKE_CURRENT_BINARY_DIR}/documents/Examples)
                   
set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXTest
    MaterialXCore
    MaterialXFormat
    MaterialXShaderGen
    ${CMAKE_DL_LIBS}
)
