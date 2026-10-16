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

if(MATERIALX_BUILD_OCIO)
    find_package(OpenColorIO REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenColorIO::OpenColorIO)
    target_compile_definitions(${TARGET_NAME} PUBLIC MATERIALX_BUILD_OCIO)
endif()

# ---------------------------------------------------------------------------
# Optional build-time regeneration of the genwgsl library from genglsl.
#
# MATERIALX_BUILD_GEN_WGSL      -> output in-place to libraries/  (CI, JS builds)
# MATERIALX_GENERATE_WGSL_LIBRARY -> output to build tree only    (local validation)
#
# Requires Python and naga.  Optional tree-sitter tidy deps are not installed
# by CMake; mxgenwgsl.py skips the tidy pass when tree-sitter is absent.
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

# If naga is still missing, try to install it via cargo at build time. Python wheel builds skip
# this: they use the pre-generated library shipped in the sdist rather than compiling naga.
set(_wgsl_install_naga OFF)
if(NOT MATERIALX_NAGA_EXECUTABLE AND NOT SKBUILD)
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
    set(_wgsl_missing "Python='${_wgsl_python}', naga='${MATERIALX_NAGA_EXECUTABLE}', cargo='${MATERIALX_CARGO_EXECUTABLE}'")
    if(NOT MATERIALX_BUILD_GEN_WGSL)
        message(WARNING
            "WGSL library generation was requested but a prerequisite was not found "
            "(${_wgsl_missing}). Skipping genwgsl library generation.")
        return()
    endif()
    set(_wgsl_install_hint "Install naga-cli ${MATERIALX_NAGA_VERSION} or set MATERIALX_NAGA_EXECUTABLE")
    set(_wgsl_error_prefix "MATERIALX_BUILD_GEN_WGSL requires Python and naga to generate the genwgsl library (${_wgsl_missing})")

    # Without the transpile tools, the library must already be complete: every genglsl node and
    # lib file needs a genwgsl counterpart (generated, or hand-written and committed).
    # mxgenwgsl.py enforces the same mapping at the end of each full run (checkOutputCoverage),
    # so the two rules can't drift apart silently.
    file(GLOB _wgsl_glsl_sources RELATIVE "${PROJECT_SOURCE_DIR}/libraries"
        "${PROJECT_SOURCE_DIR}/libraries/*/genglsl/mx_*.glsl"
        "${PROJECT_SOURCE_DIR}/libraries/*/genglsl/lib/*.glsl")
    set(_wgsl_missing_files "")
    foreach(_glsl IN LISTS _wgsl_glsl_sources)
        string(REPLACE "/genglsl/" "/genwgsl/" _wgsl_file "${_glsl}")
        string(REGEX REPLACE "\\.glsl$" ".wgsl" _wgsl_file "${_wgsl_file}")
        if(NOT EXISTS "${_wgsl_out_dir}/${_wgsl_file}")
            list(APPEND _wgsl_missing_files "${_wgsl_file}")
        endif()
    endforeach()
    # The baked OCIO fragments come from a separate optional target and aren't covered by the
    # generator's coverage check, so their absence only warrants a warning.
    if(MATERIALX_BUILD_OCIO)
        file(GLOB _wgsl_ocio_files "${_wgsl_out_dir}/stdlib/genwgsl/ocio/*.wgsl")
        if(NOT _wgsl_ocio_files)
            message(WARNING
                "MaterialXGenWgsl: Python or naga not found and no baked OCIO fragments exist "
                "under ${_wgsl_out_dir}/stdlib/genwgsl/ocio, so WGSL color management nodes "
                "won't resolve. ${_wgsl_install_hint} to bake them.")
        endif()
    endif()
    if(_wgsl_missing_files)
        list(LENGTH _wgsl_missing_files _wgsl_missing_count)
        list(GET _wgsl_missing_files 0 _wgsl_missing_example)
        message(FATAL_ERROR
            "${_wgsl_error_prefix}, and the existing library under ${_wgsl_out_dir} is missing or "
            "incomplete (${_wgsl_missing_count} file(s) missing, e.g. ${_wgsl_missing_example}), "
            "so there is nothing to reuse. ${_wgsl_install_hint} to generate it.")
    endif()

    # Generated .wgsl files are git-ignored, so a complete library can't be verified as current:
    # it may be the one shipped in the Python sdist, or one left over from an older checkout.
    message(WARNING
        "MaterialXGenWgsl: Python or naga not found; using the existing genwgsl library under "
        "${_wgsl_out_dir} as is; it can't be verified as current. ${_wgsl_install_hint} to "
        "regenerate it.")
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
        "MaterialXGenWgsl: Python ${_wgsl_py_ver} - tree-sitter tidy pass unavailable "
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

# -- OCIO: pre-bake GpuShaderDesc GLSL to stdlib/genwgsl/ocio/*.wgsl -----------------

if(MATERIALX_BUILD_OCIO)
    get_filename_component(_ocio_install_root "${OpenColorIO_DIR}/../../.." ABSOLUTE)
    set(_ocio_wgsl_out "${_wgsl_out_dir}/stdlib/genwgsl/ocio")
    set(_ocio_py_stage "${CMAKE_BINARY_DIR}/ocio_bake_py")
    file(TO_NATIVE_PATH "${_ocio_py_stage}" _ocio_py_stage_native)
    add_custom_target(MaterialXGenWgslOcioLibrary
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_ocio_wgsl_out}"
        COMMAND "${_wgsl_python}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/mxstagepymaterialx.py"
                --repo "${PROJECT_SOURCE_DIR}"
                --pyd-dir "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
                --stage-root "${_ocio_py_stage}"
                --ocio-bin "${_ocio_install_root}/bin"
        COMMAND ${CMAKE_COMMAND} -E env
                "NAGA=${MATERIALX_NAGA_EXECUTABLE}"
                "PYTHONPATH=${_ocio_py_stage_native}"
                "${_wgsl_python}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/mxgenwgsl.py"
                --bake-ocio
                --repo "${PROJECT_SOURCE_DIR}"
                --out "${_ocio_wgsl_out}"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Baking OCIO WGSL fragments (${_ocio_wgsl_out})"
        VERBATIM)
    if(_wgsl_install_naga)
        add_dependencies(MaterialXGenWgslOcioLibrary MaterialXInstallNaga)
    endif()
    if(TARGET MaterialXGenWgslLibrary)
        add_dependencies(MaterialXGenWgslLibrary MaterialXGenWgslOcioLibrary)
    endif()
endif()
