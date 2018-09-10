include_directories(    
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

# Linux checks for X and OpenGL
if (UNIX AND NOT APPLE)
   find_package(PkgConfig REQUIRED)
   find_package(X11 REQUIRED)
   if (NOT X11_FOUND)
      message("Error in building MaterialXView: X11 was not found")
   endif(NOT X11_FOUND)
   if (NOT X11_Xt_FOUND)
      message("Error in building MaterialXView: Xt was not found")
   endif(NOT X11_Xt_FOUND)

   find_package(OpenGL REQUIRED)
   if (NOT OPENGL_FOUND)
      message("Error in building MaterialXView: OpenGL was not found")
   endif(NOT OPENGL_FOUND)

   include_directories(
	${X11_INCLUDE_DIR}
   )
endif(UNIX AND NOT APPLE)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
if (APPLE)
  find_library(COCOA_FRAMEWORK Cocoa)
  find_package(OpenGL REQUIRED)
  file(GLOB_RECURSE materialx_source_oc "${CMAKE_CURRENT_SOURCE_DIR}/*.m")
  message("Objective C files: " ${materialx_source_oc})
  set_source_files_properties(${materialx_source_oc} PROPERTIES 
                            COMPILE_FLAGS "-x objective-c++")
  set(materialx_source ${materialx_source} ${materialx_source_oc})

endif(APPLE)

function(assign_source_group prefix)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${prefix}\\${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group("Header Files" ${materialx_header})
assign_source_group("Source Files" ${materialx_source})

add_library(MaterialXView STATIC 
    ${materialx_source} 
    ${materialx_header}
)

if(MSVC)
    target_link_libraries(
        MaterialXView
        MaterialXGenShader
        MaterialXCore
        Opengl32
        ${CMAKE_DL_LIBS})
elseif (APPLE)
    target_link_libraries(
        MaterialXView
        MaterialXGenShader
        MaterialXCore
        ${CMAKE_DL_LIBS}
        ${OPENGL_LIBRARIES}
	"-framework Foundation"
	"-framework Cocoa"
)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(
        MaterialXView
        MaterialXGenShader
        MaterialXCore
        ${CMAKE_DL_LIBS}
        ${OPENGL_LIBRARIES}
	${X11_LIBRARIES}
        ${X11_Xt_LIB}
    )
endif(MSVC)

set_target_properties(
    MaterialXView PROPERTIES
    OUTPUT_NAME MaterialXView
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

install(TARGETS MaterialXView
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXView/
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXView.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/documents/Examples" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/documents/Libraries" DESTINATION ${CMAKE_INSTALL_PREFIX}/documents)

