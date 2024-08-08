add_subdirectory(External/PugiXML)
set_property(TARGET pugixml PROPERTY FOLDER "External")

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXFormat
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    LIBRARIES
        MaterialXCore
    PRIVATE_LIBRARIES
        "$<BUILD_INTERFACE:pugixml::pugixml>"
    EXPORT_DEFINE
        MATERIALX_FORMAT_EXPORTS)

# we need to use BUILD_INTERFACE here to hide the use of pugixml from the cmake export when we're building
# MaterialX as a static library - feels like maybe a cmake bug?
