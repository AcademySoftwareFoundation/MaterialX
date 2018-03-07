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

    SgImplementationPtr impl;

    // Find the implementation in the document (graph or implementation element)
    vector<InterfaceElementPtr> elements = nodeDef.getDocument()->getMatchingImplementations(nodeDef.getName());
    for (InterfaceElementPtr element : elements)
    {
        if (element->isA<NodeGraph>())
        {
            NodeGraphPtr implGraph = element->asA<NodeGraph>();
            const string& matchingTarget = implGraph->getTarget();
            if (matchingTarget.empty() || matchingTarget == getTarget())
            {
                impl = getImplementation(implGraph);
                break;
            }
        }
        else
        {
            ImplementationPtr implElement = element->asA<Implementation>();
            const string& matchingTarget = implElement->getTarget();
            if (implElement->getLanguage() == getLanguage() && (matchingTarget.empty() || matchingTarget == getTarget()))
            {
                impl = getImplementation(implElement);
                break;
            }
        }
    }

    if (!impl)
    {
        throw ExceptionShaderGenError("Could not find a matching implementation for node '" + nodeDef.getNodeString() +
            "' matching language '" + getLanguage() + "' and target '" + getTarget() + "'");
    }
    
    _boundLightShaders[lightTypeId] = impl;
}

SgImplementation* HwShaderGenerator::getBoundLightShader(size_t lightTypeId)
{
    auto it = _boundLightShaders.find(lightTypeId);
    return it != _boundLightShaders.end() ? it->second.get() : nullptr;
}

}
