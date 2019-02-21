#ifndef MATERIALX_NUMLIGHTSNODEGLSL_H
#define MATERIALX_NUMLIGHTSNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Utility node for getting number of active lights for GLSL.
class NumLightsNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionDefinition(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;
};

} // namespace MaterialX

#endif
