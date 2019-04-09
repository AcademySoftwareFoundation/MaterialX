//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXRender/Handlers/HwLightHandler.h>

namespace MaterialX
{

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

void HwLightHandler::mapNodeDefToIdentiers(const std::vector<NodePtr>& nodes,
                                           std::unordered_map<string, unsigned int>& ids)
{
    unsigned int id = 1;
    for (auto node : nodes)
    {
        auto nodedef = node->getNodeDef();
        if (nodedef)
        {
            const string& name = nodedef->getName();
            if (!ids.count(name))
            {
                ids[name] = id++;
            }
        }
    }
}

void HwLightHandler::findLights(DocumentPtr doc, std::vector<NodePtr>& lights)
{
    lights.clear();
    for (NodePtr node : doc->getNodes())
    {
        const TypeDesc* type = TypeDesc::get(node->getType());
        if (type == Type::LIGHTSHADER)
        {
            lights.push_back(node);
        }
    }
}

void HwLightHandler::registerLights(DocumentPtr doc, const std::vector<NodePtr>& lights, GenContext& context)
{
    // Clear context light user data which is set when bindLightShader() 
    // is called. This is necessary in case the light types have already been
    // registered.
    HwShaderGenerator::unbindLightShaders(context);

    if (!lights.empty())
    {
        // Create a list of unique nodedefs and ids for them
        mapNodeDefToIdentiers(lights, _lightIdentifierMap);
        for (auto id : _lightIdentifierMap)
        {
            NodeDefPtr nodeDef = doc->getNodeDef(id.first);
            if (nodeDef)
            {
                HwShaderGenerator::bindLightShader(*nodeDef, id.second, context);
            }
        }
    }

    // Clamp the number of light sources to the number registered
    unsigned int lightSourceCount = static_cast<unsigned int>(lights.size());
    context.getOptions().hwMaxActiveLightSources = lightSourceCount;
}

}
