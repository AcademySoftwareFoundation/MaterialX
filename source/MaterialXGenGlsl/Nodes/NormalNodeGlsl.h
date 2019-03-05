#ifndef MATERIALX_NORMALNODEGLSL_H
#define MATERIALX_NORMALNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'normal' node for GLSL
class NormalNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
