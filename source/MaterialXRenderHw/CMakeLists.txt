include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

if(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa)
  file(GLOB materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m")
  message("Objective C files: " ${materialx_source_oc})
  set_source_files_properties(${materialx_source_oc} PROPERTIES
                              COMPILE_FLAGS "-x objective-c++")
  set(materialx_source ${materialx_source} ${materialx_source_oc})
endif(APPLE)

add_library(MaterialXRenderHw STATIC
    ${materialx_source}
    ${materialx_header}
)

if(MSVC)
    target_link_libraries(
      MaterialXRenderHw
      MaterialXRender
      ${CMAKE_DL_LIBS})
elseif(APPLE)
    target_link_libraries(
      MaterialXRenderHw
      MaterialXRender
      ${CMAKE_DL_LIBS}
      "-framework Foundation"
      "-framework Cocoa")
elseif(UNIX)
    target_link_libraries(
      MaterialXRenderHw
      MaterialXRender
      ${CMAKE_DL_LIBS}
      ${X11_LIBRARIES}
      ${X11_Xt_LIB})
endif(MSVC)

set_target_properties(
    MaterialXRenderHw PROPERTIES
    OUTPUT_NAME MaterialXRenderHw
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

install(TARGETS MaterialXRenderHw
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXRenderHw/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderHw.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)
