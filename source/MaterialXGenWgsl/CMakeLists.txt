file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXGenWgsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXGenHw
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_GENWGSL_EXPORTS)

# ---------------------------------------------------------------------------
# Optional build-time regeneration of the genwgsl library from genglsl.
#
# MATERIALX_BUILD_GEN_WGSL      -> output in-place to libraries/  (CI, JS builds)
# MATERIALX_GENERATE_WGSL_LIBRARY -> output to build tree only    (local validation)
#
# Requires Python and naga.  Optional tree-sitter cleanup deps are not installed
# by CMake; mxgenwgsl.py skips readability cleanup when tree-sitter is absent.
# CI installs source/MaterialXGenWgsl/tools/requirements-transpile.txt when needed.
# ---------------------------------------------------------------------------

set(MATERIALX_NAGA_VERSION "30.0.0" CACHE STRING
    "Version of naga-cli to install via cargo for WGSL library generation.")

set(_wgsl_generate OFF)
set(_wgsl_out_dir  "")
if(MATERIALX_BUILD_GEN_WGSL)
    set(_wgsl_generate ON)
    set(_wgsl_out_dir "${PROJECT_SOURCE_DIR}/libraries")
elseif(MATERIALX_GENERATE_WGSL_LIBRARY)
    set(_wgsl_generate ON)
    set(_wgsl_out_dir "${CMAKE_BINARY_DIR}/genwgsl_generated")
endif()

if(NOT _wgsl_generate)
    return()
endif()

# -- Python ----------------------------------------------------------------

if(MATERIALX_PYTHON_EXECUTABLE)
    set(_wgsl_python "${MATERIALX_PYTHON_EXECUTABLE}")
    execute_process(
        COMMAND "${_wgsl_python}" -c
                "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"
        OUTPUT_VARIABLE _wgsl_py_ver
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _wgsl_py_ver_rc)
else()
    find_package(Python3 COMPONENTS Interpreter)
    set(_wgsl_python  "${Python3_EXECUTABLE}")
    set(_wgsl_py_ver  "${Python3_VERSION}")
    set(_wgsl_py_ver_rc 0)
endif()

# -- Naga ------------------------------------------------------------------
# Precedence: explicit -D, NAGA env var, PATH + standard cargo bin dirs.

if(NOT MATERIALX_NAGA_EXECUTABLE AND DEFINED ENV{NAGA})
    set(MATERIALX_NAGA_EXECUTABLE "$ENV{NAGA}" CACHE FILEPATH
        "Path to the naga CLI for genwgsl transpilation." FORCE)
endif()

find_program(MATERIALX_NAGA_EXECUTABLE NAMES naga naga.exe
    HINTS
        "$ENV{CARGO_HOME}/bin"
        "$ENV{HOME}/.cargo/bin"
        "$ENV{USERPROFILE}/.cargo/bin"
    DOC "Path to the naga CLI for genwgsl transpilation.")

# If naga is still missing, try to install it via cargo at build time.
set(_wgsl_install_naga OFF)
if(NOT MATERIALX_NAGA_EXECUTABLE)
    find_program(MATERIALX_CARGO_EXECUTABLE NAMES cargo cargo.exe
        HINTS
            "${MATERIALX_CARGO_PATH}/bin"
            "$ENV{CARGO_HOME}/bin"
            "$ENV{HOME}/.cargo/bin"
            "$ENV{USERPROFILE}/.cargo/bin"
        DOC "Path to the Rust cargo executable (used to install naga-cli).")
    if(MATERIALX_CARGO_EXECUTABLE)
        set(_naga_root "${CMAKE_BINARY_DIR}/naga")
        if(CMAKE_HOST_WIN32)
            set(MATERIALX_NAGA_EXECUTABLE "${_naga_root}/bin/naga.exe")
        else()
            set(MATERIALX_NAGA_EXECUTABLE "${_naga_root}/bin/naga")
        endif()
        set(_wgsl_install_naga ON)
    endif()
endif()

# -- Guard: both Python and naga are required ------------------------------

if(NOT _wgsl_python OR NOT MATERIALX_NAGA_EXECUTABLE)
    message(WARNING
        "WGSL library generation was requested but a prerequisite was not found "
        "(Python='${_wgsl_python}', naga='${MATERIALX_NAGA_EXECUTABLE}', "
        "cargo='${MATERIALX_CARGO_EXECUTABLE}'). "
        "Skipping genwgsl library generation.")
    return()
endif()

# -- Optional dependency targets -------------------------------------------

# Naga: build-time cargo install (only when resolved to a cargo location).
if(_wgsl_install_naga)
    add_custom_command(OUTPUT "${MATERIALX_NAGA_EXECUTABLE}"
        COMMAND "${MATERIALX_CARGO_EXECUTABLE}" install naga-cli
                --locked --version ${MATERIALX_NAGA_VERSION} --root "${_naga_root}"
        COMMENT "Installing naga-cli ${MATERIALX_NAGA_VERSION} via cargo (one-time)"
        VERBATIM)
    add_custom_target(MaterialXInstallNaga DEPENDS "${MATERIALX_NAGA_EXECUTABLE}")
endif()

if(_wgsl_py_ver_rc EQUAL 0 AND _wgsl_py_ver VERSION_LESS "3.10")
    message(STATUS
        "MaterialXGenWgsl: Python ${_wgsl_py_ver} - tree-sitter cleanup unavailable "
        "(tree-sitter-language-pack requires 3.10+); mxgenwgsl.py will emit verbose WGSL")
endif()

# -- Main transpile target -------------------------------------------------

add_custom_target(MaterialXGenWgslLibrary ALL
    COMMAND ${CMAKE_COMMAND} -E env "NAGA=${MATERIALX_NAGA_EXECUTABLE}"
            "${_wgsl_python}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/mxgenwgsl.py"
            --libraries "${PROJECT_SOURCE_DIR}/libraries"
            --out "${_wgsl_out_dir}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Transpiling genglsl to genwgsl (${_wgsl_out_dir})"
    VERBATIM)

if(_wgsl_install_naga)
    add_dependencies(MaterialXGenWgslLibrary MaterialXInstallNaga)
endif()
