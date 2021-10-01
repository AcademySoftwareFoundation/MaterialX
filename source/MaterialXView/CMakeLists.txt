option(MATERIALX_NANOGUI_EXTERNAL "Build aginst an external install of NanoGUI (NANOGUI_ROOT may also need to be set)" OFF)

if (MATERIALX_NANOGUI_EXTERNAL)
    find_path(NANOGUI_INCLUDE_DIRS
        NAMES
            nanogui/nanogui.h
        HINTS
            "${NANOGUI_ROOT}/include"
            "$ENV{NANOGUI_ROOT}/include")
    find_library(NANOGUI_LIBRARIES
        NAMES
            nanogui
        HINTS
            "${NANOGUI_ROOT}/lib"
            "$ENV{NANOGUI_ROOT}/lib"
            "${NANOGUI_ROOT}/lib64"
            "$ENV{NANOGUI_ROOT}/lib64")

    if (NOT NANOGUI_INCLUDE_DIRS OR NOT NANOGUI_LIBRARIES)
        message(FATAL_ERROR "Could not find external NanoGUI installation, is NANOGUI_ROOT set?")
    endif()
else()
    if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI/ext/glfw/src")
        message(FATAL_ERROR "Building the MaterialX viewer requires the NanoGUI submodule "
            "to be present. Update your repository by calling the following:\n"
            "git submodule update --init --recursive")
    endif()

    set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
    set(NANOGUI_BUILD_SHARED OFF CACHE BOOL " " FORCE)
    set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL " " FORCE)
    set(NANOGUI_INSTALL OFF CACHE BOOL " " FORCE)

    # Locally disable additional warnings for NanoGUI and its dependencies
    set(PREV_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    set(PREV_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    if(MSVC)
        add_compile_options(-wd4389 -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DEIGEN_DONT_VECTORIZE)
    elseif(APPLE)
        add_compile_options(-Wno-objc-multiple-method-names -DGL_SILENCE_DEPRECATION)
    elseif(UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wno-format-truncation -Wno-implicit-fallthrough -Wno-int-in-bool-context
                            -Wno-maybe-uninitialized -Wno-misleading-indentation)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
    endif()

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI external/NanoGUI)
    set_property(TARGET nanogui nanogui-obj glfw glfw_objects PROPERTY FOLDER "External")

    set(NANOGUI_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../;${CMAKE_CURRENT_SOURCE_DIR}/NanoGUI/include")
    set(NANOGUI_LIBRARIES "nanogui")

    # Restore warnings for MaterialXView
    set(CMAKE_C_FLAGS ${PREV_CMAKE_C_FLAGS})
    set(CMAKE_CXX_FLAGS ${PREV_CMAKE_CXX_FLAGS})
endif()

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

add_definitions(${NANOGUI_EXTRA_DEFS})

add_executable(MaterialXView ${materialx_source} ${materialx_headers})

set(LIBS
    MaterialXView
    MaterialXFormat
    MaterialXGenGlsl
    MaterialXRenderGlsl)

if (MATERIALX_BUILD_GEN_OSL)
    LIST(APPEND LIBS MaterialXGenOsl)
endif()
if (MATERIALX_BUILD_GEN_MDL)
    LIST(APPEND LIBS MaterialXGenMdl)
endif()

target_link_libraries(
    ${LIBS}
    ${NANOGUI_LIBRARIES}
    ${NANOGUI_EXTRA_LIBS})

target_include_directories(
    MaterialXView
    PRIVATE
    ${NANOGUI_INCLUDE_DIRS}
    ${NANOGUI_EXTRA_INCS})

if(MATERIALX_BUILD_OIIO AND OPENIMAGEIO_ROOT_DIR)
    add_custom_command(TARGET MaterialXView POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${OPENIMAGEIO_ROOT_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR})
endif()

install(TARGETS MaterialXView
    EXPORT MaterialX
    RUNTIME DESTINATION bin)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/MaterialXView.pdb"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/" OPTIONAL)
