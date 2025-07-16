file(GLOB_RECURSE source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(MaterialXTest PUBLIC ${source} ${headers})

target_compile_definitions(MaterialXTest PRIVATE
        MATERIALX_MDL_BINARY_MDLC=\"${MATERIALX_MDL_BINARY_MDLC}\"
        MATERIALX_MDL_IMPL_MODULE_PATH=\"${MaterialXGenMdl_SOURCE_DIR}/mdl\"
        MATERIALX_INSTALL_MDL_MODULE_PATH=\"${MATERIALX_INSTALL_STDLIB_PATH}\"
        MATERIALX_MDL_MODULE_PATHS=\"${MATERIALX_MDL_MODULE_PATHS}\"
)

add_tests("${source}")

assign_source_group("Source Files" ${source})
assign_source_group("Header Files" ${headers})
