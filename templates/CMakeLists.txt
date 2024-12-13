
# This is a temporary validation step - so I decided not to promote this setting
# to a first class CMake option for now - we may decide we want to do this if
# running this in CI is helpful -
set(MATERIALX_COMPARE_TEMPLATES_TO_LIBRARY OFF)

if(MATERIALX_COMPARE_TEMPLATES_TO_LIBRARY)
    # Here we run both the "libraries" and "templates" folder through the
    # build library tool - this is because the tool reformats the files, due to the
    # XML writer, this makes diffing the results harder
    # We run the "libraries" folder through the build tool so that is gets
    # reformatted, no other translation happens here - because there are no
    # named constants or template elements present.
    # Next we run the "templates" folder through the build - the output is formatted
    # similarly to the processed "libraries" folder, but with all named constants
    # and template elements expanded.
    # The two output directories can then be easily diffed to confirm the templates
    # have been expanded as expected.
    # Finally we diff the two built folders to report any differences.

    set(DATA_LIBRARY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libraries)
    set(PROCESSED_DATA_LIBRARY_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/DataLibraryBuild)

    file(GLOB_RECURSE MATERIALX_DATA_LIBRARY_MTLX_SOURCE_FILES
            RELATIVE ${DATA_LIBRARY_SOURCE_DIR}
            LIST_DIRECTORIES false
            *.mtlx)

    foreach(SOURCE_FILE IN LISTS MATERIALX_DATA_LIBRARY_MTLX_SOURCE_FILES)
        set(DEST_FILEPATH ${PROCESSED_DATA_LIBRARY_BUILD_DIR}/${SOURCE_FILE})
        list(APPEND MATERIALX_PROCESSED_DATA_LIBRARY_MTLX_BUILD_FILES ${DEST_FILEPATH})
    endforeach()

    add_custom_command(
        OUTPUT ${MATERIALX_PROCESSED_DATA_LIBRARY_MTLX_BUILD_FILES}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROCESSED_DATA_LIBRARY_BUILD_DIR}
        COMMAND ${_MaterialXBuildLibrary} --sourceLibraryRoot ${DATA_LIBRARY_SOURCE_DIR} --destLibraryRoot ${PROCESSED_DATA_LIBRARY_BUILD_DIR} --expandTemplateElems --bakeNamedValues
        DEPENDS ${MATERIALX_DATA_LIBRARY_MTLX_SOURCE_FILES} MaterialXBuildLibrary
    )

    add_custom_target(MaterialXProcessDataLibrary ALL
        DEPENDS ${MATERIALX_PROCESSED_DATA_LIBRARY_MTLX_BUILD_FILES})


    set(TEMPLATE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/templates)
    set(PROCESSED_TEMPLATE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/TemplateBuild)

    file(GLOB_RECURSE MATERIALX_TEMPLATE_MTLX_SOURCE_FILES
            RELATIVE ${TEMPLATE_SOURCE_DIR}
            LIST_DIRECTORIES false
            *.mtlx)

    foreach(SOURCE_FILE IN LISTS MATERIALX_TEMPLATE_MTLX_SOURCE_FILES)
        set(DEST_FILEPATH ${PROCESSED_TEMPLATE_BUILD_DIR}/${SOURCE_FILE})
        list(APPEND MATERIALX_PROCESSED_TEMPLATE_MTLX_BUILD_FILES ${DEST_FILEPATH})
    endforeach()

    add_custom_command(
            OUTPUT ${MATERIALX_PROCESSED_TEMPLATE_MTLX_BUILD_FILES}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROCESSED_TEMPLATE_BUILD_DIR}
            COMMAND ${_MaterialXBuildLibrary} --sourceLibraryRoot ${TEMPLATE_SOURCE_DIR} --destLibraryRoot ${PROCESSED_TEMPLATE_BUILD_DIR} --expandTemplateElems --bakeNamedValues
            DEPENDS ${MATERIALX_TEMPLATE_MTLX_SOURCE_FILES} MaterialXBuildLibrary
    )

    add_custom_target(MaterialXProcessTemplate ALL
            DEPENDS ${MATERIALX_PROCESSED_TEMPLATE_MTLX_BUILD_FILES})

    add_custom_target(MaterialXProcessDataLibraryAndTemplates ALL
            DEPENDS MaterialXProcessTemplate MaterialXProcessDataLibrary
    )

    set(DIFF_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/diffReport.txt)

    add_custom_command(
            OUTPUT ${DIFF_OUTPUT_FILE}
            COMMAND diff -r ${PROCESSED_DATA_LIBRARY_BUILD_DIR} ${PROCESSED_TEMPLATE_BUILD_DIR} | tee ${DIFF_OUTPUT_FILE}
            DEPENDS MaterialXProcessDataLibraryAndTemplates
    )

    add_custom_target(MaterialXDiffTemplateAndDataLibrary ALL
            DEPENDS ${DIFF_OUTPUT_FILE}
    )

endif()
