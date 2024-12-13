
file(GLOB materialx_source       "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers      "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

set(TARGET_NAME MaterialXBuildLibrary)

set(MATERIALX_LIBRARIES
        MaterialXFormat
        MaterialXCore)

add_executable(${TARGET_NAME} ${materialx_source} ${materialx_headers})

target_link_libraries(
        ${TARGET_NAME}
        ${MATERIALX_LIBRARIES})
