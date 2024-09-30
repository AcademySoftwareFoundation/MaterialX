# Use `CMAKE_MODULE_PATH` to be able to find the Sphinx executables
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(Sphinx REQUIRED)

set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_RST_SOURCE_DIR ${SPHINX_SOURCE_DIR}/sources)
set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_HTML_OUTPUT_DIR ${SPHINX_OUTPUT_DIR})
set(MATERIALX_PYTHON_DOCS_TARGET_DEPENDENCIES
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
    PyMaterialXRenderMsl)

# Generate the Sphinx configuration file `conf.py` from `conf.py.in`
set(MATERIALX_PYTHONPATH ${CMAKE_BINARY_DIR}/lib)
set(MATERIALX_LOGO_FILENAME "MaterialXLogo_200x155.png")
configure_file(${SPHINX_SOURCE_DIR}/conf.py.in
               ${SPHINX_OUTPUT_DIR}/conf.py)

# Add a custom target to invoke `sphinx-build` to generate the Python API docs,
# which depends on the Python bindings to be built
add_custom_target(MaterialXDocsPython
                  ${SPHINX_EXECUTABLE} --fresh-env --conf-dir ${SPHINX_OUTPUT_DIR}
                  ${SPHINX_RST_SOURCE_DIR}
                  ${SPHINX_HTML_OUTPUT_DIR}
                  WORKING_DIRECTORY ${SPHINX_OUTPUT_DIR}
                  DEPENDS ${MATERIALX_PYTHON_DOCS_TARGET_DEPENDENCIES}
                  COMMENT "Building MaterialX Python API Documentation: ${SPHINX_HTML_OUTPUT_DIR}/index.html")

# Add post-build commands to copy our logo and custom style sheet to the "_static" folder
add_custom_command(TARGET MaterialXDocsPython
                   POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy
                       ${SPHINX_SOURCE_DIR}/custom.css
                       ${SPHINX_HTML_OUTPUT_DIR}/_static/custom.css)
add_custom_command(TARGET MaterialXDocsPython
                   POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/documents/Images/${MATERIALX_LOGO_FILENAME}
                       ${SPHINX_HTML_OUTPUT_DIR}/_static/)

install(DIRECTORY ${SPHINX_OUTPUT_DIR}
        DESTINATION "documents/PythonAPI" MESSAGE_NEVER)
