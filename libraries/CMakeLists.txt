
set(MATERIALX_DATA_LIBRARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/data_library)

function(build_data_library TARGET_NAME SOURCE_DIR DEST_DIR)

    if (${MATERIALX_BUILD_BAKE_NAMED_VALUES})
        add_custom_target(${TARGET_NAME} ALL
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${SOURCE_DIR}/
            ${DEST_DIR}
                COMMAND MaterialXBuildLibrary --sourceLibraryRoot ${SOURCE_DIR} --destLibraryRoot ${DEST_DIR}
                COMMAND ${CMAKE_COMMAND} -E rm -f ${DEST_DIR}/CmakeLists.txt
        )
    else()
        add_custom_target(${TARGET_NAME} ALL
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                    ${SOURCE_DIR}/
                    ${DEST_DIR}
                COMMAND ${CMAKE_COMMAND} -E rm -f ${DEST_DIR}/CmakeLists.txt
        )
    endif()

endfunction(build_data_library)

build_data_library(buildDataLibrary ${CMAKE_CURRENT_SOURCE_DIR} ${MATERIALX_DATA_LIBRARY_BUILD_DIR})

if(MATERIALX_BUILD_TESTS)
    build_data_library(buildTestDataLibrary ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libraries)
endif()



if(NOT SKBUILD)
    install(DIRECTORY ${MATERIALX_DATA_LIBRARY_BUILD_DIR}/
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}"
    )
endif()

set(MATERIALX_PYTHON_LIBRARIES_PATH "${MATERIALX_PYTHON_FOLDER_NAME}/${MATERIALX_INSTALL_STDLIB_PATH}")
if(SKBUILD)
    set(MATERIALX_PYTHON_LIBRARIES_PATH "${SKBUILD_PLATLIB_DIR}/MaterialX/libraries")
endif()

if(MATERIALX_BUILD_PYTHON)
    install(DIRECTORY ${MATERIALX_DATA_LIBRARY_BUILD_DIR}/
            DESTINATION "${MATERIALX_PYTHON_LIBRARIES_PATH}")
endif()
