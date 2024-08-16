
function(mx_copy_files FILE_LIST DESTINATION_DIR)
    set(options "")
    set(oneValueArgs
            INSTALLED_FILES_LIST)
    set(multiValueArgs
            SRC_DEST_REPLACE_RULE)
    cmake_parse_arguments(args
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    string(COMPARE NOTEQUAL "${args_SRC_DEST_REPLACE_RULE}" "" HAS_REPLACE_RULE)
    string(COMPARE NOTEQUAL "${args_INSTALLED_FILES_LIST}" "" HAS_INSTALLED_FILES_LIST)

    foreach (F ${${FILE_LIST}})
        set(SOURCE_FILE ${F})
        set(DESTINATION_FILE ${F})
        if (HAS_REPLACE_RULE)
            list(GET args_SRC_DEST_REPLACE_RULE 0 SRC_RULE)
            list(GET args_SRC_DEST_REPLACE_RULE 1 DST_RULE)
            string(REPLACE ${SRC_RULE} ${DST_RULE} DESTINATION_FILE ${DESTINATION_FILE})
        endif ()

        get_filename_component(BUILD_DIR ${DESTINATION_DIR}/${DESTINATION_FILE} DIRECTORY)

        add_custom_command(OUTPUT ${DESTINATION_DIR}/${DESTINATION_FILE}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_DIR}
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE} ${DESTINATION_DIR}/${DESTINATION_FILE}
                DEPENDS ${SOURCE_FILE}
        )

        if (HAS_INSTALLED_FILES_LIST)
            list(APPEND ${args_INSTALLED_FILES_LIST} ${DESTINATION_DIR}/${DESTINATION_FILE})
        endif ()
    endforeach ()
    if (HAS_INSTALLED_FILES_LIST)
        set(${args_INSTALLED_FILES_LIST} ${${args_INSTALLED_FILES_LIST}} PARENT_SCOPE)
    endif ()
endfunction()


set(LIBRARIES_BUILD_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/build)

set(SKIP_FILES
        "CMakeLists.txt"
        "pbrlib/genosl/pbrlib_genosl_impl.mtlx"
        "pbrlib/genosl/pbrlib_genosl_impl.legacy")

file(GLOB_RECURSE ALL_FILES CONFIGURE_DEPENDS *.*)
foreach (F ${ALL_FILES})
    cmake_path(RELATIVE_PATH F OUTPUT_VARIABLE RELATIVE_FILE)

    if (${RELATIVE_FILE} IN_LIST SKIP_FILES)
        continue()
    endif ()

    # convert to a list of path parts.
    string(REPLACE "/" ";" PATH_LIST ${RELATIVE_FILE})

    if ("genosl" IN_LIST PATH_LIST)
        list(APPEND GENOSL_FILES ${RELATIVE_FILE})
    elseif ("genmdl" IN_LIST PATH_LIST)
        list(APPEND GENMDL_FILES ${RELATIVE_FILE})
    elseif ("genglsl" IN_LIST PATH_LIST)
        list(APPEND GENGLSL_FILES ${RELATIVE_FILE})
    elseif ("genmsl" IN_LIST PATH_LIST)
        list(APPEND GENMSL_FILES ${RELATIVE_FILE})
    elseif ("_hwCommon" IN_LIST PATH_LIST)
        list(APPEND GENGLSL_FILES ${RELATIVE_FILE})
        list(APPEND GENMSL_FILES ${RELATIVE_FILE})
    else ()
        list(APPEND COMMON_FILES ${RELATIVE_FILE})
    endif ()
endforeach ()

# Stage each file set in to the build location
set(ALL_INSTALLED_LIBRARY_FILES "")

mx_copy_files(COMMON_FILES ${LIBRARIES_BUILD_FOLDER}
        INSTALLED_FILES_LIST ALL_INSTALLED_LIBRARY_FILES)

if (MATERIALX_BUILD_GEN_OSL)
    if (MATERIALX_OSL_LEGACY_CLOSURES)
        list(APPEND GENOSL_FILES "pbrlib/genosl/pbrlib_genosl_impl.legacy")
    else ()
        list(APPEND GENOSL_FILES "pbrlib/genosl/pbrlib_genosl_impl.mtlx")
    endif ()
    mx_copy_files(GENOSL_FILES ${LIBRARIES_BUILD_FOLDER}
            SRC_DEST_REPLACE_RULE ".legacy;.mtlx"
            INSTALLED_FILES_LIST ALL_INSTALLED_LIBRARY_FILES)
endif ()

if (MATERIALX_BUILD_GEN_MDL)
    mx_copy_files(GENMDL_FILES ${LIBRARIES_BUILD_FOLDER}
            INSTALLED_FILES_LIST ALL_INSTALLED_LIBRARY_FILES)
endif ()

if (MATERIALX_BUILD_GEN_GLSL)
    mx_copy_files(GENGLSL_FILES ${LIBRARIES_BUILD_FOLDER}
            SRC_DEST_REPLACE_RULE "/_hwCommon/;/genglsl/"
            INSTALLED_FILES_LIST ALL_INSTALLED_LIBRARY_FILES)
endif ()

if (MATERIALX_BUILD_GEN_MSL)
    mx_copy_files(GENMSL_FILES ${LIBRARIES_BUILD_FOLDER}
            SRC_DEST_REPLACE_RULE "/_hwCommon/;/genmsl/"
            INSTALLED_FILES_LIST ALL_INSTALLED_LIBRARY_FILES)
endif ()

add_custom_target(MaterialX_Library_Files ALL
        DEPENDS ${ALL_INSTALLED_LIBRARY_FILES})

## Finally install the staged files
if(NOT SKBUILD)
    install(DIRECTORY ${LIBRARIES_BUILD_FOLDER}/
            DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}")
endif()

if (MATERIALX_BUILD_PYTHON)
    set(MATERIALX_PYTHON_LIBRARIES_PATH "${MATERIALX_PYTHON_FOLDER_NAME}/${MATERIALX_INSTALL_STDLIB_PATH}")
    if (SKBUILD)
        set(MATERIALX_PYTHON_LIBRARIES_PATH "${SKBUILD_PLATLIB_DIR}/MaterialX/libraries")
    endif ()

    install(DIRECTORY ${LIBRARIES_BUILD_FOLDER}/
            DESTINATION "${MATERIALX_PYTHON_LIBRARIES_PATH}")
endif()
