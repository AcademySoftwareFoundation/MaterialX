set(MATERIALX_MODULE_NAME MaterialXFormat)

if (MATERIALX_BUILD_MONOLITHIC)
    set(TARGET_NAME ${MATERIALX_MONOLITHIC_TARGET})
else()
    set(TARGET_NAME ${MATERIALX_MODULE_NAME})
endif()

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

if (NOT MATERIALX_BUILD_MONOLITHIC)
    add_library(${MATERIALX_MODULE_NAME} ${materialx_source} ${materialx_headers})

    # Create version resource
    if(MATERIALX_BUILD_SHARED_LIBS AND MSVC)
        configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/MaterialXVersion.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        target_sources(${MATERIALX_MODULE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()

    set_target_properties(
            ${MATERIALX_MODULE_NAME} PROPERTIES
            OUTPUT_NAME ${MATERIALX_MODULE_NAME}${MATERIALX_LIBNAME_SUFFIX}
            COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
            LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
            INSTALL_RPATH "${MATERIALX_SAME_DIR_RPATH}"
            VERSION "${MATERIALX_LIBRARY_VERSION}"
            SOVERSION "${MATERIALX_MAJOR_VERSION}")

    target_link_libraries(
            ${MATERIALX_MODULE_NAME}
            MaterialXCore
            ${CMAKE_DL_LIBS})

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

target_compile_definitions(${TARGET_NAME} PRIVATE -DMATERIALX_FORMAT_EXPORTS)

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
