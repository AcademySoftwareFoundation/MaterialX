include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
if(NOT MATERIALX_BUILD_RENDER)
    list(REMOVE_ITEM materialx_source "MaterialXValid.cpp")
endif(NOT MATERIALX_BUILD_RENDER)
file(GLOB_RECURSE catch_headers "${CMAKE_CURRENT_SOURCE_DIR}/Catch/*.hpp")

source_group("Header Files\\Catch" FILES ${catch_headers})

add_executable(MaterialXTest ${materialx_source} ${catch_headers})

add_test(NAME MaterialXTest
         COMMAND MaterialXTest
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/Libraries ${CMAKE_CURRENT_BINARY_DIR}/documents/Libraries)

add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/Examples ${CMAKE_CURRENT_BINARY_DIR}/documents/Examples)

if (MATERIALX_BUILD_RENDER)
  add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/Images ${CMAKE_CURRENT_BINARY_DIR}/documents/Images)
  add_custom_command(TARGET MaterialXTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/documents/TestSuite ${CMAKE_CURRENT_BINARY_DIR}/documents/TestSuite)
endif(MATERIALX_BUILD_RENDER)
                   
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
)
if (MATERIALX_BUILD_GEN_OSL)
    LIST(APPEND LIBS "MaterialXGenOsl")
endif(MATERIALX_BUILD_GEN_OSL)
if (MATERIALX_BUILD_GEN_GLSL)
    LIST(APPEND LIBS "MaterialXGenGlsl")
endif(MATERIALX_BUILD_GEN_GLSL)
if (MATERIALX_BUILD_GEN_OGSFX)
    LIST(APPEND LIBS "MaterialXGenOgsFx")
endif(MATERIALX_BUILD_GEN_OGSFX)
if (MATERIALX_BUILD_RENDER)
    LIST(APPEND LIBS "MaterialXRender")
endif(MATERIALX_BUILD_RENDER)
if (MATERIALX_BUILD_CONTRIB)
    LIST(APPEND LIBS "MaterialXContrib")
endif(MATERIALX_BUILD_CONTRIB)
target_link_libraries(
    MaterialXTest ${LIBS}    
    ${CMAKE_DL_LIBS}
)
