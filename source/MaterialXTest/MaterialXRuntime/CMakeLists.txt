file(GLOB_RECURSE source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(MaterialXTest PUBLIC ${source} ${headers})

foreach(src_file ${source})
    add_tests(${src_file})
endforeach()

assign_source_group("Source Files" ${source})
assign_source_group("Header Files" ${headers})
