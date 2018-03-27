#include <MaterialXShaderGen/HwShaderGenerator.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Definition.h>

namespace MaterialX
{

HwShaderGenerator::HwShaderGenerator(SyntaxPtr syntax)
    : ShaderGenerator(syntax)
    , _maxActiveLightSources(3)
{
}

void HwShaderGenerator::bindLightShader(const NodeDef& nodeDef, size_t lightTypeId)
{
    if (nodeDef.getType() != DataType::LIGHT)
    {
        throw ExceptionShaderGenError("Error binding light shader. Given nodedef '" + nodeDef.getName() + "' is not of lightshader type");
    }

    if (getBoundLightShader(lightTypeId))
    {
        throw ExceptionShaderGenError("Error binding light shader. Light type id '" + std::to_string(lightTypeId) + "' has already been bound");
    }

    SgImplementationPtr sgimpl;

    // Find the implementation for this nodedef
    InterfaceElementPtr impl = nodeDef.getImplementation(getTarget(), getLanguage());
    if (impl)
    {
        sgimpl = getImplementation(impl);
    }
    if (!sgimpl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + getLanguage() + "' and target '" + getTarget() + "'");
    }
    
    _boundLightShaders[lightTypeId] = sgimpl;
}

SgImplementation* HwShaderGenerator::getBoundLightShader(size_t lightTypeId)
{
    auto it = _boundLightShaders.find(lightTypeId);
    return it != _boundLightShaders.end() ? it->second.get() : nullptr;
}

}
