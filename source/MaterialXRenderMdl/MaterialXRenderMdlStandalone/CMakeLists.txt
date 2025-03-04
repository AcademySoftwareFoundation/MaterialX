file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")
file(GLOB_RECURSE materialx_content "${CMAKE_CURRENT_SOURCE_DIR}/content/*")

set(POST_BUILD_DEP ${CMAKE_CURRENT_BINARY_DIR}/postbuild.d)
set_source_files_properties(${POST_BUILD_DEP} PROPERTIES GENERATED TRUE)
add_executable(MaterialXRenderMdlStandalone ${materialx_source} ${materialx_headers} ${materialx_content} ${POST_BUILD_DEP})

get_target_property(mdl_sdk_include mdl::mdl_sdk INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(MaterialXRenderMdlStandalone PRIVATE ${mdl_sdk_include})

find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SPIRV-Tools-opt CONFIG REQUIRED)

# the MDL-SDK is loaded dynamically
get_target_property(mdl_sdk_shared_library mdl::mdl_sdk IMPORTED_LOCATION_RELEASE)
get_target_property(nv_oiio_shared_library mdl::nv_openimageio IMPORTED_LOCATION_RELEASE)
get_target_property(dds_shared_library mdl::dds IMPORTED_LOCATION_RELEASE)

# other dependencies are linked
target_link_libraries(MaterialXRenderMdlStandalone
    PRIVATE
        MaterialXCore
        MaterialXFormat
        MaterialXGenShader
        MaterialXGenMdl
        glfw
        Vulkan::Headers
        volk::volk
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
        SPIRV-Tools-opt
    )
# copy the MDL SDK binaries to the output folder as the easiest solution without changing the PATH for execution
# also copy the content files
add_custom_command(TARGET MaterialXRenderMdlStandalone POST_BUILD
    BYPRODUCTS ${POST_BUILD_DEP}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${mdl_sdk_shared_library} $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${nv_oiio_shared_library} $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dds_shared_library} $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>/MaterialXRenderMdlStandalone
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${materialx_content} $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>/MaterialXRenderMdlStandalone
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/resources/Lights/goegap.hdr $<TARGET_FILE_DIR:MaterialXRenderMdlStandalone>/MaterialXRenderMdlStandalone
    COMMAND_EXPAND_LISTS
    )

install(TARGETS MaterialXRenderMdlStandalone
    EXPORT MaterialX
    RUNTIME DESTINATION bin)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXRenderMdlStandalone.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/" OPTIONAL)
