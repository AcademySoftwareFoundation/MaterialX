set(MATERIALX_MODULE_NAME MaterialXCore)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Generated.h.in ${CMAKE_CURRENT_BINARY_DIR}/Generated.h)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_BINARY_DIR}/*.h")

mx_add_library(${MATERIALX_MODULE_NAME}
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    EXPORT_DEFINE
        MATERIALX_CORE_EXPORTS)

# Need to add the binary directory to find the Generated.h file generated above.
target_include_directories(${TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../>)

if (NOT SKBUILD)
    install(FILES ${materialx_headers}
            DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/${MATERIALX_MODULE_NAME}/)
endif()