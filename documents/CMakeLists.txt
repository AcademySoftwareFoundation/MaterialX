# MaterialX C++ API Documentation
if(MATERIALX_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
        set(DOXYGEN_HTML_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/html)
        set(
            DOXYGEN_INPUT_LIST
            ${CMAKE_SOURCE_DIR}/documents/DeveloperGuide/MainPage.md
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
            ${CMAKE_SOURCE_DIR}/source/MaterialXRenderOsl
        )
        string (REPLACE ";" " " DOXYGEN_INPUT_STR "${DOXYGEN_INPUT_LIST}")

        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        add_custom_target(
            MaterialXDocs
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Building MaterialX C++ API Documentation: ${DOXYGEN_HTML_OUTPUT_DIR}/index.html"
        )
        add_custom_command(
            TARGET MaterialXDocs PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
                -E copy_directory ${CMAKE_SOURCE_DIR}/documents/DoxygenAwesome
                                  ${DOXYGEN_HTML_OUTPUT_DIR}
        )
        add_custom_command(
            TARGET MaterialXDocs PRE_BUILD
            COMMAND ${CMAKE_COMMAND}
                -E copy_directory ${CMAKE_SOURCE_DIR}/documents/Images
                                  ${DOXYGEN_HTML_OUTPUT_DIR}
        )
        install(DIRECTORY ${DOXYGEN_HTML_OUTPUT_DIR}
                DESTINATION "documents"
                MESSAGE_NEVER)
    else()
        message(WARNING "Unable to build MaterialX C++ API Documentation: Doxygen not found.")
    endif(DOXYGEN_FOUND)
endif(MATERIALX_BUILD_DOCS)


# MaterialX Python API Documentation
if(MATERIALX_BUILD_PYTHON_DOCS)
    find_package(Sphinx)
    if(SPHINX_FOUND)
        set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/documents/DeveloperGuide/)
        set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
        set(SPHINX_HTML_OUTPUT_DIR ${SPHINX_OUTPUT_DIR}/html-sphinx)
        set(
            MATERIALX_PYTHON_DOCS_TARGET_DEPENDENCIES
            MaterialXCore
            PyMaterialXCore
            PyMaterialXFormat
            PyMaterialXGenShader
            PyMaterialXGenGlsl
            PyMaterialXGenOsl
            PyMaterialXGenMdl
            PyMaterialXGenMsl
            PyMaterialXRender
            PyMaterialXRenderGlsl
            PyMaterialXRenderOsl
            PyMaterialXRenderMsl
        )

        # Configure the Sphinx configuration file `conf.py`
        set(MATERIALX_PYTHONPATH ${CMAKE_SOURCE_DIR}/lib)
        set(MATERIALX_LOGO_FILENAME "MaterialXLogo_200x155.png")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sphinx-conf.py.in
                       ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

        # Add a custom target to invoke `sphinx-build` to generate the Python
        # API docs, which depends on the Python bindings to be built
        add_custom_target(
            MaterialXDocsPython
            ${SPHINX_EXECUTABLE}
                -c ${CMAKE_CURRENT_BINARY_DIR}
                ${SPHINX_SOURCE_DIR}
                ${SPHINX_HTML_OUTPUT_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${MATERIALX_PYTHON_DOCS_TARGET_DEPENDENCIES}
            COMMENT "Building MaterialX Python API Documentation: ${SPHINX_HTML_OUTPUT_DIR}/index.html"
        )

        # Add post-build commands to copy our logo and custom style sheet to the
        # "_static" folder
        add_custom_command(
            TARGET MaterialXDocsPython POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -E copy ${CMAKE_SOURCE_DIR}/documents/sphinx-custom.css
                        ${SPHINX_HTML_OUTPUT_DIR}/_static/custom.css
        )
        add_custom_command(
            TARGET MaterialXDocsPython POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -E copy ${CMAKE_SOURCE_DIR}/documents/Images/${MATERIALX_LOGO_FILENAME}
                        ${SPHINX_HTML_OUTPUT_DIR}/_static/
        )
    else()
        message(WARNING "Unable to build MaterialX Python API Documentation: Sphinx not found.")
    endif(SPHINX_FOUND)
endif(MATERIALX_BUILD_PYTHON_DOCS)
