if(NOT SKBUILD)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MaterialX" DESTINATION "python" MESSAGE_NEVER)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" DESTINATION "python" MESSAGE_NEVER)
endif()

if(SKBUILD)
  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/"
    DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}/_scripts"
    PATTERN "README.md" EXCLUDE
  )
endif()

if(MATERIALX_PYTHON_OCIO_DIR)
  if(NOT EXISTS "${MATERIALX_PYTHON_OCIO_DIR}/config.ocio")
    message(WARNING "No file named config.ocio was found in the given OCIO directory.")
  endif()
  install(DIRECTORY "${MATERIALX_PYTHON_OCIO_DIR}/" DESTINATION "${MATERIALX_PYTHON_FOLDER_NAME}/config/" MESSAGE_NEVER)
endif()

if (MATERIALX_PYTHON_STUBS)
  # attempts to find mypy stubgen executable
  find_program(_MYPY_STUBGEN_EXECUTABLE "stubgen")
  message("Found stubgen executable: _MYPY_STUBGEN_EXECUTABLE=${MATERIALX_PYTHON_STUBS}")

  # check if stubgen can be accessed
  execute_process(
    COMMAND ${_MYPY_STUBGEN_EXECUTABLE} --help
    OUTPUT_VARIABLE _MYPY_STUBGEN_HELP_OUTPUT
    ERROR_VARIABLE _MYPY_STUBGEN_HELP_OUTPUT
    RESULT_VARIABLE _MYPY_STUBGEN_HELP_EXITCODE
  )

  # build stubs if stubgen could be found properly
  if (_MYPY_STUBGEN_HELP_EXITCODE EQUAL "0")
    install(CODE "
      execute_process(
        COMMAND ${_MYPY_STUBGEN_EXECUTABLE} -p MaterialX --inspect-mode -o \"${CMAKE_INSTALL_PREFIX}/python\" -v --ignore-errors
        WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/python\")
    ")
  else ()
    # prints errors if mypy couldn't be found
    # TODO: should this be a fatal error or?
    message(
        FATAL_ERROR
        "MATERIALX_PYTHON_STUBS=${MATERIALX_PYTHON_STUBS} but"
        " mypy stubgen not found: status=${_MYPY_STUBGEN_HELP_EXITCODE}:"
        " ${_MYPY_STUBGEN_HELP_OUTPUT}"
    )
  endif ()
endif()

if(MATERIALX_INSTALL_PYTHON AND PYTHON_EXECUTABLE AND NOT SKBUILD)
  set(SETUP_PY "${CMAKE_INSTALL_PREFIX}/python/setup.py")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in" "${SETUP_PY}")
  install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install clean --all)")
endif()
