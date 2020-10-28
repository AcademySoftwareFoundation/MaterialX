set(OUTPUT_CORE ${CMAKE_CURRENT_SOURCE_DIR}/JsMaterialXCore/)
set(OUTPUT_FORMAT ${CMAKE_CURRENT_SOURCE_DIR}/JsMaterialXFormat/)
set(OUTPUT_LIBS ${PROJECT_BINARY_DIR}/source/)

set(SOURCE_FOLDER ${CMAKE_SOURCE_DIR}/source)

set(CORE_DEPS ${OUTPUT_CORE}JsTypes.cpp
    ${OUTPUT_CORE}JsUnit.cpp
    ${OUTPUT_CORE}JsExceptions.cpp
    ${OUTPUT_CORE}JsUtil.cpp
    ${OUTPUT_CORE}JsElement.cpp
    ${OUTPUT_CORE}JsInterface.cpp
    ${OUTPUT_CORE}JsNode.cpp
    ${OUTPUT_CORE}JsDefinition.cpp
    ${OUTPUT_CORE}JsDocument.cpp
    ${OUTPUT_CORE}JsLook.cpp
    ${OUTPUT_CORE}JsProperty.cpp
    ${OUTPUT_CORE}JsGeom.cpp
    ${OUTPUT_CORE}JsVariant.cpp
    ${OUTPUT_CORE}JsValue.cpp
    ${OUTPUT_CORE}JsMaterial.cpp
    ${OUTPUT_CORE}JsRegisteredVectors.cpp
    ${OUTPUT_CORE}JsTraversal.cpp)

set(FORMAT_DEPS ${OUTPUT_FORMAT}JsXmlIo.cpp)

set(LIB_DEPS ${OUTPUT_LIBS}MaterialXCore/libMaterialXCore.a ${OUTPUT_LIBS}MaterialXFormat/libMaterialXFormat.a)

# Include the source directory
set(JS_FLAGS "-I${SOURCE_FOLDER} ")

# Add all of the required flags for emcc
string(APPEND JS_FLAGS "-s WASM=1 -s DISABLE_EXCEPTION_CATCHING=0 -s MODULARIZE=1 -s EXPORT_ES6=1 -s USE_ES6_IMPORT_META=0 -s MEMORY_GROWTH_LINEAR_STEP=32MB -s ALLOW_MEMORY_GROWTH=1 ")

# Add the post javascript core files 
string(APPEND JS_FLAGS "--post-js ${OUTPUT_CORE}JsTypes.js --post-js ${OUTPUT_CORE}JsUnit.js --post-js ${OUTPUT_CORE}JsGeom.js --post-js ${OUTPUT_CORE}JsDefinition.js --post-js ${OUTPUT_CORE}JsDocument.js --post-js ${OUTPUT_CORE}JsElement.js --post-js ${OUTPUT_CORE}JsInterface.js --post-js ${OUTPUT_CORE}JsLook.js --post-js ${OUTPUT_CORE}JsMaterial.js --post-js ${OUTPUT_CORE}JsNode.js --post-js ${OUTPUT_CORE}JsProperty.js --post-js ${OUTPUT_CORE}JsTraversal.js --post-js ${OUTPUT_CORE}JsUtil.js --post-js ${OUTPUT_CORE}JsValue.js --post-js ${OUTPUT_CORE}JsVariant.js ")

# Add the post javascript format files 
string(APPEND JS_FLAGS "--post-js ${OUTPUT_FORMAT}JsXmlIo.js --post-js ${CMAKE_CURRENT_SOURCE_DIR}/initMaterialX.js ")

if (EXTERNAL_LINK_FLAGS)
    string(APPEND JS_FLAGS "${EXTERNAL_LINK_FLAGS} ")
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("JS: Building Debug")
    string(APPEND JS_FLAGS "--bind ")
else()
    message("JS: Building Release")
    string(APPEND JS_FLAGS "-Os --bind ")
endif()

add_executable(JsMaterialX MaterialXLib.cpp)

set_target_properties(JsMaterialX 
    PROPERTIES
    OUTPUT_NAME JsMaterialX
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${JS_FLAGS}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(JsMaterialX
    PUBLIC ${CORE_DEPS}
    PUBLIC ${FORMAT_DEPS} 
    PUBLIC ${LIB_DEPS}
    PRIVATE ${CMAKE_DL_LIBS})

# Install the JavaScript output
install(TARGETS JsMaterialX DESTINATION "${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX")

# Copy the wasm output
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/JsMaterialX.wasm DESTINATION ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/)

# Copy the JavaScript files with the version appended to them.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/JsMaterialX.js DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ RENAME JsMaterialX-${MATERIALX_LIBRARY_VERSION}.js)
install(FILES ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/JsMaterialX.js DESTINATION ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/ RENAME JsMaterialX-${MATERIALX_LIBRARY_VERSION}.js)
