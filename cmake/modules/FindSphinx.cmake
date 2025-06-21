include(FindPackageHandleStandardArgs)

find_package(Python3)
if(PYTHON3_FOUND)
    get_filename_component(_PYTHON_EXECUTABLE_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
    set(_SPHINX_SEARCH_PATHS
        "${_PYTHON_EXECUTABLE_DIR}"
        "${_PYTHON_EXECUTABLE_DIR}/bin"
        "${_PYTHON_EXECUTABLE_DIR}/Scripts"
        "${_PYTHON_EXECUTABLE_DIR}/../opt/sphinx-doc/bin")
    message(STATUS "Looking for Sphinx in the following directories: ${_SPHINX_SEARCH_PATHS}")
    find_program(SPHINX_EXECUTABLE
                 NAMES sphinx-build sphinx-build.exe
                 HINTS ${_SPHINX_SEARCH_PATHS})
    mark_as_advanced(SPHINX_EXECUTABLE)

    find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
endif()
