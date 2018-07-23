install(DIRECTORY ${CMAKE_SOURCE_DIR}/documents/Libraries/
        DESTINATION "${CMAKE_INSTALL_PREFIX}/documents/Libraries")

if(MATERIALX_BUILD_DOCS)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(DOXYGEN_HTML_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/MaterialXDocs)
    set(DOXYGEN_INPUT_LIST ${CMAKE_SOURCE_DIR}/documents/DeveloperGuide
                           ${CMAKE_SOURCE_DIR}/source/MaterialXCore
                           ${CMAKE_SOURCE_DIR}/source/MaterialXFormat)
    string (REPLACE ";" " " DOXYGEN_INPUT_STR "${DOXYGEN_INPUT_LIST}")

    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        add_custom_target(MaterialXDocs ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                          COMMENT "Generating HTML documentation: ${DOXYGEN_HTML_OUTPUT_DIR}/index.html")
        add_custom_command(TARGET MaterialXDocs PRE_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           ${CMAKE_SOURCE_DIR}/documents/Images ${CMAKE_CURRENT_BINARY_DIR})
        install(DIRECTORY ${DOXYGEN_HTML_OUTPUT_DIR}
                DESTINATION "${CMAKE_INSTALL_PREFIX}/documents")
    endif(DOXYGEN_FOUND)
endif(MATERIALX_BUILD_DOCS)
