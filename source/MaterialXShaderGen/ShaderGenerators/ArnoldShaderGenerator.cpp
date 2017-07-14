#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>

namespace MaterialX
{

DEFINE_SHADER_GENERATOR(ArnoldShaderGenerator, "osl", "arnold")

Shader::VDirection ArnoldShaderGenerator::getTargetVDirection() const
{
    return Shader::VDirection::DOWN;
}

}
