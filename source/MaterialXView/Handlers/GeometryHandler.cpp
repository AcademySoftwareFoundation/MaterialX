#include <MaterialXView/Handlers/GeometryHandler.h>

namespace MaterialX
{ 
const std::string GeometryHandler::UNIT_QUAD("unit_quad");
const std::string GeometryHandler::POSITION_ATTRIBUTE("i_position");
const std::string GeometryHandler::NORMAL_ATTRIBUTE("i_normal");
const std::string GeometryHandler::TEXCOORD_ATTRIBUTE("i_texcoord");
const std::string GeometryHandler::TANGENT_ATTRIBUTE("i_tangent");
const std::string GeometryHandler::BITANGENT_ATTRIBUTE("i_bitangent");
const std::string GeometryHandler::COLOR_ATTRIBUTE("i_color");

GeometryHandler::GeometryHandler() :
    _identifier(UNIT_QUAD)
{
}

GeometryHandler::~GeometryHandler() 
{
}

void GeometryHandler::setIdentifier(const std::string& identifier)
{
    if (identifier != _identifier)
    {
        _identifier = identifier;
    }
}

}