#include <MaterialXView/Handlers/GeometryHandler.h>

namespace MaterialX
{ 
const std::string GeometryHandler::SCREEN_ALIGNED_QUAD("screen_quad");

GeometryHandler::GeometryHandler() :
    _identifier(SCREEN_ALIGNED_QUAD)
{
}

GeometryHandler::~GeometryHandler() 
{
}

void GeometryHandler::setIdentifier(const std::string identifier)
{
    if (identifier != _identifier)
    {
        _identifier = identifier;
    }
}

}