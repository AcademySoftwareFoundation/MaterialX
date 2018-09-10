#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{ 

LightSource::LightSource(size_t typeId, const NodeDef& nodeDef)
    : _typeId(typeId)
{
    for (auto port : nodeDef.getChildrenOfType<ValueElement>())
    {
        _parameters[port->getName()] = port->getValue();
    }
}

HwLightHandler::HwLightHandler()
{
}

HwLightHandler::~HwLightHandler()
{
}

void HwLightHandler::addLightShader(size_t typeId, ConstNodeDefPtr nodeDef)
{
    _lightShaders[typeId] = nodeDef;
}

LightSourcePtr HwLightHandler::createLightSource(size_t typeId)
{
    LightShaderMap::const_iterator it = _lightShaders.find(typeId);
    if (it == _lightShaders.end())
    {
        throw ExceptionShaderGenError("Not light shader for type id '" + std::to_string(typeId) + "' exists");
    }

    LightSourcePtr light = LightSourcePtr(new LightSource(typeId, *it->second));
    _lightSources.push_back(light);

    return light;
}

void HwLightHandler::bindLightShaders(HwShaderGenerator& shadergen) const
{
    for (auto shader : _lightShaders)
    {
        shadergen.bindLightShader(*shader.second, shader.first);
    }
}

}
