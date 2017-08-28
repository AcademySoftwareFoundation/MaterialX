project(MaterialXForMaya)
set(MXTOOLS_VERSION_MAJOR 0)
set(MXTOOLS_VERSION_MINOR 1)

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(ADSKMATERIALX_VERSION "${ADSKMATERIALX_VERSION_MAJOR}.${ADSKMATERIALX_VERSION_MINOR}")
set(SOVERSION ${ADSKMATERIALX_VERSION_MAJOR} CACHE STRING "Set the SO version in the SO name of the output library")
SET(MAYA_DEBUG_DIR "" CACHE STRING "Location of Maya debug installation.")
SET(MAYA_RELEASE_DIR "" CACHE STRING "Location of Maya release installation.")
SET(MAYA_RELEASE_DIR "" CACHE STRING "Location of Maya release installation.")
SET(MATERIALX_DIR "" CACHE STRING "Location of MaterialX SDK.")

macro(DEPEND_MAYAAPI_INCLUDE)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
	    include_directories("${MAYA_DEBUG_DIR}/include" "${MAYA_DEBUG_DIR}/devkit/plug-ins")
    else()
	    include_directories("${MAYA_RELEASE_DIR}/include" "${MAYA_RELEASE_DIR}/devkit/plug-ins")
    endif()
	add_definitions(-DNT_PLUGIN -DREQUIRE_IOSTREAM -DAW_NEW_IOSTREAMS)
endmacro(DEPEND_MAYAAPI_INCLUDE)

macro(DEPEND_MAYAAPI_LINK targetname)
	set(LIBS 
		"OpenMaya.lib"
		"OpenMayaRender.lib"
		"Foundation.lib"
		"OpenMayaUI.lib"
		"OpenMayaAnim.lib"
		"OpenMayaFX.lib"
	)
	FOREACH(F ${LIBS})
		target_link_libraries(${targetname} debug "${MAYA_DEBUG_DIR}/lib/${F}")
		target_link_libraries(${targetname} optimized "${MAYA_RELEASE_DIR}/lib/${F}")
	ENDFOREACH(F)
endmacro(DEPEND_MAYAAPI_LINK)

# Adjust compiler settings
if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

# Add subdirectories
add_subdirectory(source)
