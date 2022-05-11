configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Generated.h.in ${CMAKE_CURRENT_BINARY_DIR}/Generated.h)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_BINARY_DIR}/*.h")

add_library(MaterialXCore ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_CORE_EXPORTS)

if (MATERIALX_BUILD_SHARED_LIBS)
target_compile_definitions(MaterialXCore PUBLIC -DMATERIALX_BUILD_SHARED_LIBS)
endif()

set_target_properties(
    MaterialXCore PROPERTIES
    OUTPUT_NAME MaterialXCore${MATERIALX_LIBNAME_SUFFIX}
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXCore
    ${CMAKE_DL_LIBS})

target_include_directories(MaterialXCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS})

install(TARGETS MaterialXCore
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(FILES ${materialx_headers}
    DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXCore/)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXCore.pdb"
    DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
