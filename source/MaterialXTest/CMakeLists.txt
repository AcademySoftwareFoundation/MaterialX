file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")
list(APPEND materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/External/Catch/catch.hpp")

assign_source_group("Source Files" ${materialx_source})
assign_source_group("Header Files" ${materialx_headers})

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
          WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
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

function(mx_add_test_module MODULE_NAME)
    file(GLOB_RECURSE module_source "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}/*.cpp")
    if(APPLE)
        file(GLOB_RECURSE objc_source "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}/*.mm")
        list(APPEND module_source ${objc_source})
    endif()
    file(GLOB_RECURSE module_headers "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}/*.h")

    target_sources(MaterialXTest PUBLIC ${module_source} ${module_headers})
    target_link_libraries(MaterialXTest ${MODULE_NAME})

    add_tests("${module_source}")

    assign_source_group("Source Files" ${module_source})
    assign_source_group("Header Files" ${module_headers})
endfunction()

add_executable(MaterialXTest ${materialx_source} ${materialx_headers})

target_include_directories( MaterialXTest PUBLIC
    ${EXTERNAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../)

mx_add_test_module(MaterialXCore)
mx_add_test_module(MaterialXFormat)

mx_add_test_module(MaterialXGenShader)
if(MATERIALX_BUILD_GEN_GLSL OR MATERIALX_BUILD_GEN_OSL OR MATERIALX_BUILD_GEN_MDL OR MATERIALX_BUILD_GEN_MSL OR MATERIALX_BUILD_GEN_SLANG)
  if(MATERIALX_BUILD_GEN_GLSL)
    mx_add_test_module(MaterialXGenGlsl)
  endif()
  if(MATERIALX_BUILD_GEN_SLANG)
    mx_add_test_module(MaterialXGenSlang)
  endif()
  if(MATERIALX_BUILD_GEN_OSL)
    mx_add_test_module(MaterialXGenOsl)
  endif()
  if(MATERIALX_BUILD_GEN_MDL)
    mx_add_test_module(MaterialXGenMdl)
    if (NOT MATERIALX_BUILD_MONOLITHIC)
        get_target_property(MaterialXGenMdl_SOURCE_DIR MaterialXGenMdl SOURCE_DIR)
    else()
        get_target_property(PROJECT_SOURCE_DIR ${MATERIALX_MONOLITHIC_TARGET} SOURCE_DIR)
        set(MaterialXGenMdl_SOURCE_DIR "${PROJECT_SOURCE_DIR}/MaterialXGenMdl")
    endif()
    target_compile_definitions(MaterialXTest PRIVATE
        MATERIALX_MDL_BINARY_MDLC=\"${MATERIALX_MDL_BINARY_MDLC}\"
        MATERIALX_MDL_IMPL_MODULE_PATH=\"${MaterialXGenMdl_SOURCE_DIR}/mdl\"
        MATERIALX_INSTALL_MDL_MODULE_PATH=\"${MATERIALX_INSTALL_STDLIB_PATH}\"
        MATERIALX_MDL_MODULE_PATHS=\"${MATERIALX_MDL_MODULE_PATHS}\"
    )
  endif()
  if(MATERIALX_BUILD_GEN_MSL)
    mx_add_test_module(MaterialXGenMsl)
  endif()
endif()

if(MATERIALX_BUILD_RENDER)
  mx_add_test_module(MaterialXRender)
  if(MATERIALX_BUILD_GEN_GLSL AND NOT MATERIALX_RENDER_MSL_ONLY)
    mx_add_test_module(MaterialXRenderGlsl)
  endif()
  if(MATERIALX_BUILD_GEN_SLANG AND MATERIALX_SLANG_RHI_SOURCE_DIR)
    mx_add_test_module(MaterialXRenderSlang)
  endif()
  if(MATERIALX_BUILD_GEN_OSL)
    mx_add_test_module(MaterialXRenderOsl)
    target_compile_definitions(MaterialXTest PRIVATE
        MATERIALX_OSL_BINARY_OSLC=\"${MATERIALX_OSL_BINARY_OSLC}\"
        MATERIALX_OSL_BINARY_TESTRENDER=\"${MATERIALX_OSL_BINARY_TESTRENDER}\"
        MATERIALX_OSL_INCLUDE_PATH=\"${MATERIALX_OSL_INCLUDE_PATH}\"
    )
  endif()
  if(APPLE AND MATERIALX_BUILD_GEN_MSL)
    mx_add_test_module(MaterialXRenderMsl)
  endif()
  if(MATERIALX_BUILD_OIIO AND OPENIMAGEIO_ROOT_DIR)
      add_custom_command(TARGET MaterialXTest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${OPENIMAGEIO_ROOT_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR})
  endif()
endif()

# Disable deprecation warnings on Clang.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(MaterialXTest PUBLIC -Wno-deprecated-declarations)
endif()

# The MaterialX test suite maintains a synchronized copy of the data libraries.
add_custom_target(MaterialXTestData ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${MATERIALX_DATA_LIBRARY_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libraries)
if(MATERIALX_BUILD_DATA_LIBRARY)
    add_dependencies(MaterialXTestData MaterialXBuildData)
endif()

if(MATERIALX_BUILD_GEN_MDL)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../source/MaterialXGenMdl/mdl/"
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MATERIALX_INSTALL_STDLIB_PATH}/mdl)
endif()

# Copy resources to build directory - always runs during build
add_custom_target(MaterialXTestResources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
    COMMENT "Copying resources to build directory")

# Copy OSL utilities to build directory - always runs during build
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/MaterialXRenderOsl/Utilities")
    add_custom_target(MaterialXTestOslUtilities ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/MaterialXRenderOsl/Utilities ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/Utilities
        COMMENT "Copying OSL utilities to build directory")
    add_dependencies(MaterialXTestResources MaterialXTestOslUtilities)
endif()

add_dependencies(MaterialXTest MaterialXTestResources)

set_target_properties(
    MaterialXTest PROPERTIES
    OUTPUT_NAME MaterialXTest
    COMPILE_FLAGS "${EXTERNAL_COMPILE_FLAGS}"
    LINK_FLAGS "${EXTERNAL_LINK_FLAGS}"
    VERSION "${MATERIALX_LIBRARY_VERSION}"
    SOVERSION "${MATERIALX_MAJOR_VERSION}")

if(MATERIALX_BUILD_BENCHMARK_TESTS)
    target_compile_definitions(MaterialXTest PRIVATE -DCATCH_CONFIG_ENABLE_BENCHMARKING)
endif()

target_link_libraries(
    MaterialXTest
    ${CMAKE_DL_LIBS})
