if(MATERIALX_BUILD_DATA_LIBRARY)
    # Build generated products from the MaterialX data library.
    # Initially, this step is a simple copy across folders, but our intent
    # is for it to include meaningful work in the future.

    set(DATA_LIBRARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/DataLibraryBuild)

    file(GLOB_RECURSE MATERIALX_DATA_LIBRARY_SOURCE_FILES
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         LIST_DIRECTORIES false
         *.mtlx
         *.md
         *.glsl
         *.osl
         *.h
         *.metal)

    foreach(SOURCE_FILE IN LISTS MATERIALX_DATA_LIBRARY_SOURCE_FILES)
        set(SOURCE_FILEPATH ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
        set(DEST_FILEPATH ${DATA_LIBRARY_BUILD_DIR}/${SOURCE_FILE})
        add_custom_command(
                OUTPUT ${DEST_FILEPATH}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOURCE_FILEPATH} ${DEST_FILEPATH}
                DEPENDS ${SOURCE_FILEPATH})
        list(APPEND MATERIALX_DATA_LIBRARY_BUILD_FILES ${DEST_FILEPATH})
    endforeach()

    add_custom_target(MaterialXBuildData ALL
        DEPENDS ${MATERIALX_DATA_LIBRARY_BUILD_FILES})

    set(DATA_LIBRARY_DIR ${DATA_LIBRARY_BUILD_DIR})
else()
    set(DATA_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT SKBUILD)
    install(DIRECTORY ${DATA_LIBRARY_DIR}/
            DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}"
            PATTERN "CMakeLists.txt" EXCLUDE)
endif()

if(MATERIALX_BUILD_PYTHON)
    set(MATERIALX_PYTHON_LIBRARIES_PATH "${MATERIALX_PYTHON_FOLDER_NAME}/${MATERIALX_INSTALL_STDLIB_PATH}")
    if(SKBUILD)
        set(MATERIALX_PYTHON_LIBRARIES_PATH "${SKBUILD_PLATLIB_DIR}/MaterialX/libraries")
    endif()

    install(DIRECTORY ${DATA_LIBRARY_DIR}/
            DESTINATION "${MATERIALX_PYTHON_LIBRARIES_PATH}"
            PATTERN "CMakeLists.txt" EXCLUDE)
endif()

set(MATERIALX_DATA_LIBRARY_DIR ${DATA_LIBRARY_DIR} PARENT_SCOPE)
