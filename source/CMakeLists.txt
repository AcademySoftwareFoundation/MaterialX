if (MATERIALX_BUILD_MONOLITHIC)
    set(MATERIALX_MODULE_NAME ${MATERIALX_MONOLITHIC_TARGET})

    # No sources are added - this will be added from each separate module, if the options are set
    # such that the individual module would have been built if not in monolithic build mode
    add_library(${MATERIALX_MODULE_NAME} "" "" )

    set_target_properties(${MATERIALX_MODULE_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${MATERIALX_MODULE_NAME} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    # Create version resource
    if(MATERIALX_BUILD_SHARED_LIBS AND MSVC)
        configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/MaterialXVersion.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        target_sources(${MATERIALX_MODULE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()

    set_target_properties(${MATERIALX_MODULE_NAME} PROPERTIES
            OUTPUT_NAME ${MATERIALX_MODULE_NAME}${MATERIALX_LIBNAME_SUFFIX}
            COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
            LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
            INSTALL_RPATH "${MATERIALX_SAME_DIR_RPATH}"
            VERSION "${MATERIALX_LIBRARY_VERSION}"
            SOVERSION "${MATERIALX_MAJOR_VERSION}")

    target_link_libraries(${MATERIALX_MODULE_NAME}
            PUBLIC
            ${CMAKE_DL_LIBS})

    target_include_directories(${MATERIALX_MODULE_NAME}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
            $<INSTALL_INTERFACE:${MATERIALX_INSTALL_INCLUDE_PATH}>
            PRIVATE
            ${EXTERNAL_INCLUDE_DIRS})

    # Monolithic target is installed in the root CMakeLists.txt to allow for collection
    # of all necessary headers.
endif()
