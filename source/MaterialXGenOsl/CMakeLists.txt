file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/LibsToOso.cpp")

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

if (MATERIALX_BUILD_OSOS)
    file(GLOB GenNodes_SRC "${CMAKE_CURRENT_SOURCE_DIR}/LibsToOso.cpp")

    add_executable(MaterialXGenOsl_LibsToOso ${GenNodes_SRC})

    target_link_libraries(MaterialXGenOsl_LibsToOso
        PRIVATE
            MaterialXCore
            MaterialXFormat
            MaterialXGenShader
            MaterialXGenOsl)

    if (OSL_FOUND)
        target_link_libraries(MaterialXGenOsl_LibsToOso
            PRIVATE
                OSL::oslcomp)
        target_compile_definitions(MaterialXGenOsl_LibsToOso
            PRIVATE
                USE_OSLCOMP)
    endif()

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
