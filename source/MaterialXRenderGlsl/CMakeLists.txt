file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.c*")
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
    add_compile_options(-DGL_SILENCE_DEPRECATION)
elseif(UNIX)
    find_package(X11 REQUIRED)
    # Note - can't just require the Xt component because FindX11 in cmake 3.1
    # doesn't support it
    if(NOT X11_Xt_FOUND)
        message(FATAL_ERROR "Error in building MaterialXRenderGlsl: Xt was not found")
    endif()

    find_package(OpenGL REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
endif()

# Disable OpenGL deprecation warnings on Clang.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-declarations)
endif()

add_library(MaterialXRenderGlsl ${materialx_source} ${materialx_headers})

add_definitions(-DMATERIALX_RENDERGLSL_EXPORTS)

set(COMMON_LIBRARIES
    MaterialXRenderHw
    MaterialXGenGlsl
    ${CMAKE_DL_LIBS})

if(MSVC)
    target_link_libraries(
        MaterialXRenderGlsl
        ${COMMON_LIBRARIES}
        Opengl32)
elseif(APPLE)
    target_link_libraries(
        MaterialXRenderGlsl
        ${COMMON_LIBRARIES}
        ${OPENGL_LIBRARIES}
        "-framework Foundation"
        "-framework Cocoa")
elseif(UNIX)
    target_link_libraries(
        MaterialXRenderGlsl
        ${COMMON_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xt_LIB})
endif()

set_target_properties(
    MaterialXRenderGlsl PROPERTIES
    OUTPUT_NAME MaterialXRenderGlsl${MATERIALX_LIBNAME_SUFFIX}
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
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    RUNTIME DESTINATION bin)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/MaterialXRenderGlsl/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderGlsl.pdb"
    DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
