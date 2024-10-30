file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXGenMdl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenShader
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENMDL_EXPORTS)

if(NOT SKBUILD)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mdl
            DESTINATION "${MATERIALX_INSTALL_MDL_MODULE_PATH}")
endif()
