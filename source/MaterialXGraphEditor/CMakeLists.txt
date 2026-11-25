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

set(materialx_source
    "${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graph.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderView.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UiNode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GlslRenderPipeline.cpp")

if(MATERIALX_BUILD_RENDER_CYCLES OR MATERIALX_BUILD_RENDER_OSL_TESTRENDER)
    list(APPEND materialx_source
        "${CMAKE_CURRENT_SOURCE_DIR}/GraphEditorOslGenerator.cpp")
endif()

if(MATERIALX_BUILD_RENDER_CYCLES)
    list(APPEND materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/CyclesRenderPipeline.cpp")
endif()

if(MATERIALX_BUILD_RENDER_OSL_TESTRENDER)
    list(APPEND materialx_source
        "${CMAKE_CURRENT_SOURCE_DIR}/OslTestRenderPipeline.cpp")
endif()

if (APPLE)
    list(APPEND materialx_source ${CMAKE_CURRENT_SOURCE_DIR}/FileDialog_Darwin.mm)
endif ()

set(materialx_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/FileDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graph.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/GlslRenderPipeline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layout.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderPipeline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderTypes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderView.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/UiNode.h")

if(MATERIALX_BUILD_RENDER_CYCLES OR MATERIALX_BUILD_RENDER_OSL_TESTRENDER)
    list(APPEND materialx_headers
        "${CMAKE_CURRENT_SOURCE_DIR}/GraphEditorOslGenerator.h")
endif()

if(MATERIALX_BUILD_RENDER_CYCLES)
    list(APPEND materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/CyclesRenderPipeline.h")
endif()

if(MATERIALX_BUILD_RENDER_OSL_TESTRENDER)
    list(APPEND materialx_headers
        "${CMAKE_CURRENT_SOURCE_DIR}/OslTestRenderPipeline.h")
endif()

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

    # An embedded Cycles build creates its shader directory (headers and
    # compiled node_*.oso files) during the build via cycles_build_shaders, so
    # it does not exist yet at configure time.  Only an installed package is
    # expected to ship a complete shader directory up front.
    if(NOT _materialx_cycles_embedded AND
       (NOT IS_DIRECTORY "${MATERIALX_CYCLES_SHADER_DIR}" OR
        NOT EXISTS "${MATERIALX_CYCLES_SHADER_DIR}/stdosl.h" OR
        NOT EXISTS "${MATERIALX_CYCLES_SHADER_DIR}/stdcycles.h"))
        message(FATAL_ERROR "The Cycles shader directory is incomplete: "
            "'${MATERIALX_CYCLES_SHADER_DIR}'. Expected stdosl.h, stdcycles.h, "
            "and the compiled node_*.oso shaders.")
    endif()

    target_compile_definitions(MaterialXGraphEditor PRIVATE
        MATERIALX_BUILD_RENDER_CYCLES
        CYCLES_SHADER_DIR="${MATERIALX_CYCLES_SHADER_DIR}")
endif()

if(MATERIALX_BUILD_RENDER_OSL_TESTRENDER)
    if(NOT TARGET OpenImageIO::OpenImageIO)
        find_package(OpenImageIO CONFIG REQUIRED)
    endif()
    list(APPEND MATERIALX_LIBRARIES MaterialXGenOsl OSL::libtestrender OpenImageIO::OpenImageIO)
    # Optional: static OSL exports pugixml through its CMake package, and shared
    # OSL installs may still need libpugixml on the link/runtime path.
    if(TARGET pugixml::pugixml)
        list(APPEND MATERIALX_LIBRARIES pugixml::pugixml)
    endif()

    # Determine the compiled-in fallback location of stdosl.h.  At runtime the
    # pipeline prefers a location relative to the executable (or
    # MATERIALX_OSL_SHADER_PATH), so this only matters for uninstalled builds.
    if(MATERIALX_OSL_INCLUDE_PATH)
        set(_materialx_osl_stdosl "${MATERIALX_OSL_INCLUDE_PATH}/stdosl.h")
    elseif(MATERIALX_OSL_EMBEDDED AND MATERIALX_OSL_SOURCE_ROOT AND
           EXISTS "${MATERIALX_OSL_SOURCE_ROOT}/src/shaders/stdosl.h")
        # Uninstalled embedded builds use OSL's source shader directory.
        set(_materialx_osl_stdosl
            "${MATERIALX_OSL_SOURCE_ROOT}/src/shaders/stdosl.h")
    elseif(MATERIALX_OSL_EMBEDDED)
        set(_materialx_osl_stdosl
            "${CMAKE_INSTALL_PREFIX}/share/OSL/shaders/stdosl.h")
    elseif(DEFINED OSL_INCLUDE_DIR AND OSL_INCLUDE_DIR)
        get_filename_component(_materialx_osl_prefix "${OSL_INCLUDE_DIR}/.." ABSOLUTE)
        set(_materialx_osl_stdosl
            "${_materialx_osl_prefix}/share/OSL/shaders/stdosl.h")
    else()
        set(_materialx_osl_stdosl "")
    endif()
    target_compile_definitions(MaterialXGraphEditor PRIVATE
        MATERIALX_BUILD_RENDER_OSL_TESTRENDER
        MATERIALX_OSL_STDOSL_PATH="${_materialx_osl_stdosl}")

    if(MATERIALX_OSL_BUNDLE_RUNTIME AND RPATH_RELATIVE_SYMBOL)
        # Copy the entire OSL/OpenImageIO/LLVM runtime closure next to the
        # executable and resolve it through a relative rpath so the resulting
        # install tree and release archive can be moved anywhere.
        set_target_properties(MaterialXGraphEditor PROPERTIES
            BUILD_RPATH "${RPATH_RELATIVE_SYMBOL}"
            INSTALL_RPATH "${RPATH_RELATIVE_SYMBOL};${RPATH_RELATIVE_SYMBOL}/../${MATERIALX_INSTALL_LIB_PATH}")
        if(UNIX AND NOT APPLE)
            # Emit DT_RPATH instead of DT_RUNPATH so that the executable's
            # $ORIGIN also resolves transitive dependencies of the bundled
            # libraries (for example libclang-cpp -> libLLVM).
            target_link_options(MaterialXGraphEditor PRIVATE "-Wl,--disable-new-dtags")
        endif()

        set(_materialx_osl_runtime_dirs "")
        if(DEFINED OSL_LIB_DIR AND OSL_LIB_DIR)
            get_filename_component(_materialx_osl_lib_dir "${OSL_LIB_DIR}" ABSOLUTE)
            list(APPEND _materialx_osl_runtime_dirs "${_materialx_osl_lib_dir}")
        endif()
        foreach(_target OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util pugixml::pugixml)
            if(TARGET ${_target})
                get_target_property(_materialx_dep_location ${_target} IMPORTED_LOCATION)
                if(NOT _materialx_dep_location)
                    get_target_property(_materialx_dep_location ${_target} IMPORTED_IMPLIB)
                endif()
                if(_materialx_dep_location)
                    get_filename_component(_materialx_dep_dir
                        "${_materialx_dep_location}" DIRECTORY)
                    list(APPEND _materialx_osl_runtime_dirs "${_materialx_dep_dir}")
                endif()
            endif()
        endforeach()
        # Locate the LLVM runtime directory.  LLVM_DIR is set by find_package
        # in an embedded OSL build; otherwise ask llvm-config.
        if(DEFINED LLVM_DIR AND LLVM_DIR)
            get_filename_component(_materialx_llvm_lib_dir "${LLVM_DIR}/../.." ABSOLUTE)
            list(APPEND _materialx_osl_runtime_dirs "${_materialx_llvm_lib_dir}")
        else()
            find_program(_materialx_llvm_config NAMES llvm-config)
            if(_materialx_llvm_config)
                execute_process(COMMAND "${_materialx_llvm_config}" --libdir
                    OUTPUT_VARIABLE _materialx_llvm_lib_dir
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
                if(_materialx_llvm_lib_dir)
                    list(APPEND _materialx_osl_runtime_dirs "${_materialx_llvm_lib_dir}")
                endif()
            endif()
        endif()
        list(REMOVE_DUPLICATES _materialx_osl_runtime_dirs)
        list(JOIN _materialx_osl_runtime_dirs ";" _materialx_osl_runtime_dirs_arg)

        add_custom_command(TARGET MaterialXGraphEditor POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                "-DEXECUTABLE=$<TARGET_FILE:MaterialXGraphEditor>"
                "-DDESTINATION=$<TARGET_FILE_DIR:MaterialXGraphEditor>"
                "-DDIRECTORIES=${_materialx_osl_runtime_dirs_arg}"
                "-DINCLUDE_SYSTEM_LIBS=ON"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/BundleCyclesRuntime.cmake"
            COMMENT "Bundling OSL runtime libraries beside MaterialXGraphEditor"
            VERBATIM)
        set_property(TARGET MaterialXGraphEditor APPEND PROPERTY LINK_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/BundleCyclesRuntime.cmake")
    else()
        # Without bundling, record the configured dependency locations so the
        # install tree can run without an explicit LD_LIBRARY_PATH.
        set(_materialx_osl_rpaths "")
        if(DEFINED OSL_LIB_DIR AND OSL_LIB_DIR)
            get_filename_component(_dir "${OSL_LIB_DIR}" ABSOLUTE)
            list(APPEND _materialx_osl_rpaths "${_dir}")
        endif()
        foreach(_target OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util pugixml::pugixml)
            if(TARGET ${_target})
                get_target_property(_materialx_dep_location ${_target} IMPORTED_LOCATION)
                if(NOT _materialx_dep_location)
                    get_target_property(_materialx_dep_location ${_target} IMPORTED_IMPLIB)
                endif()
                if(_materialx_dep_location)
                    get_filename_component(_materialx_dep_dir
                        "${_materialx_dep_location}" DIRECTORY)
                    list(APPEND _materialx_osl_rpaths "${_materialx_dep_dir}")
                endif()
            endif()
        endforeach()
        if(_materialx_osl_rpaths)
            list(REMOVE_DUPLICATES _materialx_osl_rpaths)
            set_property(TARGET MaterialXGraphEditor APPEND PROPERTY INSTALL_RPATH
                "${_materialx_osl_rpaths}")
        endif()
    endif()

    # When OSL is an external package, install its standard shader headers so
    # the graph editor can find stdosl.h in a self-contained install tree.
    # An embedded OSL build installs its own shaders.
    if(NOT MATERIALX_OSL_EMBEDDED AND EXISTS "${_materialx_osl_stdosl}")
        get_filename_component(_materialx_osl_shader_dir
            "${_materialx_osl_stdosl}" DIRECTORY)
        install(DIRECTORY "${_materialx_osl_shader_dir}/"
                DESTINATION "share/OSL/shaders")
    endif()
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

if(MATERIALX_BUILD_RENDER_CYCLES)
    # Copy the Cycles OSL shaders and headers beside the executable so that
    # CYCLES_SHADER_PATH can be resolved relative to the executable at runtime.
    add_custom_command(TARGET MaterialXGraphEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MATERIALX_CYCLES_SHADER_DIR}"
            "$<TARGET_FILE_DIR:MaterialXGraphEditor>/shader"
        COMMENT "Copying Cycles shader directory"
        VERBATIM)

    install(DIRECTORY "${MATERIALX_CYCLES_SHADER_DIR}/"
            DESTINATION "${MATERIALX_INSTALL_BIN_PATH}/shader")
endif()

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
        INSTALL_RPATH "${RPATH_RELATIVE_SYMBOL};${RPATH_RELATIVE_SYMBOL}/../${MATERIALX_INSTALL_LIB_PATH}")

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

# When Cycles comes from an installed development package, its libraries are
# already collected in Cycles_LIBRARY_DIR with $ORIGIN run paths.  Copy them
# beside the executable so the same relocatable layout is produced.
if(MATERIALX_BUILD_RENDER_CYCLES AND MATERIALX_CYCLES_BUNDLE_RUNTIME AND
   NOT _materialx_cycles_embedded AND DEFINED Cycles_LIBRARY_DIR AND
   IS_DIRECTORY "${Cycles_LIBRARY_DIR}" AND RPATH_RELATIVE_SYMBOL)
    set_target_properties(MaterialXGraphEditor PROPERTIES
        BUILD_RPATH "${RPATH_RELATIVE_SYMBOL};${RPATH_RELATIVE_SYMBOL}/../${MATERIALX_INSTALL_LIB_PATH}"
        INSTALL_RPATH "${RPATH_RELATIVE_SYMBOL};${RPATH_RELATIVE_SYMBOL}/../${MATERIALX_INSTALL_LIB_PATH}")

    add_custom_command(TARGET MaterialXGraphEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCE_DIR=${Cycles_LIBRARY_DIR}"
            "-DDEST_DIR=$<TARGET_FILE_DIR:MaterialXGraphEditor>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/BundleRuntimeLibraries.cmake"
        COMMENT "Copying Cycles runtime libraries"
        VERBATIM)

    install(CODE "
        set(SOURCE_DIR \"${Cycles_LIBRARY_DIR}\")
        set(DEST_DIR \"\${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_BIN_PATH}\")
        include(\"${CMAKE_CURRENT_SOURCE_DIR}/BundleRuntimeLibraries.cmake\")")
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
if(MATERIALX_BUILD_RENDER_OSL_TESTRENDER AND MATERIALX_OSL_BUNDLE_RUNTIME)
    # Install the OSL runtime closure that was copied beside the build-tree
    # executable so the install tree is relocatable as well.
    install(CODE
        "file(GLOB _materialx_osl_runtime_libs
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.so*\"
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.dylib*\"
            \"$<TARGET_FILE_DIR:MaterialXGraphEditor>/*.dll\")
         if(_materialx_osl_runtime_libs)
            file(INSTALL \${_materialx_osl_runtime_libs}
                DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${MATERIALX_INSTALL_BIN_PATH}\")
         endif()")
endif()
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:MaterialXGraphEditor>
            DESTINATION ${MATERIALX_INSTALL_BIN_PATH} OPTIONAL)
endif()
