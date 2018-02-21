#include <MaterialXShaderGen/HwShaderGenerator.h>

namespace MaterialX
{

HwShaderGenerator::HwShaderGenerator(SyntaxPtr syntax)
    : ShaderGenerator(syntax)
    , _maxActiveLightSources(3)
{
}

void HwShaderGenerator::bindLightShader(size_t lightTypeId, SgImplementationPtr impl)
{
    if (getBoundLightShader(lightTypeId))
    {
        throw ExceptionShaderGenError("Light type id '" + std::to_string(lightTypeId) + "' has already been bound");
    }
    _boundLightShaders[lightTypeId] = impl;
}

SgImplementation* HwShaderGenerator::getBoundLightShader(size_t lightTypeId)
{
    auto it = _boundLightShaders.find(lightTypeId);
    return it != _boundLightShaders.end() ? it->second.get() : nullptr;
}

}
