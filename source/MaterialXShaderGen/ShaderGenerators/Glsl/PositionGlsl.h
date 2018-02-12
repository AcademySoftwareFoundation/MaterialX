#ifndef MATERIALX_POSITIONGLSL_H
#define MATERIALX_POSITIONGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'position' node for GLSL
class PositionGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr creator();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
