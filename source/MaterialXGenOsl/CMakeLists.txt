file(GLOB_RECURSE materialx_source
 "${CMAKE_CURRENT_SOURCE_DIR}/OslShaderGenerator.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/OslSyntax.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkShaderGenerator.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkSyntax.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/Nodes/OsoNode.cpp")

file(GLOB_RECURSE materialx_headers
  "${CMAKE_CURRENT_SOURCE_DIR}/OslShaderGenerator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OslSyntax.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkShaderGenerator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/OslNetworkSyntax.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Nodes/OsoNode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Export.h")

mx_add_library(MaterialXGenOsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenShader
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENOSL_EXPORTS)

# FIXME: LibsToOso has a dependency on the OslRenderer.
if (MATERIALX_BUILD_RENDER)
    file(GLOB GenNodes_SRC "${CMAKE_CURRENT_SOURCE_DIR}/LibsToOso.cpp")

    set(MATERIALX_LIBRARIES
        MaterialXCore
        MaterialXFormat
        MaterialXGenShader
        MaterialXGenOsl
        MaterialXRenderOsl)

    add_executable(MaterialXGenOsl_LibsToOso ${GenNodes_SRC})

    target_link_libraries(
        MaterialXGenOsl_LibsToOso
        ${MATERIALX_LIBRARIES})

    set_target_properties(
        MaterialXGenOsl_LibsToOso PROPERTIES
        INSTALL_RPATH "${MATERIALX_UP_ONE_RPATH}")

    # TODO: We likely want to install that file elsewhere and not under `bin`,
    # if at all, as we maybe want to keep this executable available at build time only.
    install(TARGETS MaterialXGenOsl_LibsToOso
        EXPORT MaterialX
        RUNTIME DESTINATION ${MATERIALX_INSTALL_BIN_PATH})
    if(MSVC)
        install(FILES $<TARGET_PDB_FILE:MaterialXGenOsl_LibsToOso>
                DESTINATION ${MATERIALX_INSTALL_BIN_PATH} OPTIONAL)
    endif()
endif()
