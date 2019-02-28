#ifndef MATERIALX_TRANSFORMNODEGLSL_H
#define MATERIALX_TRANSFORMNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of transforms (point/vector/normal) for GLSL
class TransformNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;
};

} // namespace MaterialX

#endif
