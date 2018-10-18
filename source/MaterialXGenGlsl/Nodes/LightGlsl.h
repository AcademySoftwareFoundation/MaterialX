#ifndef MATERIALX_LIGHTGLSL_H
#define MATERIALX_LIGHTGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'light' node for GLSL
class LightGlsl : public GlslImplementation
{
  public:
    static ShaderImplementationPtr create();

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
