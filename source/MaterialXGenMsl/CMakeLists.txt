set(MATERIALX_MODULE_NAME MaterialXGenMsl)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(${MATERIALX_MODULE_NAME}
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    LIBRARIES
        MaterialXGenShader
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENMSL_EXPORTS)

if(NOT SKBUILD)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
            DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/${MATERIALX_MODULE_NAME}/ MESSAGE_NEVER
            FILES_MATCHING PATTERN "*.h*")
endif()