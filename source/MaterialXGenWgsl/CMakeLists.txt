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

# Optional build-time regeneration of the genwgsl shader-node library from genglsl. This runs the
# offline transpiler (tools/mxgenwgsl.py) over libraries/ into the build tree purely for
# validation: transpile or naga errors -- and any previously-generable node that regresses -- make
# the build fail. The generated genwgsl .wgsl node files are NOT committed; they are produced from
# genglsl (the single source of truth) by CI, which runs the transpiler with `--out libraries` to
# populate the runtime library. This target is a decoupled local validation aid only (output goes to
# the build dir, not the source tree). Requires a Python interpreter and the `naga` CLI.
if(MATERIALX_GENERATE_WGSL_LIBRARY)
    if(MATERIALX_PYTHON_EXECUTABLE)
        set(_wgsl_python "${MATERIALX_PYTHON_EXECUTABLE}")
    else()
        find_package(Python3 COMPONENTS Interpreter)
        set(_wgsl_python "${Python3_EXECUTABLE}")
    endif()

    # Locate the naga CLI. Precedence: an explicit -D/cache value, then the NAGA env var, then a
    # search of PATH and the standard cargo bin directories. `cargo install naga-cli` lands naga in
    # ~/.cargo/bin (or $CARGO_HOME/bin), which is frequently NOT on PATH in CI / IDE / GUI shells --
    # so a developer who installed naga still hits "not found" without these hints.
    if(NOT MATERIALX_NAGA_EXECUTABLE AND DEFINED ENV{NAGA})
        set(MATERIALX_NAGA_EXECUTABLE "$ENV{NAGA}" CACHE FILEPATH
            "Path to the naga CLI used to transpile/validate the genwgsl library." FORCE)
    endif()
    find_program(MATERIALX_NAGA_EXECUTABLE NAMES naga naga.exe
                 HINTS
                     "$ENV{CARGO_HOME}/bin"      # custom cargo home (all platforms)
                     "$ENV{HOME}/.cargo/bin"     # default on Linux / macOS
                     "$ENV{USERPROFILE}/.cargo/bin"  # default on Windows
                 DOC "Path to the naga CLI used to transpile/validate the genwgsl library.")

    # If naga is still not found, obtain it via cargo. cargo is located EMSDK-style (mirrors
    # MATERIALX_EMSDK_PATH): an explicit MATERIALX_CARGO_PATH, then $CARGO_HOME, then the standard
    # cargo bin dirs, then PATH. cargo is assumed to already be installed -- we do NOT bootstrap
    # Rust. When found, naga is built into the build tree at *build* time (not configure, so
    # configure stays fast) and cached there across rebuilds.
    set(_wgsl_install_naga OFF)
    if(NOT MATERIALX_NAGA_EXECUTABLE)
        find_program(MATERIALX_CARGO_EXECUTABLE NAMES cargo cargo.exe
                     HINTS
                         "${MATERIALX_CARGO_PATH}/bin"   # explicit -DMATERIALX_CARGO_PATH
                         "$ENV{CARGO_HOME}/bin"          # custom cargo home
                         "$ENV{HOME}/.cargo/bin"         # default on Linux / macOS
                         "$ENV{USERPROFILE}/.cargo/bin"  # default on Windows
                     DOC "Path to the Rust cargo executable used to install the naga CLI.")
        if(MATERIALX_CARGO_EXECUTABLE)
            if(CMAKE_HOST_WIN32)
                set(_naga_exe "naga.exe")
            else()
                set(_naga_exe "naga")
            endif()
            set(_naga_root "${CMAKE_BINARY_DIR}/naga")
            # Predicted install location -- the file may not exist yet; a plain set() (not
            # find_program) so the generation target can reference it before it is built.
            set(MATERIALX_NAGA_EXECUTABLE "${_naga_root}/bin/${_naga_exe}")
            set(_wgsl_install_naga ON)
        endif()
    endif()

    if(_wgsl_python AND MATERIALX_NAGA_EXECUTABLE)
        # Build-time, one-time cargo install of naga-cli (only when resolved to a cargo location).
        # The custom-command OUTPUT is the naga binary, so it re-runs only when that file is missing.
        if(_wgsl_install_naga)
            add_custom_command(OUTPUT "${MATERIALX_NAGA_EXECUTABLE}"
                COMMAND "${MATERIALX_CARGO_EXECUTABLE}" install naga-cli --root "${_naga_root}"
                COMMENT "Installing naga-cli via cargo into ${_naga_root} (one-time; compiles from source)"
                VERBATIM)
            add_custom_target(MaterialXInstallNaga DEPENDS "${MATERIALX_NAGA_EXECUTABLE}")
        endif()

        add_custom_target(MaterialXGenWgslLibrary ALL
            COMMAND ${CMAKE_COMMAND} -E env "NAGA=${MATERIALX_NAGA_EXECUTABLE}"
                    "${_wgsl_python}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/mxgenwgsl.py"
                    --libraries "${PROJECT_SOURCE_DIR}/libraries"
                    --out "${CMAKE_BINARY_DIR}/genwgsl_generated"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "Transpiling genglsl shader-node library to genwgsl (validation)"
            VERBATIM)
        if(_wgsl_install_naga)
            add_dependencies(MaterialXGenWgslLibrary MaterialXInstallNaga)
        endif()
    else()
        message(WARNING
            "MATERIALX_GENERATE_WGSL_LIBRARY is ON but a prerequisite was not found "
            "(Python='${_wgsl_python}', naga='${MATERIALX_NAGA_EXECUTABLE}', "
            "cargo='${MATERIALX_CARGO_EXECUTABLE}'). Install naga (`cargo install naga-cli`) and point "
            "-DMATERIALX_NAGA_EXECUTABLE / the NAGA env var at it, or install cargo and set "
            "-DMATERIALX_CARGO_PATH to its home so naga can be built into the tree. "
            "Skipping genwgsl library generation.")
    endif()
endif()
