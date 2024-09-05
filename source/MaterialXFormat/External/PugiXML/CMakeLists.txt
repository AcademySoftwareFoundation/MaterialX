# Don't actually build a library for PugiXML as if we are building MaterialX as a static library
# then we would also need to ship the PugiXML library, instead we just compile the sources
# directly into MaterialXFormat, but take care to hide the symbols.

file(GLOB pugixml_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB pugixml_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set_source_files_properties(${pugixml_source} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_source_files_properties(${pugixml_source} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set_source_files_properties(${pugixml_source} PROPERTIES FOLDER External)
set_source_files_properties(${pugixml_headers} PROPERTIES FOLDER External)

set(pugixml_source ${pugixml_source} PARENT_SCOPE)
set(pugixml_headers ${pugixml_headers} PARENT_SCOPE)

