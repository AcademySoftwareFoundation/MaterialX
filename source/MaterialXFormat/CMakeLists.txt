add_subdirectory(External/PugiXML)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXFormat
    SOURCE_FILES
        ${materialx_source}
        ${pugixml_source}
    HEADER_FILES
        ${materialx_headers}
        ${pugixml_headers}
    MTLX_MODULES
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_FORMAT_EXPORTS)
