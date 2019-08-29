include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

add_library(MaterialXCore STATIC ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_MAJOR_VERSION=${MATERIALX_MAJOR_VERSION})
add_definitions(-DMATERIALX_MINOR_VERSION=${MATERIALX_MINOR_VERSION})
add_definitions(-DMATERIALX_BUILD_VERSION=${MATERIALX_BUILD_VERSION})

set_target_properties(
    MaterialXCore PROPERTIES
    OUTPUT_NAME MaterialXCore
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXCore
    ${CMAKE_DL_LIBS})

install(TARGETS MaterialXCore
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

install(FILES ${materialx_headers}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXCore/)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXCore.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)
