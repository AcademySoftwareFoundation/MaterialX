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

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

  private:
      HwClosureContextPtr _callEmission;
};

} // namespace MaterialX

#endif
