set(DEAR_IMGUI_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui" CACHE STRING "Path to Dear ImGui")

if (NOT IS_DIRECTORY "${DEAR_IMGUI_PREFIX}/backends")
    message(FATAL_ERROR "Building the MaterialX graph editor requires the ImGui submodule "
        "to be present. Update your repository by calling the following:\n"
        "git submodule update --init --recursive")
endif()

if(MSVC)
    add_compile_options(-wd4100 -wd4152 -wd4201 -wd4244 -wd4456)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-unused -Wno-deprecated -Wno-comment -Wno-unused-variable -Wno-unused-parameter -Wno-shadow)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-format-truncation -Wno-use-after-free -Wno-comment -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-shadow)
endif()

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

if (APPLE)
    list(APPEND materialx_source ${CMAKE_CURRENT_SOURCE_DIR}/FileDialog_Darwin.mm)
endif ()

file(GLOB imgui_source "${DEAR_IMGUI_PREFIX}/*.cpp")
file(GLOB imgui_headers "${DEAR_IMGUI_PREFIX}/*.h*")

set(imgui_source ${imgui_source}
    "${DEAR_IMGUI_PREFIX}/backends/imgui_impl_glfw.cpp"
    "${DEAR_IMGUI_PREFIX}/backends/imgui_impl_opengl3.cpp"
    "${DEAR_IMGUI_PREFIX}/misc/cpp/imgui_stdlib.cpp")
set(imgui_headers ${imgui_headers}
    "${DEAR_IMGUI_PREFIX}/backends/imgui_impl_glfw.h"
    "${DEAR_IMGUI_PREFIX}/backends/imgui_impl_opengl3.h"
    "${DEAR_IMGUI_PREFIX}/misc/cpp/imgui_stdlib.h")

file(GLOB imguinode_source "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/*.cpp")
file(GLOB imguinode_headers "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/*.h*")

