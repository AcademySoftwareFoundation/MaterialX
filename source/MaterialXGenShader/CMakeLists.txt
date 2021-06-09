file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(NOT MATERIALX_BUILD_OCIO)
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/OCIOColorManagementSystem.cpp")
endif()

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXGenShader ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_GENSHADER_EXPORTS)

target_link_libraries(
    MaterialXGenShader
    MaterialXCore
    MaterialXFormat
    ${CMAKE_DL_LIBS})

if(MATERIALX_BUILD_OCIO)
    set(MATERIALX_OCIO_DIR ${MATERIALX_OCIO_DIR})
    message(STATUS "OCIO root path: ${MATERIALX_OCIO_DIR}")

    message(STATUS "OCIO include path: ${MATERIALX_OCIO_DIR}/include")
    include_directories("${MATERIALX_OCIO_DIR}/include")

    message(STATUS "OCIO library path: ${MATERIALX_OCIO_DIR}/lib")
    file(GLOB OCIO_LIBRARIES "${MATERIALX_OCIO_DIR}/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
    message(STATUS "OCIO library files: ${OCIO_LIBRARIES}")
    target_link_libraries(MaterialXGenShader "${OCIO_LIBRARIES}")
    # Expose to other libraries
    set(MATERIALX_OCIO_LIBRARIES ${OCIO_LIBRARIES} PARENT_SCOPE)
endif()

set_target_properties(
    MaterialXGenShader PROPERTIES
    OUTPUT_NAME MaterialXGenShader
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_include_directories(MaterialXGenShader
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS})

install(TARGETS MaterialXGenShader
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXGenShader/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGenShader.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../resources"
    DESTINATION ${CMAKE_INSTALL_PREFIX} MESSAGE_NEVER)
