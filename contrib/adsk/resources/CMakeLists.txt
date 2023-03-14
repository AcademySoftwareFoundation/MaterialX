# Copy adsk resoures to output

add_custom_target(Contrib_resources ALL)

add_custom_command(TARGET Contrib_resources
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "resources" MESSAGE_NEVER)
