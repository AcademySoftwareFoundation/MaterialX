# Use `CMAKE_MODULE_PATH` to be able to find the Sphinx executables
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(Python3 REQUIRED)
find_package(Sphinx REQUIRED)

execute_process(COMMAND ${Python3_EXECUTABLE} -m pip show myst-parser
                RESULT_VARIABLE RESULT
                OUTPUT_QUIET)
if(NOT ${RESULT} EQUAL 0)
    message(FATAL_ERROR
            "The `myst-parser` Python package does not appear to be available for `${Python3_EXECUTABLE}`.\n"
            "Consider installing it via `pip install myst-parser`.\n"
            "PyPI: https://pypi.org/project/myst-parser/")
endif()

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
                  COMMENT "Building MaterialX Python API Documentation in ${SPHINX_HTML_OUTPUT_DIR}/ ...")

# Add post-build commands to copy our logo and style sheets to the "_static" folder
add_custom_command(TARGET MaterialXDocsPython
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_SOURCE_DIR}/documents/Images/${MATERIALX_LOGO_FILENAME}
                       ${SPHINX_HTML_OUTPUT_DIR}/_static/
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${SPHINX_SOURCE_DIR}/custom.css
                       ${SPHINX_HTML_OUTPUT_DIR}/_static/custom.css
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/documents/DoxygenAwesome
                       ${SPHINX_HTML_OUTPUT_DIR}/_static/DoxygenAwesome
                   COMMENT "Python API Documentation built: open ${SPHINX_HTML_OUTPUT_DIR}/index.html to view")

install(DIRECTORY ${SPHINX_OUTPUT_DIR}
        DESTINATION "documents/PythonAPI" MESSAGE_NEVER)
