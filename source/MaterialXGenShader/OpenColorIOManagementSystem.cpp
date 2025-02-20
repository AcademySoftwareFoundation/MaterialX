//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OCIO

#include "MaterialXCore/Library.h"

#include <MaterialXGenShader/OpenColorIOManagementSystem.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/OpenColorIONode.h>

#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorTypes.h>

#include <exception>
#include <stdexcept>
#include <map>
#include <string>

MATERIALX_NAMESPACE_BEGIN

const string OpenColorIOManagementSystem::IMPL_PREFIX = "IMPL_MXOCIO_";
const string OpenColorIOManagementSystem::ND_PREFIX = "ND_MXOCIO_";

namespace
{

const string CMS_NAME = "OpenColorIO";

// Remap from legacy color space names to their ACES 1.3 equivalents.
const std::map<string, string> COLOR_SPACE_REMAP = {
    { "gamma18", "Gamma 1.8 Rec.709 - Texture" },
    { "gamma22", "Gamma 2.2 Rec.709 - Texture" },
    { "gamma24", "Gamma 2.4 Rec.709 - Texture" },
    // TODO: Add support for adobergb and lin_adobergb
};

} // anonymous namespace

//
// OpenColorIOManagementSystem methods
//

OpenColorIOManagementSystemPtr OpenColorIOManagementSystem::create(const OCIO::ConstConfigRcPtr& config, const string& target)
{
    if (target != "genglsl" && target != "genmsl" && target != "genosl")
    {
        throw std::runtime_error("OCIO does not support this target");
    }

    return OpenColorIOManagementSystemPtr(new OpenColorIOManagementSystem(config, target));
}

OpenColorIOManagementSystem::OpenColorIOManagementSystem(const OCIO::ConstConfigRcPtr& config, const string& target) :
    DefaultColorManagementSystem(target),
    _target(target),
    _config(std::move(config))
{
}

const string& OpenColorIOManagementSystem::getName() const
{
    return CMS_NAME;
}

const char* OpenColorIOManagementSystem::getSupportedColorSpaceName(const char* colorSpace) const
{
    if (_config->getColorSpace(colorSpace))
    {
        return colorSpace;
    }

    auto remap = COLOR_SPACE_REMAP.find(colorSpace);
    if (remap != COLOR_SPACE_REMAP.end())
    {
        return getSupportedColorSpaceName(remap->second.c_str());
    }

    auto cgConfig = OCIO::Config::CreateFromBuiltinConfig("ocio://studio-config-latest");
    try
    {
        // Throws on failure. Try at least two configs:
        return OCIO::Config::IdentifyBuiltinColorSpace(_config, cgConfig, colorSpace);
    }
    catch (const std::exception& /*e*/)
    {
        return nullptr;
    }
}

NodeDefPtr OpenColorIOManagementSystem::getNodeDef(const ColorSpaceTransform& transform) const
{
    // See if the default color management system already handles this:
    if (auto cmNodeDef = DefaultColorManagementSystem::getNodeDef(transform)) {
        return cmNodeDef;
    }

    OCIO::ConstProcessorRcPtr processor;
    // Check if directly supported in the config:
    const char* sourceColorSpace = getSupportedColorSpaceName(transform.sourceSpace.c_str());
    const char* targetColorSpace = getSupportedColorSpaceName(transform.targetSpace.c_str());
    if (!sourceColorSpace || !targetColorSpace)
    {
        return {};
    }

    try
    {
        processor = _config->getProcessor(sourceColorSpace, targetColorSpace);
    }
    catch (const std::exception& /*e*/)
    {
        return {};
    }

    if (!processor)
    {
        return {};
    }

    auto gpuProcessor = processor->getDefaultGPUProcessor();
    if (!gpuProcessor)
    {
        return {};
    }

    if (gpuProcessor->isNoOp())
    {
        return _document->getNodeDef("ND_dot_" + transform.type.getName());
    }

    // Reject transforms requiring textures (1D and 3D LUTs)
    OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
    gpuProcessor->extractGpuShaderInfo(shaderDesc);
    if (shaderDesc->getNum3DTextures() || shaderDesc->getNumTextures())
    {
        // TODO: Support LUTs
        return {};
    }

    static const auto NODE_NAME = string{ "ocio_color_conversion" };
    const auto functionName = NODE_NAME + "_" + processor->getCacheID();
    const auto implName = IMPL_PREFIX + functionName + "_" + transform.type.getName();
    const auto nodeDefName = ND_PREFIX + functionName + "_" + transform.type.getName();
    auto nodeDef = _document->getNodeDef(nodeDefName);
    if (!nodeDef)
    {
        nodeDef = _document->addNodeDef(nodeDefName, "", functionName);
        nodeDef->setNodeGroup("colortransform");

        nodeDef->addInput("in", transform.type.getName());
        nodeDef->addOutput("out", transform.type.getName());

        auto implementation = _document->addImplementation(implName);
        implementation->setTarget(_target);
        implementation->setNodeDef(nodeDef);
    }

    _implementations.emplace(implName, gpuProcessor);

    return nodeDef;
}

bool OpenColorIOManagementSystem::hasImplementation(const string& implName) const
{
    if (DefaultColorManagementSystem::hasImplementation(implName)) {
        return true;
    }
    return _implementations.count(implName);
}

ShaderNodeImplPtr OpenColorIOManagementSystem::createImplementation(const string& implName) const
{
    if (auto impl = DefaultColorManagementSystem::createImplementation(implName)) {
        return impl;
    }

    if (_implementations.count(implName))
    {
        return OpenColorIONode::create();
    }
    return {};
}

OCIO::ConstGPUProcessorRcPtr OpenColorIOManagementSystem::getGpuProcessor(const string& implName)
{
    auto it = _implementations.find(implName);

    return it != _implementations.end() ? it->second : nullptr;
}

MATERIALX_NAMESPACE_END
#endif
