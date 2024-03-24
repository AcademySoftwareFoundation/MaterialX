set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DOXYGEN_HTML_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/html)
set(DOXYGEN_INPUT_LIST ${CMAKE_CURRENT_BINARY_DIR}/MainPage.md)

set(MATERIALX_DOXYGEN_SOURCE_FOLDERS
    ${CMAKE_SOURCE_DIR}/source/MaterialXCore
    ${CMAKE_SOURCE_DIR}/source/MaterialXFormat
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenShader
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenShader/Nodes
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenGlsl
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenGlsl/Nodes
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenOsl
    ${CMAKE_SOURCE_DIR}/source/MaterialXGenMdl
    ${CMAKE_SOURCE_DIR}/source/MaterialXRender
    ${CMAKE_SOURCE_DIR}/source/MaterialXRenderHw
    ${CMAKE_SOURCE_DIR}/source/MaterialXRenderGlsl
    ${CMAKE_SOURCE_DIR}/source/MaterialXRenderOsl)

find_package(Doxygen REQUIRED)

foreach(FOLDER ${MATERIALX_DOXYGEN_SOURCE_FOLDERS})
    file(GLOB FOLDER_HEADERS ${FOLDER}/*.h)
    list(APPEND DOXYGEN_INPUT_LIST ${FOLDER_HEADERS})
endforeach()

string (REPLACE ";" " " DOXYGEN_INPUT_STR "${DOXYGEN_INPUT_LIST}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MainPage.md
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/DeveloperGuide/MainPage.md ${CMAKE_CURRENT_BINARY_DIR}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DeveloperGuide/MainPage.md)

add_custom_command(OUTPUT ${DOXYGEN_HTML_OUTPUT_DIR}/index.html
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/DoxygenAwesome ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Images ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${DOXYGEN_INPUT_LIST} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                   COMMENT "Generating HTML documentation: ${DOXYGEN_HTML_OUTPUT_DIR}/index.html")
add_custom_target(MaterialXDocs ALL DEPENDS ${DOXYGEN_HTML_OUTPUT_DIR}/index.html)

install(DIRECTORY ${DOXYGEN_HTML_OUTPUT_DIR}
        DESTINATION "documents" MESSAGE_NEVER)
