#ifndef MATERIALX_LIGHTNODEGLSL_H
#define MATERIALX_LIGHTNODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'light' node for GLSL
class LightNodeGlsl : public GlslImplementation
{
  public:
    LightNodeGlsl();

    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

  private:
      HwClosureContextPtr _callEmission;
};

} // namespace MaterialX

#endif
