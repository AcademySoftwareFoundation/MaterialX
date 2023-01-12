set(CMAKE_CXX_STANDARD 17)

set(DEAR_IMGUI_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui" CACHE STRING "Path to Dear ImGui")

if(MSVC)
    add_compile_options(-wd4100)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-unused -Wno-deprecated -Wno-comment -Wno-unused-variable)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-format-truncation -Wno-use-after-free -Wno-comment -Wno-unused-but-set-variable)
endif()

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

file(GLOB imgui_source "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/*.cpp")
file(GLOB imgui_headers "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/*.h*")

set(imgui_source ${imgui_source}
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/misc/cpp/imgui_stdlib.cpp")
set(imgui_headers ${imgui_headers}
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/backends/imgui_impl_glfw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/backends/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/ImGui/misc/cpp/imgui_stdlib.h")

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

find_package(OpenGL REQUIRED)

if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
endif()

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories("${DEAR_IMGUI_PREFIX}")
include_directories("${DEAR_IMGUI_PREFIX}/backends")
include_directories("${DEAR_IMGUI_PREFIX}/misc/cpp")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiNodeEditor/examples/blueprints-example/utilities")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/External/ImGuiFileBrowser")

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

add_subdirectory(External/Glfw)

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

target_link_libraries(
    MaterialXGraphEditor
    PRIVATE
    ${MATERIALX_LIBRARIES}
    ${OPENGL_LIBRARIES}
 	glfw_minimal)

if (UNIX)
    if (APPLE)
        target_link_libraries(MaterialXGraphEditor
            PRIVATE
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo")
    else()
        target_link_libraries(MaterialXGraphEditor
            PRIVATE
                ${CMAKE_THREAD_LIBS_INIT}
                ${X11_LIBRARIES}
                ${CMAKE_DL_LIBS})
    endif()
endif()

install(TARGETS MaterialXGraphEditor
    EXPORT MaterialX
    RUNTIME DESTINATION bin)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXGraphEditor.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/" OPTIONAL)
