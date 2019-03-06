include_directories(
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

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

add_library(MaterialXRender STATIC
    ${materialx_source}
    ${materialx_header}
)

if(MSVC)
    target_link_libraries(
        MaterialXRender
        MaterialXGenShader
        MaterialXCore
        Opengl32
        ${CMAKE_DL_LIBS})
elseif (APPLE)
    target_link_libraries(
        MaterialXRender
        MaterialXGenShader
        MaterialXCore
        ${CMAKE_DL_LIBS}
)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(
        MaterialXRender
        MaterialXGenShader
        MaterialXCore
        ${CMAKE_DL_LIBS}
    )
endif(MSVC)

if(MATERIALX_BUILD_OIIO)
	set(OPENIMAGEIO_ROOT_DIR ${MATERIALX_OIIO_DIR})
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/OpenImageIO")
	find_package(OpenImageIO REQUIRED)
	if(OPENIMAGEIO_FOUND)
		include_directories(${OPENIMAGEIO_INCLUDE_DIR})
		target_link_libraries(MaterialXRender ${OPENIMAGEIO_LIBRARIES})
	endif()
endif()

set_target_properties(
    MaterialXRender PROPERTIES
    OUTPUT_NAME MaterialXRender
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

install(TARGETS MaterialXRender
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MaterialXRender/ MESSAGE_NEVER
    FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRender.pdb"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/" OPTIONAL)