set(imguinode_source ${imguinode_source}
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities/drawing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities/widgets.cpp")
set(imguinode_headers ${imguinode_headers}
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities/drawing.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities/widgets.h")

assign_source_group("Source Files" ${materialx_source} ${imgui_source} ${imguinode_source})
assign_source_group("Header Files" ${materialx_headers} ${imgui_headers} ${imguinode_headers})

include_directories("${DEAR_IMGUI_PREFIX}")
include_directories("${DEAR_IMGUI_PREFIX}/backends")
include_directories("${DEAR_IMGUI_PREFIX}/misc/cpp")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities")

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

# Explicitly build GLFW with static libraries, independent of the broader build settings.
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(External/Glfw)
set_property(TARGET glfw_minimal PROPERTY FOLDER "External")

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

add_executable(MaterialXGraphEditor
    ${materialx_source}
    ${materialx_headers}
    ${imgui_source}
    ${imgui_headers}
    ${imguinode_source}
    ${imguinode_headers})

set(MATERIALX_LIBRARIES
    MaterialXFormat
    MaterialXGenGlsl
    MaterialXRenderGlsl)

if(MATERIALX_BUILD_RENDER_CYCLES)
    if(NOT MATERIALX_BUILD_GEN_OSL)
        message(FATAL_ERROR "The Cycles render backend requires MATERIALX_BUILD_GEN_OSL.")
    endif()
    list(APPEND MATERIALX_LIBRARIES MaterialXGenOsl MaterialXRenderOsl)

    if(TARGET cycles_build_shaders)
        # Cycles is embedded in the same CMake build (e.g. fetched by the
        # top-level project or a superbuild that pulls in both projects). Link
        # the in-tree target and use its build-tree shader directory instead of
        # an installed package.
        if(NOT TARGET Cycles::cycles)
            message(FATAL_ERROR "The embedded Cycles build does not provide the Cycles::cycles target.")
        endif()
        list(APPEND MATERIALX_LIBRARIES Cycles::cycles)
        set(MATERIALX_CYCLES_SHADER_DIR "${CYCLES_BUILD_SHADER_DIR}")
        add_dependencies(MaterialXGraphEditor cycles_build_shaders)
        set(_materialx_cycles_embedded TRUE)
    else()
        find_package(Cycles CONFIG REQUIRED)
        list(APPEND MATERIALX_LIBRARIES Cycles::cycles)
        set(MATERIALX_CYCLES_SHADER_DIR "${Cycles_SHADER_DIR}")
    endif()

    if(NOT MATERIALX_CYCLES_SHADER_DIR)
        message(FATAL_ERROR "Could not determine the Cycles shader directory. "
            "The embedded build must define CYCLES_BUILD_SHADER_DIR, or an "
            "installed Cycles package must define Cycles_SHADER_DIR.")
    endif()

    target_compile_definitions(MaterialXGraphEditor PRIVATE
        MATERIALX_BUILD_RENDER_CYCLES
        CYCLES_SHADER_DIR="${MATERIALX_CYCLES_SHADER_DIR}")
endif()

if(APPLE)
    find_library(CORE_FOUNDATION Foundation REQUIRED)
    list(APPEND MATERIALX_LIBRARIES ${CORE_FOUNDATION})
endif()

target_link_libraries(
    MaterialXGraphEditor
    PRIVATE
    ${MATERIALX_LIBRARIES}
 	glfw_minimal)

# The Cycles precompiled libraries use DT_RUNPATH="$ORIGIN" and depend on
# siblings in other directories, so the executable cannot resolve them through
# its own rpath. Copy the whole dependency closure next to the executable and
# point the rpaths at "$ORIGIN" so that it runs without LD_LIBRARY_PATH.
if(MATERIALX_BUILD_RENDER_CYCLES AND MATERIALX_CYCLES_BUNDLE_RUNTIME AND _materialx_cycles_embedded AND RPATH_RELATIVE_SYMBOL)
    # cycles_shared's build-tree rpath points at the original precompiled
    # directories, which would bypass the bundled copies, so override it.
    set_target_properties(cycles_shared PROPERTIES
        BUILD_RPATH "${RPATH_RELATIVE_SYMBOL}"
        INSTALL_RPATH "${RPATH_RELATIVE_SYMBOL}")

    get_target_property(_materialx_build_rpath MaterialXGraphEditor BUILD_RPATH)
    if(NOT _materialx_build_rpath)
        set(_materialx_build_rpath "")
    endif()
    set_target_properties(MaterialXGraphEditor PROPERTIES
        BUILD_RPATH "${RPATH_RELATIVE_SYMBOL};${_materialx_build_rpath}"
        INSTALL_RPATH "${RPATH_RELATIVE_SYMBOL}")

    add_custom_command(TARGET MaterialXGraphEditor POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-DEXECUTABLE=$<TARGET_FILE:MaterialXGraphEditor>"
            "-DDESTINATION=$<TARGET_FILE_DIR:MaterialXGraphEditor>"
            "-DEXTRA_LIBRARIES=$<TARGET_FILE:Cycles::cycles>"
            "-DCOPY_FILES=$<TARGET_SONAME_FILE:Cycles::cycles>"
            "-DDIRECTORIES=${CYCLES_BUNDLED_LIBRARY_DIRS}"
            "-DCYCLES_LIBRARY_DIR=$<TARGET_FILE_DIR:Cycles::cycles>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/BundleCyclesRuntime.cmake"
        COMMENT "Bundling Cycles runtime libraries beside MaterialXGraphEditor"
        VERBATIM)
    # Re-run the bundle when the script changes.
    set_property(TARGET MaterialXGraphEditor APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/BundleCyclesRuntime.cmake")
endif()

install(TARGETS MaterialXGraphEditor
    EXPORT MaterialX
    RUNTIME DESTINATION ${MATERIALX_INSTALL_BIN_PATH})
if(MATERIALX_BUILD_RENDER_CYCLES AND MATERIALX_CYCLES_BUNDLE_RUNTIME AND _materialx_cycles_embedded)
    # Install the libraries bundled beside the build-tree executable. Using the
    # actual build output directory handles multi-config generators; the install
    # prefix is resolved at install time so that --prefix works.
    install(CODE
        "file(GLOB _materialx_cycles_runtime_libs
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.so*\"
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.dylib*\"
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.dll\")
         if(_materialx_cycles_runtime_libs)
            file(INSTALL \${_materialx_cycles_runtime_libs}
                DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_BIN_PATH}\")
         endif()")
endif()
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:MaterialXGraphEditor>
            DESTINATION ${MATERIALX_INSTALL_BIN_PATH} OPTIONAL)
endif()
