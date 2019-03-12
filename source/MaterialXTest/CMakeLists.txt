include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
if(NOT MATERIALX_BUILD_RENDER)
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/Render.cpp")
endif(NOT MATERIALX_BUILD_RENDER)
if(NOT MATERIALX_BUILD_GEN_OGSFX)
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/GenOgsfx.cpp")
endif(NOT MATERIALX_BUILD_GEN_OGSFX)

file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE catch_headers "${CMAKE_CURRENT_SOURCE_DIR}/Catch/*.hpp")

function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})
source_group("Header Files\\Catch" FILES ${catch_headers})

add_executable(MaterialXTest ${materialx_source} ${materialx_header} ${catch_headers})

add_test(NAME MaterialXTest
         COMMAND MaterialXTest
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../resources" DESTINATION ${CMAKE_CURRENT_BINARY_DIR} MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/Examples" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/documents MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/Libraries" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/documents MESSAGE_NEVER)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../documents/TestSuite" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/documents MESSAGE_NEVER)

set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

set (LIBS
    MaterialXCore
    MaterialXFormat
    MaterialXGenShader
    MaterialXGenOsl
    MaterialXGenGlsl
)

# Add language specific generators
if (MATERIALX_BUILD_GEN_OGSFX)
    LIST(APPEND LIBS "MaterialXGenOgsFx")
endif(MATERIALX_BUILD_GEN_OGSFX)

# Add render libraries
if (MATERIALX_BUILD_RENDER)
    LIST(APPEND LIBS "MaterialXRender")
    if (MATERIALX_BUILD_RENDEROSL)
        LIST(APPEND LIBS "MaterialXRenderOsl")
    endif()
    if (MATERIALX_BUILD_RENDERGLSL)
        LIST(APPEND LIBS "MaterialXRenderHw")
        LIST(APPEND LIBS "MaterialXRenderGlsl")
    endif()
endif(MATERIALX_BUILD_RENDER)

# Add contrib libraries
if (MATERIALX_BUILD_CONTRIB)
    LIST(APPEND LIBS "MaterialXContrib")
endif(MATERIALX_BUILD_CONTRIB)

target_link_libraries(
    MaterialXTest ${LIBS}
    ${CMAKE_DL_LIBS}
)
