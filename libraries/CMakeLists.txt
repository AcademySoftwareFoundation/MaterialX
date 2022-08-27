install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}" MESSAGE_NEVER
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "pbrlib_genosl_impl.legacy" EXCLUDE)

if (MATERIALX_OSL_LEGACY_CLOSURES)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pbrlib/genosl/pbrlib_genosl_impl.legacy"
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}/pbrlib/genosl/" RENAME pbrlib_genosl_impl.mtlx)
endif()
