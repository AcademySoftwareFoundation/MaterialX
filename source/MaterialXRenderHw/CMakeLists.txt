
file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(APPLE)
    if (NOT MATERIALX_BUILD_IOS)
        find_library(COCOA_FRAMEWORK Cocoa)
    endif()
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


mx_add_library(MaterialXRenderHw
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    EXPORT_DEFINE
        MATERIALX_RENDERHW_EXPORTS
)

if(MSVC)
    target_link_libraries(
            ${TARGET_NAME}
            MaterialXRender
            ${CMAKE_DL_LIBS})
elseif(APPLE)
    target_link_libraries(
            ${TARGET_NAME}
            MaterialXRender
            ${CMAKE_DL_LIBS}
            "-framework Foundation"
            "-framework Metal")
    if (NOT MATERIALX_BUILD_IOS)
        target_link_libraries(
                ${TARGET_NAME}
                "-framework Cocoa"
        )
    endif()
elseif(UNIX)
    target_link_libraries(
            ${TARGET_NAME}
            MaterialXRender
            ${CMAKE_DL_LIBS}
            X11::X11
            X11::Xt)
endif()
