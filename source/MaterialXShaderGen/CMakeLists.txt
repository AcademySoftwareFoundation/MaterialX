include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB shadergenerators_source "${CMAKE_CURRENT_SOURCE_DIR}/ShaderGenerators/*.cpp")
file(GLOB shadergenerators_header "${CMAKE_CURRENT_SOURCE_DIR}/ShaderGenerators/*.h")
file(GLOB implementations_source "${CMAKE_CURRENT_SOURCE_DIR}/Implementations/*.cpp")
file(GLOB implementations_header "${CMAKE_CURRENT_SOURCE_DIR}/Implementations/*.h")

source_group("Header Files\\Implementations" FILES ${implementations_header})
source_group("Source Files\\Implementations" FILES ${implementations_source})
source_group("Header Files\\ShaderGenerators" FILES ${shadergenerators_header})
source_group("Source Files\\ShaderGenerators" FILES ${shadergenerators_source})

add_library(MaterialXShaderGen STATIC 
    ${materialx_source} 
    ${materialx_header}
    ${implementations_source} 
    ${implementations_header}
    ${shadergenerators_source} 
    ${shadergenerators_header}
)

set_target_properties(
    MaterialXShaderGen PROPERTIES
    OUTPUT_NAME MaterialXShaderGen
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXShaderGen
    MaterialXCore
    ${CMAKE_DL_LIBS}
)

install(TARGETS MaterialXShaderGen
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(FILES ${materialx_header}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXShaderGen/)
install(FILES ${shadergenerators_header}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXShaderGen/ShaderGenerators)
install(FILES ${implementations_header}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXShaderGen/Implementations)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXShaderGen.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/documents/Examples" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/documents/Libraries" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents)
