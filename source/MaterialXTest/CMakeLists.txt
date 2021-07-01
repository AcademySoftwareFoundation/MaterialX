file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")
list(APPEND materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/Catch/catch.hpp")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

set(MATERIALX_TEST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Discover all tests and allow them to be run in parallel (ctest -j20):
function(add_tests _sources)
  foreach(src_file ${_sources})
    file(STRINGS ${src_file} matched_lines REGEX "TEST_CASE")
    foreach(matched_line ${matched_lines})
      string(REGEX REPLACE "(TEST_CASE[( \"]+)" "" test_name ${matched_line})
      string(REGEX REPLACE "(\".*)" "" test_name ${test_name})
      string(REGEX REPLACE "[^A-Za-z0-9_]+" "_" test_safe_name ${test_name})
      add_test(NAME "MaterialXTest_${test_safe_name}"
          COMMAND MaterialXTest ${test_name}
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
      if(MATERIALX_BUILD_OIIO AND MSVC)
        # Add path to OIIO library so it can be found for the test.
        # On windows we have to escape the semicolons, otherwise only
        # the first path entry will be passed to the test executable
        STRING(REPLACE ";" "\\;" TESTPATH "$ENV{PATH}")
        STRING(APPEND TESTPATH "\\;${OPENIMAGEIO_ROOT_DIR}/bin")
        STRING(REPLACE "/" "\\" TESTPATH "${TESTPATH}")
        set_tests_properties("MaterialXTest_${test_safe_name}" PROPERTIES
                             ENVIRONMENT "PATH=${TESTPATH}")
      endif()
    endforeach()
  endforeach()
endfunction()

add_executable(MaterialXTest ${materialx_source} ${materialx_headers})

target_include_directories( MaterialXTest PUBLIC
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

add_subdirectory(MaterialXCore)
target_link_libraries(MaterialXTest MaterialXCore)
add_subdirectory(MaterialXFormat)
target_link_libraries(MaterialXTest MaterialXFormat)

if(MATERIALX_BUILD_GEN)
  if (MATERIALX_BUILD_OCIO)
     target_link_libraries(MaterialXTest ${MATERIALX_OCIO_LIBRARIES})
  endif()
  add_subdirectory(MaterialXGenShader)
  target_link_libraries(MaterialXTest MaterialXGenShader)
  if(MATERIALX_BUILD_GEN_GLSL)
    add_subdirectory(MaterialXGenGlsl)
    target_link_libraries(MaterialXTest MaterialXGenGlsl)
  endif()
  if(MATERIALX_BUILD_GEN_OSL)
    add_subdirectory(MaterialXGenOsl)
    target_link_libraries(MaterialXTest MaterialXGenOsl)
  endif()
  if(MATERIALX_BUILD_GEN_MDL)
    add_subdirectory(MaterialXGenMdl)
    target_link_libraries(MaterialXTest MaterialXGenMdl)
  endif()
  if(MATERIALX_BUILD_GEN_OGSXML)
    add_subdirectory(MaterialXGenOgsXml)
    target_link_libraries(MaterialXTest MaterialXGenOgsXml MaterialXGenGlsl)
    if (MATERIALX_BUILD_CROSS)
      target_link_libraries(MaterialXTest MaterialXCross)
    endif()
  endif()
  if(MATERIALX_BUILD_GEN_OGSFX)
    add_subdirectory(MaterialXGenOgsFx)
    target_link_libraries(MaterialXTest MaterialXGenGlsl MaterialXGenOgsFx)
  endif()
  if(MATERIALX_BUILD_GEN_ARNOLD)
    add_subdirectory(MaterialXGenArnold)
    target_link_libraries(MaterialXTest MaterialXGenArnold)
  endif()
endif()

if(MATERIALX_BUILD_RUNTIME)
  add_subdirectory(MaterialXRuntime)
  target_link_libraries(MaterialXTest MaterialXRuntime)
endif()

if(MATERIALX_BUILD_RENDER) # MATERIALX_BUILD_GEN is implied; can assume relevant MaterialXGen* libs already linked above
  add_subdirectory(MaterialXRender)
  target_link_libraries(MaterialXTest MaterialXRender)
  if(MATERIALX_BUILD_CONTRIB)
    target_link_libraries(MaterialXTest MaterialXContrib)
  endif()
  if (MATERIALX_BUILD_OIIO)
    target_link_libraries(MaterialXTest ${OPENIMAGEIO_LIBRARIES})
  endif()
  if (MATERIALX_BUILD_OCIO)
     target_link_libraries(MaterialXTest ${MATERIALX_OCIO_LIBRARIES})
  endif()
  if(MATERIALX_BUILD_GEN_GLSL)
    add_subdirectory(MaterialXRenderGlsl)
    target_link_libraries(MaterialXTest MaterialXRenderGlsl)
  endif()
  if(MATERIALX_BUILD_GEN_OSL)
    add_subdirectory(MaterialXRenderOsl)
    target_link_libraries(MaterialXTest MaterialXRenderOsl)
  endif()
  if(MATERIALX_BUILD_GEN_OGSFX)
    add_subdirectory(MaterialXRenderOgsFx)
  endif()
  if(MATERIALX_BUILD_GEN_ARNOLD)
    add_subdirectory(MaterialXRenderArnold)
  endif()
  if(MATERIALX_BUILD_OIIO AND OPENIMAGEIO_ROOT_DIR)
      add_custom_command(TARGET MaterialXTest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${OPENIMAGEIO_ROOT_DIR}/bin ${CMAKE_BINARY_DIR}/bin)
  endif()
  if(MATERIALX_BUILD_OCIO AND MATERIALX_OCIO_DIR)
      add_custom_command(TARGET MaterialXTest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${MATERIALX_OCIO_DIR}/bin ${CMAKE_BINARY_DIR}/bin)
  endif()
endif()

# TODO: Only do this stuff when it's relevant

add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../libraries ${CMAKE_BINARY_DIR}/bin/libraries)
if(MATERIALX_BUILD_GEN_ESSL)
  add_custom_command(TARGET MaterialXTest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/../../source/MaterialXGenEssl/libraries ${CMAKE_BINARY_DIR}/bin/libraries)
endif()

if(MATERIALX_BUILD_GEN_OSL)
    install(DIRECTORY DESTINATION ${CMAKE_BINARY_DIR}/bin/libraries/stdlib/reference/osl)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/stdlib/osl/"
        DESTINATION ${CMAKE_BINARY_DIR}/bin/libraries/stdlib/reference/osl
        FILES_MATCHING PATTERN "*.h")
endif()

if(MATERIALX_BUILD_GEN_MDL)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../source/MaterialXGenMdl/mdl/"
        DESTINATION ${CMAKE_BINARY_DIR}/bin/${MATERIALX_INSTALL_MDL_MODULE_PATH}/mdl)
endif()

add_custom_command(TARGET MaterialXTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_BINARY_DIR}/bin/resources)

set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

target_link_libraries(
    MaterialXTest
    ${CMAKE_DL_LIBS})
