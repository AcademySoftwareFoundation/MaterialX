set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
set(SETUP_PY "${CMAKE_INSTALL_PREFIX}/python/setup.py")

configure_file(${SETUP_PY_IN} ${SETUP_PY})

add_custom_command(OUTPUT ${SETUP_PY}
                   COMMAND ${PYTHON} ${SETUP_PY} build)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MaterialX" DESTINATION "python" MESSAGE_NEVER)

if(MATERIALX_PYTHON_OCIO_DIR)
  if(NOT EXISTS "${MATERIALX_PYTHON_OCIO_DIR}/config.ocio")
    message(WARNING "No file named config.ocio was found in the given OCIO directory.")
  endif()
  install(DIRECTORY "${MATERIALX_PYTHON_OCIO_DIR}/" DESTINATION "python/MaterialX/config/" MESSAGE_NEVER)
endif()

if(MATERIALX_INSTALL_PYTHON AND PYTHON_EXECUTABLE)
  install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} install)" MESSAGE_NEVER)
endif()
