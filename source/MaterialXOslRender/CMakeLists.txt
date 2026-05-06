file(GLOB materialx_osl_render_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(materialx-osl ${materialx_osl_render_source})

target_link_libraries(materialx-osl
    MaterialXCore
    MaterialXFormat
    MaterialXGenShader
    MaterialXGenOsl
    MaterialXRender
    MaterialXRenderOsl)

target_include_directories(materialx-osl PRIVATE
    ${EXTERNAL_INCLUDE_DIRS})

target_compile_definitions(materialx-osl PRIVATE
    MATERIALX_OSL_BINARY_OSLC=\"${MATERIALX_OSL_BINARY_OSLC}\"
    MATERIALX_OSL_BINARY_TESTRENDER=\"${MATERIALX_OSL_BINARY_TESTRENDER}\"
    MATERIALX_OSL_INCLUDE_PATH=\"${MATERIALX_OSL_INCLUDE_PATH}\"
    MATERIALX_SOURCE_ROOT=\"${PROJECT_SOURCE_DIR}\"
    MATERIALX_RUNTIME_OUTPUT_DIR=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\")

if(MATERIALX_BUILD_OIIO)
    target_compile_definitions(materialx-osl PRIVATE MATERIALX_BUILD_OIIO)
endif()

set_target_properties(materialx-osl PROPERTIES
    INSTALL_RPATH "${MATERIALX_UP_ONE_RPATH}")

add_custom_target(MaterialXOslRenderResources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Utilities
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../MaterialXTest/MaterialXRenderOsl/Utilities
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Utilities
    COMMENT "Copying OSL render utilities")

add_dependencies(materialx-osl MaterialXOslRenderResources)

install(TARGETS materialx-osl
    EXPORT MaterialX
    RUNTIME DESTINATION ${MATERIALX_INSTALL_BIN_PATH})
