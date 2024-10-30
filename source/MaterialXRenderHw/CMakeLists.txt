
file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if(APPLE)
    if (NOT MATERIALX_BUILD_APPLE_EMBEDDED)
        find_library(COCOA_FRAMEWORK Cocoa)
    endif()
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
    MTLX_MODULES
        MaterialXRender
    EXPORT_DEFINE
        MATERIALX_RENDERHW_EXPORTS
    ADD_OBJECTIVE_C_CODE)

if(APPLE)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            "-framework Foundation"
            "-framework Metal")
    if (NOT MATERIALX_BUILD_APPLE_EMBEDDED)
        target_link_libraries(${TARGET_NAME}
                PUBLIC
                "-framework Cocoa"
        )
    endif()
elseif(UNIX)
    target_link_libraries(${TARGET_NAME}
            PUBLIC
            X11::X11
            X11::Xt)
endif()
