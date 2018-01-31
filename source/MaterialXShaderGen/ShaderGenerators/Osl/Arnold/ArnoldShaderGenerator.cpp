#include <MaterialXShaderGen/ShaderGenerators/Osl/Arnold/ArnoldShaderGenerator.h>

namespace MaterialX
{

const string ArnoldShaderGenerator::TARGET = "arnold";

Shader::VDirection ArnoldShaderGenerator::getTargetVDirection() const
{
    return Shader::VDirection::DOWN;
}

}
