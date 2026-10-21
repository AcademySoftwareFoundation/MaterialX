# Copyright Contributors to the MaterialX Project
# SPDX-License-Identifier: Apache-2.0
#
# Bundle the runtime shared-library dependencies of a built executable into a
# destination directory (normally beside the executable).
#
# The Cycles precompiled libraries use DT_RUNPATH="$ORIGIN" and depend on
# siblings that live in other directories, so the consuming executable's rpath
# cannot resolve them. Copying the whole dependency closure into one directory
# makes every "$ORIGIN" resolve to that directory.
#
# This script is intended to be invoked with `cmake -P` after the target has
# been linked (for example from a POST_BUILD command).
#
# Required -D arguments:
#   EXECUTABLE   Absolute path to the executable to inspect.
#   DESTINATION  Directory to copy the resolved libraries into.
#
# Optional -D arguments:
#   EXTRA_LIBRARIES      Additional libraries whose dependencies are resolved.
#   DIRECTORIES          Extra directories to search while resolving.
#   COPY_FILES           Files to copy unconditionally (e.g. a SONAME symlink).
#   PRE_EXCLUDE_REGEXES  Regexes applied before dependency resolution.
#   POST_EXCLUDE_REGEXES Regexes applied after dependency resolution.

cmake_minimum_required(VERSION 3.26)

if(NOT DEFINED EXECUTABLE OR NOT EXISTS "${EXECUTABLE}")
    message(FATAL_ERROR "BundleCyclesRuntime: executable not found: '${EXECUTABLE}'")
endif()
if(NOT DEFINED DESTINATION OR "${DESTINATION}" STREQUAL "")
    message(FATAL_ERROR "BundleCyclesRuntime: DESTINATION is required.")
endif()

# Libraries that are always provided by the base system and must not be bundled.
if(NOT DEFINED PRE_EXCLUDE_REGEXES OR "${PRE_EXCLUDE_REGEXES}" STREQUAL "")
    set(PRE_EXCLUDE_REGEXES
        "ld-linux.*\\.so"
        "libc\\.so.*"
        "libdl\\.so.*"
        "libm\\.so.*"
        "libpthread\\.so.*"
        "librt\\.so.*"
        "libgcc_s\\.so.*"
        "libstdc\\+\\+\\.so.*")
endif()
if(NOT DEFINED POST_EXCLUDE_REGEXES OR "${POST_EXCLUDE_REGEXES}" STREQUAL "")
    set(POST_EXCLUDE_REGEXES
        "^/lib/"
        "^/lib64/"
        "^/usr/lib"
        "^/usr/local/lib"
        "^/System/Library/"
        "^[a-zA-Z]:/Windows/")
endif()

# The Cycles library itself lives in the build tree, not in the bundled
# precompiled directories, so add its location to the search path.
if(DEFINED CYCLES_LIBRARY_DIR AND NOT "${CYCLES_LIBRARY_DIR}" STREQUAL "")
    list(APPEND DIRECTORIES "${CYCLES_LIBRARY_DIR}")
endif()

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${EXECUTABLE}"
    LIBRARIES ${EXTRA_LIBRARIES}
    DIRECTORIES ${DIRECTORIES}
    PRE_EXCLUDE_REGEXES ${PRE_EXCLUDE_REGEXES}
    POST_EXCLUDE_REGEXES ${POST_EXCLUDE_REGEXES}
    CONFLICTING_DEPENDENCIES_PREFIX _materialx_runtime_conflicts
    RESOLVED_DEPENDENCIES_VAR _materialx_runtime_deps
    UNRESOLVED_DEPENDENCIES_VAR _materialx_runtime_unresolved)

foreach(_dep IN LISTS _materialx_runtime_unresolved)
    message(STATUS "BundleCyclesRuntime: unresolved dependency '${_dep}'")
endforeach()

file(MAKE_DIRECTORY "${DESTINATION}")

# Copy explicitly requested files first so SONAME symlink chains are present
# even if dependency resolution returned the versioned real file.
foreach(_file IN LISTS COPY_FILES)
    if(EXISTS "${_file}")
        file(COPY "${_file}" DESTINATION "${DESTINATION}" FOLLOW_SYMLINK_CHAIN)
    endif()
endforeach()

set(_copied 0)
foreach(_dep IN LISTS _materialx_runtime_deps)
    file(COPY "${_dep}" DESTINATION "${DESTINATION}" FOLLOW_SYMLINK_CHAIN)
    math(EXPR _copied "${_copied} + 1")
endforeach()

# OpenImageDenoise loads its per-device backends with dlopen at run time, so
# they are not reported by GET_RUNTIME_DEPENDENCIES. Copy the CPU backend used
# by the graph editor's denoiser.
foreach(_dir IN LISTS DIRECTORIES)
    file(GLOB _materialx_oidn_devices "${_dir}/libOpenImageDenoise_device_cpu.so*")
    foreach(_device IN LISTS _materialx_oidn_devices)
        file(COPY "${_device}" DESTINATION "${DESTINATION}" FOLLOW_SYMLINK_CHAIN)
        math(EXPR _copied "${_copied} + 1")
    endforeach()
endforeach()

message(STATUS "BundleCyclesRuntime: copied ${_copied} runtime libraries into ${DESTINATION}")
