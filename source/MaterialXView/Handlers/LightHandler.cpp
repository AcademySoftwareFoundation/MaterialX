#include <MaterialXView/Handlers/LightHandler.h>
#include <MaterialXShaderGen/HwShaderGenerator.h>

namespace MaterialX
{ 

LightSource::LightSource(LightSource::Type type, const NodeDef& nodeDef)
    : _type(type)
{
    for (auto port : nodeDef.getChildrenOfType<ValueElement>())
    {
        _parameters[port->getName()] = port->getValue();
    }
}

LightHandler::LightHandler()
{
}

LightHandler::~LightHandler()
{
}

LightSourcePtr LightHandler::createLightSource(ConstNodeDefPtr nodeDef)
{
    LightSource::Type type;

    LightShaderMap::const_iterator it = _lightShaders.find(nodeDef);
    if (it != _lightShaders.end())
    {
        // Already registed, get the light type id
        type = it->second;
    }
    else
    {
        // Register new light shader, and get light type id
        _lightShaders[nodeDef] = type = _lightShaders.size();
    }

    LightSourcePtr light = LightSourcePtr(new LightSource(type, *nodeDef));
    _lightSources.push_back(light);

    return light;
}

void LightHandler::bindLightShaders(HwShaderGenerator& shadergen) const
{
    for (auto shader : _lightShaders)
    {
        shadergen.bindLightShader(*shader.first, shader.second);
    }

    // Extend max lights limit if needed
    shadergen.setMaxActiveLightSources(
        std::max(_lightShaders.size(), shadergen.getMaxActiveLightSources())
    );
}

}
