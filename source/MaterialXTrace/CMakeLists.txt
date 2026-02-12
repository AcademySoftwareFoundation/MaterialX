# MaterialXTrace - Performance tracing infrastructure for MaterialX
#
# This module provides an abstract tracing interface that can be backed by
# different implementations (Perfetto, USD TraceCollector, etc.).

file(GLOB materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Public headers only - PerfettoSink.h is internal (includes Perfetto SDK headers)
set(materialx_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Tracing.h
)

# Exclude Perfetto sink when tracing is disabled
if(NOT MATERIALX_BUILD_PERFETTO_TRACING)
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
if(MATERIALX_BUILD_PERFETTO_TRACING)
    # The Perfetto SDK is distributed as an amalgamated single-file source (sdk/perfetto.cc).
    # This is the official recommended integration method per Google's documentation:
    # https://perfetto.dev/docs/instrumentation/tracing-sdk
    # Compiling directly (vs. linking a library) simplifies integration and enables
    # better compiler optimizations within a single translation unit.
    #
    # NOTE: Compile flags for perfetto.cc are set in root CMakeLists.txt so they apply
    # to all targets (including monolithic builds which aggregate sources).
    target_sources(${TARGET_NAME} PRIVATE 
        "${perfetto_SOURCE_DIR}/sdk/perfetto.cc")
    # Use generator expression - only needed at build time, not install
    target_include_directories(${TARGET_NAME} PUBLIC 
        $<BUILD_INTERFACE:${perfetto_SOURCE_DIR}/sdk>)
    target_compile_definitions(${TARGET_NAME} PUBLIC MATERIALX_BUILD_PERFETTO_TRACING)
    
    # Platform-specific link libraries
    if(WIN32)
        # ws2_32: Windows Sockets 2 library for Perfetto IPC
        target_link_libraries(${TARGET_NAME} PRIVATE ws2_32)
    elseif(UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # Perfetto requires pthread on Linux/Unix (but not iOS where it's built-in)
        find_package(Threads REQUIRED)
        target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
    endif()
    
    # Apply Perfetto compile flags (defined in root CMakeLists.txt)
    # NOTE: set_source_files_properties is directory-scoped, so we must set them
    # here for non-monolithic builds AND in root CMakeLists.txt for monolithic builds.
    set_source_files_properties("${perfetto_SOURCE_DIR}/sdk/perfetto.cc"
        PROPERTIES
        COMPILE_DEFINITIONS "${MATERIALX_PERFETTO_COMPILE_DEFINITIONS}"
        COMPILE_FLAGS "${MATERIALX_PERFETTO_COMPILE_FLAGS}")
endif()

