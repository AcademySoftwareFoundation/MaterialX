//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangContext.h>
#include <MaterialXRenderSlang/SlangTypeUtils.h>
#include <MaterialXRenderSlang/SlangBlit.h>
#include <MaterialXRenderSlang/SlangShaderCache.h>
#include <string>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
    #pragma push_macro("NOMINMAX")
    #undef NOMINMAX
    #define NOMINMAX
    #include <d3d12.h>
    #include <comdef.h>
    #pragma pop_macro("NOMINMAX")
#endif

MATERIALX_NAMESPACE_BEGIN

SlangContext::SlangContext(std::string_view deviceType)
{
    using namespace rhi;

    rhi::getRHI()->enableDebugLayers();
    slang::createGlobalSession(&_slangGlobalSession);

    _debugCallback = std::make_unique<SlangDebugCallback>();
    _shaderCache = new SlangShaderCache("./shadercache");

    DeviceDesc deviceDesc = {};
    if (deviceType == "Default")
    {
        deviceDesc.deviceType = DeviceType::Default;
    }
    else if (deviceType == "D3D12")
    {
        deviceDesc.deviceType = DeviceType::D3D12;
    }
    else if (deviceType == "Vulkan")
    {
        deviceDesc.deviceType = DeviceType::Vulkan;
    }
    else if (deviceType == "Metal")
    {
        deviceDesc.deviceType = DeviceType::Metal;
    }
    else if (deviceType == "WGPU")
    {
        deviceDesc.deviceType = DeviceType::WGPU;
    }
    else
    {
        throw ExceptionRenderError("Unknown deviceType request: " + std::string(deviceType));
    }

    std::vector<const char*> searchPaths;
    std::vector<slang::PreprocessorMacroDesc> preprocessorMacros;
    std::vector<slang::CompilerOptionEntry> compilerOptions;

    slang::CompilerOptionEntry compilerOption;
    compilerOption.name = slang::CompilerOptionName::EmitSpirvDirectly;
    compilerOption.value.intValue0 = 1;
    compilerOptions.push_back(compilerOption);

    deviceDesc.slang.slangGlobalSession = _slangGlobalSession;
    deviceDesc.slang.searchPaths = searchPaths.data();
    deviceDesc.slang.searchPathCount = (uint32_t) searchPaths.size();
    deviceDesc.slang.preprocessorMacros = preprocessorMacros.data();
    deviceDesc.slang.preprocessorMacroCount = (uint32_t) preprocessorMacros.size();
    deviceDesc.slang.compilerOptionEntries = compilerOptions.data();
    deviceDesc.slang.compilerOptionEntryCount = (uint32_t) compilerOptions.size();
    deviceDesc.debugCallback = _debugCallback.get();
    deviceDesc.persistentShaderCache = _shaderCache.get();

#ifdef _DEBUG
    deviceDesc.enableValidation = true;
    rhi::getRHI()->enableDebugLayers();
#endif

    rhi::getRHI()->createDevice(deviceDesc, _device.writeRef());

    _queue = _device->getQueue(rhi::QueueType::Graphics);
    _blitter = std::make_shared<SlangBlit>(this);
}

SlangContext::~SlangContext()
{
    _blitter.reset();
}

SlangContextPtr SlangContext::create(std::string_view deviceType)
{
    return std::make_shared<SlangContext>(deviceType);
}

MATERIALX_NAMESPACE_END
