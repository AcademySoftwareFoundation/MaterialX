#ifndef MATERIALX_FRAMENODEGLSL_H
#define MATERIALX_FRAMENODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of frame node for GLSL
class FrameNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;
};

} // namespace MaterialX

#endif
