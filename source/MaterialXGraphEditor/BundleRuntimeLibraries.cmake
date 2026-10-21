# Copyright Contributors to the MaterialX Project
# SPDX-License-Identifier: Apache-2.0
#
# Copy the Cycles runtime shared libraries into a directory beside the
# executable.  The MaterialX libraries are intentionally skipped: the graph
# editor links them statically, and an installed Cycles package may also
# contain its own MaterialX build.
#
# Required -D arguments:
#   SOURCE_DIR   Directory containing the Cycles runtime libraries.
#   DEST_DIR     Directory to copy them into (normally the executable's dir).

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "BundleRuntimeLibraries: SOURCE_DIR is required.")
endif()
if(NOT DEFINED DEST_DIR OR "${DEST_DIR}" STREQUAL "")
    message(FATAL_ERROR "BundleRuntimeLibraries: DEST_DIR is required.")
endif()

file(MAKE_DIRECTORY "${DEST_DIR}")

file(GLOB _materialx_runtime_libs
    "${SOURCE_DIR}/*.so*"
    "${SOURCE_DIR}/*.dylib*"
    "${SOURCE_DIR}/*.dll")

foreach(_lib IN LISTS _materialx_runtime_libs)
    get_filename_component(_name "${_lib}" NAME)
    if(_name MATCHES "^libMaterialX")
        continue()
    endif()
    file(COPY "${_lib}" DESTINATION "${DEST_DIR}")
endforeach()
