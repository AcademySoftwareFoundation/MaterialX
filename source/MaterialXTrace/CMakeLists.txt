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
    LIBRARIES
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
    if(WIN32)
        # ws2_32: Windows Sockets 2 library. Perfetto uses Winsock internally for
        # inter-process communication (IPC), even in single-process tracing mode.
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
        # Prevent Windows.h min/max macros from breaking std::numeric_limits
        # Also need /bigobj because perfetto.cc has too many sections for MSVC
        set_source_files_properties("${perfetto_SOURCE_DIR}/sdk/perfetto.cc"
            PROPERTIES 
            COMPILE_DEFINITIONS "NOMINMAX;WIN32_LEAN_AND_MEAN"
            COMPILE_OPTIONS "/bigobj")
    endif()
endif()

