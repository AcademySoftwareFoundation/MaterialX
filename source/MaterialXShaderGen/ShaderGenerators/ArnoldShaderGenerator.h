#ifndef MATERIALX_ARNOLDSHADERGENERATOR_H
#define MATERIALX_ARNOLDSHADERGENERATOR_H

#include <MaterialXShaderGen/ShaderGenerators/OslShaderGenerator.h>

namespace MaterialX
{

/// An OSL shader generator targeting the Arnold renderer
class ArnoldShaderGenerator : public OslShaderGenerator
{
    DECLARE_SHADER_GENERATOR(ArnoldShaderGenerator)
public:
    ArnoldShaderGenerator() : OslShaderGenerator() {}
    Shader::VDirection getTargetVDirection() const override;
};

}

#endif
