
# Shared functions
function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

function(mx_add_library MATERIALX_MODULE_NAME)
    set(options ADD_OBJECTIVE_C_CODE)
    set(oneValueArgs EXPORT_DEFINE)
    set(multiValueArgs
            SOURCE_FILES
            HEADER_FILES
            INLINED_FILES
            MTLX_MODULES)
    cmake_parse_arguments(args
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    if (APPLE AND args_ADD_OBJECTIVE_C_CODE)
        file(GLOB_RECURSE materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m*")
        set_source_files_properties(${materialx_source_oc} PROPERTIES
                COMPILE_FLAGS "-x objective-c++")
        set(args_SOURCE_FILES ${args_SOURCE_FILES} ${materialx_source_oc})
    endif()

    assign_source_group("Source Files" ${args_SOURCE_FILES})
    assign_source_group("Source Files" ${args_INLINED_FILES})
    assign_source_group("Header Files" ${args_HEADER_FILES})

    if (NOT MATERIALX_BUILD_MONOLITHIC)
        set(TARGET_NAME ${MATERIALX_MODULE_NAME})
        add_library(${TARGET_NAME})

        # Create version resource
        if(MATERIALX_BUILD_SHARED_LIBS AND MSVC)
            configure_file(${PROJECT_SOURCE_DIR}/cmake/modules/MaterialXVersion.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
            target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        endif()

        target_link_libraries(${TARGET_NAME}
                PUBLIC
                ${args_MTLX_MODULES}
                ${CMAKE_DL_LIBS})

        target_include_directories(${TARGET_NAME}
                PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
                $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
                PRIVATE
                ${EXTERNAL_INCLUDE_DIRS})

        set_target_properties(
                ${TARGET_NAME} PROPERTIES
                OUTPUT_NAME ${MATERIALX_MODULE_NAME}${MATERIALX_LIBNAME_SUFFIX}
                COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
                LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
                INSTALL_RPATH "${MATERIALX_SAME_DIR_RPATH}"
                VERSION "${MATERIALX_LIBRARY_VERSION}"
                SOVERSION "${MATERIALX_MAJOR_VERSION}")
    else()
        set(TARGET_NAME ${MATERIALX_MONOLITHIC_TARGET})

        add_library(${MATERIALX_MODULE_NAME} ALIAS ${MATERIALX_MONOLITHIC_TARGET})

        # Store the aliased MaterialX modules name to create cmake export aliases later.
        set_property(GLOBAL APPEND PROPERTY MATERIALX_MODULES ${MATERIALX_MODULE_NAME})
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    target_sources(${TARGET_NAME}
            PRIVATE
            ${args_SOURCE_FILES}
            PUBLIC
            FILE_SET
            mxHeaders
            TYPE
            HEADERS
            BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_BINARY_DIR}/..
            FILES
            ${args_HEADER_FILES}
            ${args_INLINED_FILES})

    target_include_directories(${TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>)

    target_compile_definitions(${TARGET_NAME} PRIVATE "-D${args_EXPORT_DEFINE}")

    if(NOT SKBUILD)
        if(NOT MATERIALX_BUILD_MONOLITHIC)
            install(TARGETS ${MATERIALX_MODULE_NAME}
                    EXPORT MaterialX
                    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
                    RUNTIME DESTINATION bin
                    FILE_SET mxHeaders DESTINATION ${MATERIALX_INSTALL_INCLUDE_PATH})
        endif()

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${MATERIALX_MODULE_NAME}.pdb"
                DESTINATION "${MATERIALX_INSTALL_LIB_PATH}/" OPTIONAL)
    endif()

    # pass TARGET_NAME back to call site - so caller can modify the build target
    set(TARGET_NAME ${TARGET_NAME} PARENT_SCOPE)
endfunction()
