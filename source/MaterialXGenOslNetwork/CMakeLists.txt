file(GLOB_RECURSE materialx_source
  "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkShaderGenerator.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkSyntax.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Nodes/OsoNode.cpp"
)
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXGenOslNetwork
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenShader
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENOSLNETWORK_EXPORTS)


file(GLOB GenNodes_SRC "${CMAKE_CURRENT_SOURCE_DIR}/LibsToOso.cpp")

set(MATERIALX_LIBRARIES
    MaterialXCore
    MaterialXFormat
    MaterialXGenShader
    MaterialXGenOsl
    MaterialXRenderOsl)

add_executable(MaterialXGenOslNetwork_LibsToOso ${GenNodes_SRC})

target_link_libraries(
    MaterialXGenOslNetwork_LibsToOso
    ${MATERIALX_LIBRARIES})

set_target_properties(
    MaterialXGenOslNetwork_LibsToOso PROPERTIES
    INSTALL_RPATH "${MATERIALX_UP_ONE_RPATH}")

# TODO: We likely want to install that file elsewhere and not under `bin`,
# if at all, as we maybe want to keep this executable available at build time only.
install(TARGETS MaterialXGenOslNetwork_LibsToOso
    EXPORT MaterialX
    RUNTIME DESTINATION ${MATERIALX_INSTALL_BIN_PATH})
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:MaterialXGenOslNetwork_LibsToOso>
            DESTINATION ${MATERIALX_INSTALL_BIN_PATH} OPTIONAL)
endif()
