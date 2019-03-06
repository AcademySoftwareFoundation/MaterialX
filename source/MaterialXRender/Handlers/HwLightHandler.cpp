//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

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
