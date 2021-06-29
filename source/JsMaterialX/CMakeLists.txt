set(CORE ${CMAKE_CURRENT_SOURCE_DIR}/JsMaterialXCore/)
set(FORMAT ${CMAKE_CURRENT_SOURCE_DIR}/JsMaterialXFormat/)

set(SOURCE_FOLDER ${CMAKE_SOURCE_DIR}/source)

set(CORE_DEPS ${CORE}JsTypes.cpp
    ${CORE}JsMaterial.cpp
    ${CORE}JsUnit.cpp
    ${CORE}JsExceptions.cpp
    ${CORE}JsUtil.cpp
    ${CORE}JsElement.cpp
    ${CORE}JsInterface.cpp
    ${CORE}JsNode.cpp
    ${CORE}JsDefinition.cpp
    ${CORE}JsDocument.cpp
    ${CORE}JsLook.cpp
    ${CORE}JsProperty.cpp
    ${CORE}JsGeom.cpp
    ${CORE}JsVariant.cpp
    ${CORE}JsValue.cpp
    ${CORE}JsTraversal.cpp)

set(FORMAT_DEPS
    ${FORMAT}JsUtil.cpp
    ${FORMAT}JsXmlIo.cpp
    ${FORMAT}JsXmlExport.cpp
    ${FORMAT}JsEnviron.cpp)

# Linker flags
set(JS_LINK_FLAGS "")
if (EXTERNAL_LINK_FLAGS)
    string(APPEND JS_LINK_FLAGS "${EXTERNAL_LINK_FLAGS} ")
endif()

string(APPEND JS_LINK_FLAGS " -I${SOURCE_FOLDER} \
    -s WASM=1 \
    -s DISABLE_EXCEPTION_CATCHING=0 \
    -s MODULARIZE=1 \
    -s USE_ES6_IMPORT_META=0 \
    -s MEMORY_GROWTH_LINEAR_STEP=32MB \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s FORCE_FILESYSTEM=1 ")

# Add the post javascript files
string(APPEND JS_LINK_FLAGS "--post-js ${CMAKE_CURRENT_SOURCE_DIR}/post.js \
 --post-js ${CORE}post.js \
 --post-js ${FORMAT}post.js ")

# Compiler flags
set(JS_COMPILE_FLAGS "")
if (EXTERNAL_COMPILE_FLAGS)
    string(APPEND JS_COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS} ")
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("JS: Building Debug")
    string(APPEND JS_LINK_FLAGS "--bind ")
else()
    message("JS: Building Release")
    string(APPEND JS_LINK_FLAGS "-flto -Os --bind ")
    string(APPEND JS_COMPILE_FLAGS "-flto -Os ")
endif()

add_executable(JsMaterialX MaterialXLib.cpp
    ${CORE_DEPS}
    ${FORMAT_DEPS})

set_target_properties(JsMaterialX 
    PROPERTIES
    OUTPUT_NAME JsMaterialX
    COMPILE_FLAGS "${JS_COMPILE_FLAGS}"
    LINK_FLAGS "${JS_LINK_FLAGS}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(JsMaterialX
    PUBLIC MaterialXCore
    PUBLIC MaterialXFormat
    PRIVATE ${CMAKE_DL_LIBS})

# Install the JavaScript output
install(TARGETS JsMaterialX DESTINATION "${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX")

# Copy the wasm output
install(FILES ${CMAKE_BINARY_DIR}/bin/JsMaterialX.wasm DESTINATION ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/)

# Copy the JavaScript files with the version appended to them.
install(FILES ${CMAKE_BINARY_DIR}/bin/JsMaterialX.js DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ RENAME JsMaterialX-${MATERIALX_LIBRARY_VERSION}.js)
install(FILES ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/JsMaterialX.js DESTINATION ${CMAKE_INSTALL_PREFIX}/JavaScript/MaterialX/ RENAME JsMaterialX-${MATERIALX_LIBRARY_VERSION}.js)
