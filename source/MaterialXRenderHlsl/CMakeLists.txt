file(GLOB_RECURSE materialx_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE materialx_headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")

mx_add_library(MaterialXRenderHlsl
    SOURCE_FILES
        ${materialx_source}
    HEADER_FILES
        ${materialx_headers}
    MTLX_MODULES
        MaterialXRender
        MaterialXGenHlsl
        MaterialXGenShader
        MaterialXFormat
        MaterialXCore
    EXPORT_DEFINE
        MATERIALX_RENDERHLSL_EXPORTS)

# Link against the Windows SDK D3D libraries we use directly:
#   d3dcompiler  - FXC backend (D3DCompile, D3DReflect)
#   d3d11        - HlslContext device + immediate context
# The dxcompiler entry points used for the DXC backend are loaded
# dynamically via LoadLibrary at first use, so we deliberately do NOT
# add dxcompiler to the link line - that would create a hard build-time
# dependency on the DXC redistributable. The dynamic load still works on
# any system that has dxcompiler.dll on PATH or alongside the executable.
target_link_libraries(MaterialXRenderHlsl PUBLIC d3dcompiler d3d11)

# Locate dxcompiler.dll / dxil.dll in the Windows SDK and stage them next
# to consumer binaries (currently MaterialXTest) so the DXC backend can
# load them without requiring callers to alter PATH. Lookup is best-
# effort: if the DLLs are not found the DXC code path returns a clean
# "dxcompiler.dll not found" diagnostic at runtime instead of failing.
if(WIN32)
    # Probe likely Windows SDK locations for the DXC redistributable. The
    # SDK ships dxcompiler under bin/<sdk_version>/<arch>/. We don't make
    # the build depend on finding it; if absent the runtime DXC code path
    # surfaces a clean diagnostic.
    set(_mx_sdk_arch "x64")
    if(CMAKE_GENERATOR_PLATFORM)
        string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _mx_sdk_arch)
    endif()
    set(_mx_sdk_search_paths)
    if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
        list(APPEND _mx_sdk_search_paths
            "$ENV{WindowsSdkDir}bin/$ENV{WindowsSDKVersion}/${_mx_sdk_arch}")
    endif()
    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        list(APPEND _mx_sdk_search_paths
            "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/${_mx_sdk_arch}")
    endif()
    list(APPEND _mx_sdk_search_paths
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/${_mx_sdk_arch}"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/${_mx_sdk_arch}"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.20348.0/${_mx_sdk_arch}")

    find_file(MATERIALX_HLSL_DXCOMPILER_DLL
        NAMES dxcompiler.dll
        PATHS ${_mx_sdk_search_paths}
        NO_DEFAULT_PATH
        DOC "Full path to dxcompiler.dll for DXC backend support.")
    find_file(MATERIALX_HLSL_DXIL_DLL
        NAMES dxil.dll
        PATHS ${_mx_sdk_search_paths}
        NO_DEFAULT_PATH
        DOC "Full path to dxil.dll for DXC backend support.")
    if(MATERIALX_HLSL_DXCOMPILER_DLL)
        message(STATUS "MaterialXRenderHlsl: DXC found at ${MATERIALX_HLSL_DXCOMPILER_DLL}")
    else()
        message(STATUS "MaterialXRenderHlsl: dxcompiler.dll not located; DXC backend tests will be skipped at runtime")
    endif()
endif()
