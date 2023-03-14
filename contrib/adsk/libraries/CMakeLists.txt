# Copy adsk libraries to output

add_custom_target(Contrib_libraries ALL)

add_custom_command(TARGET Contrib_libraries
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libraries)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}" MESSAGE_NEVER
        PATTERN "CMakeLists.txt" EXCLUDE)
