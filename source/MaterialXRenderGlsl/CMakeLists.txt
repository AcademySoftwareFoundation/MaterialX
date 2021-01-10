file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_package(OpenGL REQUIRED)
    file(GLOB_RECURSE materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m")
    message("Objective C files: " ${materialx_source_oc})
    set_source_files_properties(${materialx_source_oc} PROPERTIES
        COMPILE_FLAGS "-x objective-c++")
    set(materialx_source ${materialx_source} ${materialx_source_oc})
    add_compile_options(-Wno-deprecated-declarations -DGL_SILENCE_DEPRECATION)
elseif(UNIX)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    if(NOT X11_Xt_FOUND)
        message("Error in building MaterialXRender: Xt was not found")
    endif()
    include_directories(${X11_INCLUDE_DIR})
endif()

add_library(MaterialXRenderGlsl ${materialx_source} ${materialx_headers})

if(MSVC)
    target_link_libraries(
        MaterialXRenderGlsl
        MaterialXRenderHw
        MaterialXGenGlsl
        Opengl32
        ${CMAKE_DL_LIBS})
elseif(APPLE)
    target_link_libraries(
        MaterialXRenderGlsl
        MaterialXRenderHw
        MaterialXGenGlsl
        ${CMAKE_DL_LIBS}
        ${OPENGL_LIBRARIES}
        "-framework Foundation"
        "-framework Cocoa")
elseif(UNIX)
    target_link_libraries(
        MaterialXRenderGlsl
        MaterialXRenderHw
        MaterialXGenGlsl
        ${CMAKE_DL_LIBS}
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xt_LIB})
endif()

set_target_properties(
    MaterialXRenderGlsl PROPERTIES
    OUTPUT_NAME MaterialXRenderGlsl
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_include_directories(MaterialXRenderGlsl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
    PRIVATE
        ${EXTERNAL_INCLUDE_DIRS})

install(TARGETS MaterialXRenderGlsl
    EXPORT MaterialX
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH})

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXRenderGlsl/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderGlsl.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
