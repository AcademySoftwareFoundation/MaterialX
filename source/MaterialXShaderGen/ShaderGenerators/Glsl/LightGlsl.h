#ifndef MATERIALX_LIGHTGLSL_H
#define MATERIALX_LIGHTGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'light' node for GLSL
class LightGlsl : public GlslImplementation
{
  public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
