set(TARGET_NAME MaterialXBuildLibrary)

file(GLOB materialx_source       "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers      "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

add_executable(${TARGET_NAME} ${materialx_source} ${materialx_headers})

target_link_libraries(${TARGET_NAME} PRIVATE
        MaterialXFormat
        MaterialXCore)
