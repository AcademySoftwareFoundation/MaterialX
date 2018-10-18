#ifndef MATERIALX_NORMALGLSL_H
#define MATERIALX_NORMALGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'normal' node for GLSL
class NormalGlsl : public GlslImplementation
{
public:
    static ShaderImplementationPtr create();

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
