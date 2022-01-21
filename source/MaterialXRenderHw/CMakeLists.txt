file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa)
  file(GLOB materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m")
  message("Objective C files: " ${materialx_source_oc})
  set_source_files_properties(${materialx_source_oc} PROPERTIES
                              COMPILE_FLAGS "-x objective-c++")
  set(materialx_source ${materialx_source} ${materialx_source_oc})
elseif(UNIX)
  find_package(X11 REQUIRED)
  # Note - can't just require the Xt component because FindX11 in cmake 3.1
  # doesn't support it
  if(NOT X11_Xt_FOUND)
    message(FATAL_ERROR "Error in building MaterialXRenderHw: Xt was not found")
  endif()
endif()

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

add_library(MaterialXRenderHw ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_RENDERHW_EXPORTS)

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
endif()

set_target_properties(
    MaterialXRenderHw PROPERTIES
    OUTPUT_NAME MaterialXRenderHw${MATERIALX_LIBNAME_SUFFIX}
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_include_directories(MaterialXRenderHw
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS})

install(TARGETS MaterialXRenderHw
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXRenderHw/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderHw.pdb"
    DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
