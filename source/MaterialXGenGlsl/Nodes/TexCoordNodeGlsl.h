#ifndef MATERIALX_TEXCOORDNODEGLSL_H
#define MATERIALX_TEXCOORDNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'texcoord' node for GLSL
class TexCoordNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;
};

} // namespace MaterialX

#endif
