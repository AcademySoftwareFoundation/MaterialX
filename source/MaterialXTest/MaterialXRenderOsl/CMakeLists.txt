file(GLOB source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(MaterialXTest PUBLIC ${source} ${headers})

target_compile_definitions(MaterialXTest PRIVATE
        MATERIALX_OSL_BINARY_OSLC=\"${MATERIALX_OSL_BINARY_OSLC}\"
        MATERIALX_OSL_BINARY_TESTRENDER=\"${MATERIALX_OSL_BINARY_TESTRENDER}\"
        MATERIALX_OSL_INCLUDE_PATH=\"${MATERIALX_OSL_INCLUDE_PATH}\"
)

add_tests("${source}")

assign_source_group("Source Files" ${source})
assign_source_group("Header Files" ${headers})
