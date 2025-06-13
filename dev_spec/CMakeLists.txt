

set(MATX_LIBRARIES_DIR ${PROJECT_SOURCE_DIR}/libraries)
set(DOCS_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/dev_spec/nodes)

set(MD_SRC_DIR ${PROJECT_SOURCE_DIR}/dev_spec/source)
set(MD_DEST_DIR ${PROJECT_SOURCE_DIR}/dev_spec/dest)

set(NODE_MD_MANIFEST ${DOCS_OUTPUT_DIR}/_manifest.txt)

file(GLOB_RECURSE MATERIALX_DATA_LIBRARY_MTLX_SOURCE_FILES
        LIST_DIRECTORIES false
        ${MATX_LIBRARIES_DIR}/*.mtlx)

file(GLOB_RECURSE MATERIALX_MD_SOURCE_FILES
        LIST_DIRECTORIES false
        ${MD_SRC_DIR}/*.md)

add_custom_command(
    OUTPUT ${NODE_MD_MANIFEST}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCS_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${MD_DEST_DIR}
    COMMAND MaterialXBuildDocs --sourceLibraryRoot ${MATX_LIBRARIES_DIR} --destDocRoot ${DOCS_OUTPUT_DIR} --sourceMDRoot  ${MD_SRC_DIR} --destMDRoot  ${MD_DEST_DIR} --manifestFile ${NODE_MD_MANIFEST}
    DEPENDS ${MATERIALX_DATA_LIBRARY_MTLX_SOURCE_FILES} ${MATERIALX_MD_SOURCE_FILES} MaterialXBuildDocs
)

add_custom_target(MaterialXBuildSpecNodeTables ALL
        DEPENDS ${NODE_MD_MANIFEST})



