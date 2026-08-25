//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OCIO

#include "MaterialXCore/Library.h"

#include <MaterialXGenShader/OcioColorManagementSystem.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Nodes/OcioNode.h>

#include <OpenColorIO/OpenColorABI.h>
#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorTypes.h>

#include <exception>
#include <stdexcept>
#include <map>

namespace OCIO = OCIO_NAMESPACE;

MATERIALX_NAMESPACE_BEGIN

const string OcioColorManagementSystem::IMPL_PREFIX = "IMPL_MXOCIO_";
const string OcioColorManagementSystem::OCIO_SOURCE_URI = "materialx://OcioColorManagementSystem.cpp";
const string ND_PREFIX = "ND_MXOCIO_";

namespace
{

const string CMS_NAME = "OpenColorIO";

// Remap from legacy color space names to their ACES 1.3 equivalents.
const std::map<string, string> COLOR_SPACE_REMAP =
{
    { "gamma18", "Gamma 1.8 Rec.709 - Texture" },
    { "gamma22", "Gamma 2.2 Rec.709 - Texture" },
    { "gamma24", "Gamma 2.4 Rec.709 - Texture" },
};

} // anonymous namespace

//
// OcioColorManagementSystemImpl class and methods
//

class OcioColorManagementSystemImpl
{
  public:
    OcioColorManagementSystemImpl(OCIO::ConstConfigRcPtr config, string target) :
        _config(std::move(config)), _target(std::move(target)) { }

    const char* getSupportedColorSpaceName(const char* colorSpace) const;

    bool isNoOpColorSpace(const string& colorSpace) const;

    string getUserFacingName(const string& colorSpace) const;

    NodeDefPtr getNodeDef(const ColorSpaceTransform& transform, const DocumentPtr& document) const;

    bool hasImplementation(const string& implName) const
    {
        return _implementations.count(implName);
    }

    ShaderNodeImplPtr createImplementation(const string& implName) const
    {
        if (_implementations.count(implName))
        {
            return OcioNode::create();
        }
        return {};
    }

    string getGpuProcessorCode(const string& implName, const string& functionName) const;

    const string& target() const { return _target; }

  private:
    OCIO::ConstConfigRcPtr _config;
    string _target;
    mutable std::map<string, OCIO::ConstGPUProcessorRcPtr> _implementations;

    // Lazily-created built-in config used to identify color spaces that are not
    // directly recognized by the user's config (see getSupportedColorSpaceName).
    mutable OCIO::ConstConfigRcPtr _cgConfig;

    // Memo of color space names already resolved by getSupportedColorSpaceName,
    // keyed by the requested name. An empty value denotes a name that could not
    // be resolved. Without this cache, a color space that requires the built-in
    // config fallback below would re-parse that config on every lookup, and
    // populateColorTransformMap performs up to four such lookups per color port.
    mutable std::map<string, string> _resolvedColorSpaceNames;
};

const char* OcioColorManagementSystemImpl::getSupportedColorSpaceName(const char* colorSpace) const
{
    auto cached = _resolvedColorSpaceNames.find(colorSpace);
    if (cached != _resolvedColorSpaceNames.end())
    {
        return cached->second.empty() ? nullptr : cached->second.c_str();
    }

    string resolved;
    if (_config->getColorSpace(colorSpace))
    {
        resolved = colorSpace;
    }
    else if (auto remap = COLOR_SPACE_REMAP.find(colorSpace); remap != COLOR_SPACE_REMAP.end())
    {
        if (const char* remapped = getSupportedColorSpaceName(remap->second.c_str()))
        {
            resolved = remapped;
        }
    }
    else
    {
        // Check if the requested name is known in the built-in ACES config. If so, then
        // look to see if an equivalent color space is available in the user's config by
        // checking the math of each color space implementation.
        if (!_cgConfig)
        {
            _cgConfig = OCIO::Config::CreateFromBuiltinConfig("ocio://cg-config-latest");
        }
        try
        {
            // Throws on failure:
            resolved = OCIO::Config::IdentifyBuiltinColorSpace(_config, _cgConfig, colorSpace);
        }
        catch (const std::exception& /*e*/)
        {
        }
    }

    string& cacheEntry = _resolvedColorSpaceNames[colorSpace];
    cacheEntry = resolved;
    return cacheEntry.empty() ? nullptr : cacheEntry.c_str();
}

