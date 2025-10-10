//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGCONTEXT_H
#define MATERIALX_SLANGCONTEXT_H

/// @file
/// Slang context

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangBlit.h>
#include <MaterialXRenderSlang/SlangRhi.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

#include <iostream>
#include <optional>
#include <memory>

MATERIALX_NAMESPACE_BEGIN

using SlangContextPtr = std::shared_ptr<class SlangContext>;
using SlangBlitPtr = std::shared_ptr<class SlangBlit>;
class SlangShaderCache;

class SlangContext
{
  public:
    SlangContext(std::string_view deviceType = "Default");
    ~SlangContext();

    static SlangContextPtr create(std::string_view deviceType = "Default");

    const rhi::ComPtr<rhi::IDevice>& getDevice() { return _device; }

    SlangCommandEncoderPtr createCommandEncoder()
    {
        return _queue->createCommandEncoder();
    }

    void submitCommandEncoder(SlangCommandEncoderPtr commandEncoder)
    {
        _queue->submit(commandEncoder->finish());
    }

    void waitOnHost()
    {
        _queue->waitOnHost();
    }

    // Deduplicates base name of slang modules
    std::string deduplicateName(const std::string& name)
    {
        auto it = _nameToFrequency.find(name);
        if (it == _nameToFrequency.end())
        {
            _nameToFrequency[name] = 1;
            return name;
        }

        std::string result = name + "_" + std::to_string(it->second);
        it->second++;
        // Make sure we did not accidentally create an existing used name.
        // For example "foo" -> "foo_1", but "foo_1" could already be registered by the user.
        return deduplicateName(result);
    }

  public:
    rhi::ComPtr<rhi::IDevice> _device;
    rhi::ComPtr<rhi::ICommandQueue> _queue;
    SlangBlitPtr _blitter;

    slang::IGlobalSession* _slangGlobalSession;
    // How many times has the name already appeared.
    std::map<std::string, int> _nameToFrequency;

    rhi::ComPtr<SlangShaderCache> _shaderCache;

    class SlangDebugCallback : public rhi::IDebugCallback
    {
      public:
        virtual SLANG_NO_THROW void SLANG_MCALL
        handleMessage(rhi::DebugMessageType type, rhi::DebugMessageSource source, const char* message)
        {
            std::string msg;
            switch (source)
            {
                case rhi::DebugMessageSource::Layer:
                    msg += "Layer: ";
                    break;
                case rhi::DebugMessageSource::Driver:
                    msg += "Driver: ";
                    break;
                case rhi::DebugMessageSource::Slang:
                    msg += "Slang: ";
                    break;
            }

            switch (type)
            {
                case rhi::DebugMessageType::Info:
                    msg += "(Info): ";
                    break;
                case rhi::DebugMessageType::Warning:
                    msg += "(Warning): ";
                    break;
                case rhi::DebugMessageType::Error:
                    msg += "(Error): ";
                    break;
            }

            std::cout << msg << message << std::endl;
        }
    };

    std::unique_ptr<SlangDebugCallback> _debugCallback;
};

MATERIALX_NAMESPACE_END

#endif
