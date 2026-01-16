# MaterialXTrace - Performance tracing infrastructure for MaterialX
#
# This module provides an abstract tracing interface that can be backed by
# different implementations (Perfetto, USD TraceCollector, etc.).

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# Exclude Perfetto sink when tracing is disabled
if(NOT MATERIALX_BUILD_TRACING)
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/Tracing.cpp")
    list(REMOVE_ITEM materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/PerfettoSink.cpp")
    # Keep headers but they'll be no-ops via macros
endif()

mx_add_library(MaterialXTrace
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    EXPORT_DEFINE
        MATERIALX_TRACE_EXPORTS
    MTLX_MODULES
        MaterialXCore)

# Perfetto tracing support
if(MATERIALX_BUILD_TRACING)
    # The Perfetto SDK is distributed as an amalgamated single-file source (sdk/perfetto.cc).
    # This is the official recommended integration method per Google's documentation:
    # https://perfetto.dev/docs/instrumentation/tracing-sdk
    # Compiling directly (vs. linking a library) simplifies integration and enables
    # better compiler optimizations within a single translation unit.
    target_sources(${TARGET_NAME} PRIVATE 
        "${perfetto_SOURCE_DIR}/sdk/perfetto.cc")
    # Use generator expression - only needed at build time, not install
    target_include_directories(${TARGET_NAME} PUBLIC 
        $<BUILD_INTERFACE:${perfetto_SOURCE_DIR}/sdk>)
    target_compile_definitions(${TARGET_NAME} PUBLIC MATERIALX_BUILD_TRACING)
    # For shared library builds, Perfetto needs proper symbol visibility
    if(BUILD_SHARED_LIBS OR MATERIALX_BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME} PRIVATE PERFETTO_COMPONENT_EXPORT=MX_TRACE_API)
    endif()
    if(WIN32)
        # ws2_32: Windows Sockets 2 library. Perfetto uses Winsock internally for
        # inter-process communication (IPC), even in single-process tracing mode.
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
        # Prevent Windows.h min/max macros from breaking std::numeric_limits
        # Also need /bigobj because perfetto.cc has too many sections for MSVC
        # Note: Use COMPILE_FLAGS (not COMPILE_OPTIONS) for source file properties
        set_source_files_properties("${perfetto_SOURCE_DIR}/sdk/perfetto.cc"
            PROPERTIES 
            COMPILE_DEFINITIONS "NOMINMAX;WIN32_LEAN_AND_MEAN"
            COMPILE_FLAGS "/bigobj")
    elseif(UNIX)
        # Perfetto requires pthread on Linux/Unix for thread-local storage and synchronization
        # On iOS/macOS, threads are built-in to the system libraries
        if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
            find_package(Threads REQUIRED)
            target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
        endif()
        # Suppress warnings from Perfetto SDK (they use patterns that trigger GCC/Clang warnings)
        # This prevents build failures when MATERIALX_WARNINGS_AS_ERRORS is ON
        # Also ensure proper symbol visibility for shared library builds
        # Note: Use COMPILE_FLAGS (not COMPILE_OPTIONS) for source file properties
        set_source_files_properties("${perfetto_SOURCE_DIR}/sdk/perfetto.cc"
            PROPERTIES
            COMPILE_FLAGS "-Wno-error -fvisibility=default")
    endif()
endif()