bool OcioColorManagementSystemImpl::isNoOpColorSpace(const string& colorSpace) const
{
    const char* supportedColorSpace = getSupportedColorSpaceName(colorSpace.c_str());
    if (!supportedColorSpace)
    {
        return false;
    }

    OCIO::ConstColorSpaceRcPtr colorSpaceObj = _config->getColorSpace(supportedColorSpace);
    return colorSpaceObj && colorSpaceObj->isData();
}

string OcioColorManagementSystemImpl::getUserFacingName(const string& colorSpace) const
{
    const char* supportedColorSpace = getSupportedColorSpaceName(colorSpace.c_str());
    if (!supportedColorSpace)
    {
        return colorSpace;
    }

    const char* canonicalName = _config->getCanonicalName(supportedColorSpace);
    return canonicalName ? canonicalName : colorSpace;
}

NodeDefPtr OcioColorManagementSystemImpl::getNodeDef(const ColorSpaceTransform& transform, const DocumentPtr& document) const
{
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
        return document->getNodeDef("ND_dot_" + transform.type.getName());
    }

    // Reject transforms requiring textures (1D and 3D LUTs)
    OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
    gpuProcessor->extractGpuShaderInfo(shaderDesc);
    if (shaderDesc->getNum3DTextures() || shaderDesc->getNumTextures())
    {
        // TODO: Support LUTs
        return {};
    }

    // Build a function name from the source/target color space names and the first
    // six characters of the cache ID.  Each name component is sanitized individually:
    // invalid characters are replaced with underscores and consecutive underscores are
    // collapsed to one so that OCIO's own identifier normalizer cannot produce a
    // different name than the one we register on the node def.
    auto sanitizeName = [](const string& s) -> string
    {
        string result = createValidName(s);
        string collapsed;
        bool prevUnderscore = false;
        for (char c : result)
        {
            if (c == '_')
            {
                if (!prevUnderscore)
                    collapsed += c;
                prevUnderscore = true;
            }
            else
            {
                collapsed += c;
                prevUnderscore = false;
            }
        }
        return collapsed;
    };
    static const auto NODE_NAME = string{ "ocio_color_conversion" };
    const string cacheID = processor->getCacheID();
    const auto functionName = NODE_NAME + "_" + sanitizeName(sourceColorSpace) + "_to_" + sanitizeName(targetColorSpace) + "_" + cacheID.substr(0, 6);
    const auto implName = OcioColorManagementSystem::IMPL_PREFIX + functionName + "_" + transform.type.getName();
    const auto nodeDefName = ND_PREFIX + functionName + "_" + transform.type.getName();
    auto nodeDef = document->getNodeDef(nodeDefName);
    if (!nodeDef)
    {
        nodeDef = document->addNodeDef(nodeDefName, "", functionName);
        nodeDef->setNodeGroup("colortransform");
        nodeDef->setSourceUri(OcioColorManagementSystem::OCIO_SOURCE_URI);

        nodeDef->addInput("in", transform.type.getName());
        nodeDef->addOutput("out", transform.type.getName());

        auto implementation = document->addImplementation(implName);
        implementation->setTarget(_target);
        implementation->setNodeDef(nodeDef);
        implementation->setSourceUri(OcioColorManagementSystem::OCIO_SOURCE_URI);
    }

    _implementations.emplace(implName, gpuProcessor);

    return nodeDef;
}

string OcioColorManagementSystemImpl::getGpuProcessorCode(const string& implName, const string& functionName) const
{
    auto it = _implementations.find(implName);
    if (it == _implementations.end())
    {
        return {};
    }

    auto gpuProcessor = it->second;
    OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();

    // TODO: Extend to essl and MDL and possibly SLang.
    bool isOSL = false;
    if (_target == "genglsl")
    {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
    }
    else if (_target == "genmsl")
    {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_MSL_2_0);
    }
    else if (_target == "genosl")
    {
        shaderDesc->setLanguage(OCIO::LANGUAGE_OSL_1);
        isOSL = true;
    }

    shaderDesc->setFunctionName(functionName.c_str());

    gpuProcessor->extractGpuShaderInfo(shaderDesc);

    string shaderText = shaderDesc->getShaderText();

    // For OSL, we need to extract the function from the shader OCIO creates.
    if (isOSL)
    {
        auto startpos = shaderText.find(string{ "color4 " } + shaderDesc->getFunctionName());
        if (startpos != string::npos)
        {
            auto endpos = shaderText.find(string{ "outColor = " } + shaderDesc->getFunctionName(), startpos);
            if (endpos != string::npos)
            {
                shaderText = shaderText.substr(startpos, endpos - startpos);
            }
        }
#if OCIO_VERSION_HEX <= 0x02040100
        // Need to transpose the matrix if we have an OCIO < 2.4.2 see:
        //    https://github.com/AcademySoftwareFoundation/OpenColorIO/pull/2121
        startpos = shaderText.find(string{ "matrix(" });
        if (startpos != string::npos)
        {
            auto endpos = shaderText.find(string{ ")" }, startpos);
            if (endpos != string::npos)
            {
                shaderText.insert(endpos, ")");
                shaderText.insert(startpos, "transpose(");
            }
        }
#endif
    }

    return shaderText;
}

