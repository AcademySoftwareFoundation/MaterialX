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

}
