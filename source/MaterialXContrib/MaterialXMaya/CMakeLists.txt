project(MaterialXMaya)
set(MATERIALX_NODE_VERSION_MAJOR 0)
set(MATERIALX_NODE_VERSION_MINOR 1)
set(MATERIALX_NODE_BUILD_VERSION 0)

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(MATERIALX_NODE_VERSION "${MATERIALX_NODE_VERSION_MAJOR}.${MATERIALX_NODE_VERSION_MINOR}")
set(SOVERSION ${MATERIALX_NODE_VERSION_MAJOR} CACHE STRING "Set the SO version in the SO name of the output library")
set(MAYA_DEBUG_DIR "" CACHE STRING "Location of Maya debug installation.")
set(MAYA_RELEASE_DIR "" CACHE STRING "Location of Maya release installation.")

macro(DEPEND_MAYAAPI_INCLUDE)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        include_directories("${MAYA_DEBUG_DIR}/include" "${MAYA_DEBUG_DIR}/devkit/plug-ins")
    else()
        include_directories("${MAYA_RELEASE_DIR}/include" "${MAYA_RELEASE_DIR}/devkit/plug-ins")
    endif()
endmacro(DEPEND_MAYAAPI_INCLUDE)

macro(DEPEND_MAYAAPI_LINK targetname)
    set(LIBS
        "OpenMaya"
        "OpenMayaRender"
        "Foundation"
        "OpenMayaUI"
        "OpenMayaAnim"
        "OpenMayaFX"
    )
    if (${APPLE})
        add_definitions(-DOSMac_)
        set(MAYA_LIB_PREFIX "lib")
        set(MAYA_LIB_EXT "dylib")
        set(MAYA_LIB_DEBUG_DIR "${MAYA_DEBUG_DIR}/Maya.app/Contents/MacOS")
        set(MAYA_LIB_RELEASE_DIR "${MAYA_RELEASE_DIR}/Maya.app/Contents/MacOS")
    elseif(MSVC)
        add_definitions(-DNT_PLUGIN)
        set(MAYA_LIB_PREFIX "")
        set(MAYA_LIB_EXT "lib")
        set(MAYA_LIB_DEBUG_DIR "${MAYA_DEBUG_DIR}/lib")
        set(MAYA_LIB_RELEASE_DIR "${MAYA_RELEASE_DIR}/lib")
    else()
        add_definitions(-DLINUX)
        set(MAYA_LIB_PREFIX "lib")
        set(MAYA_LIB_EXT "so")
        set(MAYA_LIB_DEBUG_DIR "${MAYA_DEBUG_DIR}/lib")
        set(MAYA_LIB_RELEASE_DIR "${MAYA_RELEASE_DIR}/lib")
    endif()
    FOREACH(F ${LIBS})
        if (CMAKE_BUILD_TYPE MATCHES Debug)
            target_link_libraries(${targetname} debug "${MAYA_LIB_DEBUG_DIR}/${MAYA_LIB_PREFIX}${F}.${MAYA_LIB_EXT}")
        else()
            target_link_libraries(${targetname} optimized "${MAYA_LIB_RELEASE_DIR}/${MAYA_LIB_PREFIX}${F}.${MAYA_LIB_EXT}")
        endif()
    ENDFOREACH(F)
endmacro(DEPEND_MAYAAPI_LINK)

if (EXISTS "${MAYA_DEBUG_DIR}" OR EXISTS "${MAYA_RELEASE_DIR}")
    # Adjust compiler settings
    if(MSVC)
        if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
        endif()
    endif()

    file(GLOB _source  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB _headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    file(GLOB _scripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.py")
    file(GLOB _AEtemplate "${CMAKE_CURRENT_SOURCE_DIR}/AEtemplate/AE*Template.mel")
    file(GLOB _vp2ShaderFragments "${CMAKE_CURRENT_SOURCE_DIR}/vp2ShaderFragments/*.xml")
    file(GLOB _mods "${CMAKE_CURRENT_SOURCE_DIR}/mods/*.mod")

    source_group("Header Files" FILES ${_headers})
    source_group("Source Files" FILES ${_source})
    source_group("Scripts" FILES ${_scripts})
    source_group("AETemplates" FILES ${_AEtemplate})
    source_group("VP2ShaderFragments" FILES ${_vp2ShaderFragments})
    source_group("Mods" FILES ${_mods})

    add_library(${PROJECT_NAME} SHARED
        ${_source}
        ${_headers}
    )

    if (${APPLE})
        set(MAYA_PLUGIN_EXT ".bundle")
    elseif(MSVC)
        set(MAYA_PLUGIN_EXT ".mll")
    else()
        set(MAYA_PLUGIN_EXT ".so")
    endif()

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
        LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
        VERSION ${MATERIALX_NODE_VERSION}
        SOVERSION ${MATERIALX_NODE_VERSION}
        PREFIX ""
        SUFFIX ${MAYA_PLUGIN_EXT}
        DEBUG_POSTFIX ""
    )

    include_directories(
        ${EXTERNAL_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../source
    )
    DEPEND_MAYAAPI_INCLUDE()

    target_link_libraries(
        ${PROJECT_NAME}
        MaterialXContrib
        MaterialXCore
        MaterialXFormat
        MaterialXGenShader
        MaterialXGenOgsXml
        MaterialXRender
        ${CMAKE_DL_LIBS}
    )
    DEPEND_MAYAAPI_LINK(${PROJECT_NAME})

    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/plug-ins)
    install(FILES ${_scripts} DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/scripts)
    install(FILES ${_AEtemplate} DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/AEtemplate)
    install(FILES ${_vp2ShaderFragments} DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/plug-ins/vp2ShaderFragments)
    install(FILES ${_mods} DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../resources DESTINATION "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}")
endif()