//
// OcioColorManagementSystem methods
//

OcioColorManagementSystemPtr OcioColorManagementSystem::createFromEnv(string target)
{
    if (target != "genglsl" && target != "genmsl" && target != "genosl")
    {
        throw std::runtime_error("OCIO does not support this target");
    }

    auto config = OCIO::Config::CreateFromEnv();
    return OcioColorManagementSystemPtr(new OcioColorManagementSystem(new OcioColorManagementSystemImpl(config, target)));
}

OcioColorManagementSystemPtr OcioColorManagementSystem::createFromFile(const string& filename, string target)
{
    if (target != "genglsl" && target != "genmsl" && target != "genosl")
    {
        throw std::runtime_error("OCIO does not support this target");
    }

    auto config = OCIO::Config::CreateFromFile(filename.c_str());
    return OcioColorManagementSystemPtr(new OcioColorManagementSystem(new OcioColorManagementSystemImpl(config, target)));
}

OcioColorManagementSystemPtr OcioColorManagementSystem::createFromBuiltinConfig(const string& configName, string target)
{
    if (target != "genglsl" && target != "genmsl" && target != "genosl")
    {
        throw std::runtime_error("OCIO does not support this target");
    }

    auto config = OCIO::Config::CreateFromBuiltinConfig(configName.c_str());
    return OcioColorManagementSystemPtr(new OcioColorManagementSystem(new OcioColorManagementSystemImpl(config, target)));
}

OcioColorManagementSystem::OcioColorManagementSystem(OcioColorManagementSystemImpl* impl) :
    DefaultColorManagementSystem(impl->target()),
    _impl(impl)
{
}

OcioColorManagementSystem::~OcioColorManagementSystem() = default;

const string& OcioColorManagementSystem::getName() const
{
    return CMS_NAME;
}

const char* OcioColorManagementSystem::getSupportedColorSpaceName(const char* colorSpace) const
{
    return _impl->getSupportedColorSpaceName(colorSpace);
}

NodeDefPtr OcioColorManagementSystem::getNodeDef(const ColorSpaceTransform& transform) const
{
    // See if the default color management system already handles this:
    if (auto cmNodeDef = DefaultColorManagementSystem::getNodeDef(transform))
    {
        return cmNodeDef;
    }

    return _impl->getNodeDef(transform, _document);
}

bool OcioColorManagementSystem::isNoOpColorSpace(const string& colorSpace) const
{
    if (isReservedNoOpColorSpace(colorSpace))
    {
        return true;
    }
    return _impl->isNoOpColorSpace(colorSpace);
}

string OcioColorManagementSystem::getUserFacingName(const string& colorSpace) const
{
    // The config author's name takes precedence over the DefaultColorManagementSystem names.
    string name = _impl->getUserFacingName(colorSpace);
    if (name != colorSpace)
    {
        return name;
    }
    return DefaultColorManagementSystem::getUserFacingName(colorSpace);
}

bool OcioColorManagementSystem::hasImplementation(const string& implName) const
{
    if (DefaultColorManagementSystem::hasImplementation(implName))
    {
        return true;
    }
    return _impl->hasImplementation(implName);
}

ShaderNodeImplPtr OcioColorManagementSystem::createImplementation(const string& implName) const
{
    if (auto impl = DefaultColorManagementSystem::createImplementation(implName))
    {
        return impl;
    }

    return _impl->createImplementation(implName);
}

string OcioColorManagementSystem::getGpuProcessorCode(const string& implName, const string& functionName) const
{
    return _impl->getGpuProcessorCode(implName, functionName);
}

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO
