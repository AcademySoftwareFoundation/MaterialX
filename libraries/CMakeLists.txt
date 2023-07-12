install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}" MESSAGE_NEVER
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "pbrlib_genosl_impl.*" EXCLUDE)

if (MATERIALX_OSL_LEGACY_CLOSURES)
    set(PBRLIB_SUFFIX "legacy")
else()
    set(PBRLIB_SUFFIX "mtlx")
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pbrlib/genosl/pbrlib_genosl_impl.${PBRLIB_SUFFIX}"
        DESTINATION "${MATERIALX_INSTALL_STDLIB_PATH}/pbrlib/genosl/" RENAME pbrlib_genosl_impl.mtlx)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION "python/MaterialX/${MATERIALX_INSTALL_STDLIB_PATH}" 
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "pbrlib_genosl_impl.*" EXCLUDE)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pbrlib/genosl/pbrlib_genosl_impl.${PBRLIB_SUFFIX}"
        DESTINATION "python/MaterialX/${MATERIALX_INSTALL_STDLIB_PATH}/pbrlib/genosl/" RENAME pbrlib_genosl_impl.mtlx)
