include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

add_library(MaterialXGenShader STATIC
    ${materialx_source}
    ${materialx_header}
)

set_target_properties(
    MaterialXGenShader PROPERTIES
    OUTPUT_NAME MaterialXGenShader
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXGenShader
    MaterialXCore
    ${CMAKE_DL_LIBS})

install(TARGETS MaterialXGenShader
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXGenShader/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGenShader.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../resources" DESTINATION ${CMAKE_INSTALL_PREFIX} MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/Examples" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/Libraries" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/TestSuite" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents MESSAGE_NEVER)
