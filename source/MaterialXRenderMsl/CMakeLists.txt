set(MATERIALX_MODULE_NAME MaterialXRenderMsl)

if (MATERIALX_BUILD_MONOLITHIC)
    set(TARGET_NAME ${MATERIALX_MONOLITHIC_TARGET})
else()
    set(TARGET_NAME ${MATERIALX_MODULE_NAME})
endif()

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.m*")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(APPLE)
    if(NOT MATERIALX_BUILD_IOS)
        find_library(COCOA_FRAMEWORK Cocoa)
        find_package(OpenGL REQUIRED)
    endif()
    file(GLOB_RECURSE materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m")
    message("Objective C files: " ${materialx_source_oc})
    set_source_files_properties(${materialx_source_oc} PROPERTIES
        COMPILE_FLAGS "-x objective-c++")
    set(materialx_source ${materialx_source} ${materialx_source_oc})
elseif(UNIX)
    find_package(X11 REQUIRED)
    # Note - can't just require the Xt component because FindX11 in cmake 3.1
    # doesn't support it
    if(NOT X11_Xt_FOUND)
        message(FATAL_ERROR "Error in building MaterialXRenderMsl: Xt was not found")
    endif()

    find_package(OpenGL REQUIRED)
endif()


if (NOT MATERIALX_BUILD_MONOLITHIC)
    add_library(${MATERIALX_MODULE_NAME} ${materialx_source} ${materialx_headers})

    target_link_libraries(
            ${MATERIALX_MODULE_NAME}
            MaterialXRenderHw
            MaterialXGenMsl
            ${CMAKE_DL_LIBS})

    set_target_properties(
            ${MATERIALX_MODULE_NAME} PROPERTIES
            OUTPUT_NAME ${MATERIALX_MODULE_NAME}${MATERIALX_LIBNAME_SUFFIX}
            COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
            LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
            VERSION "${MATERIALX_LIBRARY_VERSION}"
            SOVERSION "${MATERIALX_MAJOR_VERSION}")

    target_include_directories(${MATERIALX_MODULE_NAME}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
            $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
            PRIVATE
            ${EXTERNAL_INCLUDE_DIRS})

else()
    target_sources(${MATERIALX_MONOLITHIC_TARGET} PRIVATE ${materialx_source})
    target_sources(${MATERIALX_MONOLITHIC_TARGET} PRIVATE ${materialx_headers})

    add_library(${MATERIALX_MODULE_NAME} ALIAS ${MATERIALX_MONOLITHIC_TARGET})
    list(APPEND MATERIALX_MODULES ${MATERIALX_MODULE_NAME})
    set(MATERIALX_MODULES ${MATERIALX_MODULES} PARENT_SCOPE)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE -DMATERIALX_RENDERMSL_EXPORTS)

if(APPLE)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DGL_SILENCE_DEPRECATION)
elseif(UNIX)
    target_include_directories(${TARGET_NAME} PRIVATE ${X11_INCLUDE_DIR})
endif()

# Disable OpenGL deprecation warnings on Clang.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(
    ${TARGET_NAME}
    MaterialXRenderHw
    MaterialXGenMsl
    ${CMAKE_DL_LIBS})

if(MSVC)
    target_link_libraries(
        ${TARGET_NAME}
        Opengl32)
elseif(APPLE)
    if(NOT MATERIALX_BUILD_IOS)
        target_link_libraries(
            ${TARGET_NAME}
            "-framework Cocoa"
            "-framework OpenGL")
    endif()
    target_link_libraries(
        ${TARGET_NAME}
        "-framework Foundation"
        "-framework Metal")
elseif(UNIX)
    target_link_libraries(
        ${TARGET_NAME}
        OpenGL::GL
        X11::X11
        X11::Xt)
endif()

if(NOT SKBUILD)
    if (NOT MATERIALX_BUILD_MONOLITHIC)
        install(TARGETS ${MATERIALX_MODULE_NAME}
            EXPORT MaterialX
            ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
            LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
            RUNTIME DESTINATION bin)
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH}/${MATERIALX_MODULE_NAME}/ MESSAGE_NEVER
        FILES_MATCHING PATTERN "*.h*")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${MATERIALX_MODULE_NAME}.pdb"
        DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
endif()
