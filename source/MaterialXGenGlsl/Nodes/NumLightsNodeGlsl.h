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

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionDefinition(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;
};

} // namespace MaterialX

#endif
