#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

unsigned int HwLightHandler::getLightType(NodePtr node)
{
    size_t hash = std::hash<std::string>{}(node->getCategory());

    // If we're on a 64-bit platform, convert to 32-bits
    bool on64BitPlatform = sizeof(hash) == 8;
    if (on64BitPlatform)
    {
        // Convert hash to 32-bits
        return static_cast<unsigned int>(hash & 0xFFFFFFFF) ^
               static_cast<unsigned int>((static_cast<unsigned long long>(hash) >> 32) & 0xFFFFFFFF);
    }
    else
    {
        return static_cast<unsigned int>(hash);
    }
}

HwLightHandler::HwLightHandler()
{
}

HwLightHandler::~HwLightHandler()
{
}

void HwLightHandler::addLightSource(NodePtr node)
{
    _lightSources.push_back(node);
}

void HwLightHandler::bindLightShaders(HwShaderGenerator& shadergen, const GenOptions& options) const
{
    for (auto lightSource : _lightSources)
    {
        shadergen.bindLightShader(*lightSource->getNodeDef(), getLightType(lightSource), options);
    }
}

}
