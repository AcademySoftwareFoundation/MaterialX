project(MayaExporter)
set(MAYA_EXPORTER_VERSION_MAJOR 0)
set(MAYA_EXPORTER_VERSION_MINOR 1)
set(MAYA_EXPORTER_BUILD_VERSION 0)

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(MAYA_EXPORTER_VERSION "${MAYA_EXPORTER_VERSION_MAJOR}.${MAYA_EXPORTER_VERSION_MINOR}")
set(SOVERSION ${MAYA_EXPORTER_VERSION_MAJOR} CACHE STRING "Set the SO version in the SO name of the output library")
SET(MAYA_DEBUG_DIR "" CACHE STRING "Location of Maya debug installation.")
SET(MAYA_RELEASE_DIR "" CACHE STRING "Location of Maya release installation.")

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
	else()
	   set(MAYA_LIB_PREFIX "")
	   set(MAYA_LIB_EXT "lib")
	   set(MAYA_LIB_DEBUG_DIR "${MAYA_DEBUG_DIR}/lib")
	   set(MAYA_LIB_RELEASE_DIR "${MAYA_RELEASE_DIR}/lib")
	endif()
	FOREACH(F ${LIBS})
		target_link_libraries(${targetname} debug "${MAYA_LIB_DEBUG_DIR}/${MAYA_LIB_PREFIX}${F}.${MAYA_LIB_EXT}")
		target_link_libraries(${targetname} optimized "${MAYA_LIB_RELEASE_DIR}/${MAYA_LIB_PREFIX}${F}.${MAYA_LIB_EXT}")
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
add_subdirectory(documents)
