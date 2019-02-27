#ifndef MATERIALX_SURFACENODEGLSL_H
#define MATERIALX_SURFACENODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'surface' node for GLSL
class SurfaceNodeGlsl : public GlslImplementation
{
  public:
    SurfaceNodeGlsl();

    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

  private:
    /// Closure contexts for calling closure functions.
    HwClosureContextPtr _callReflection;
    HwClosureContextPtr _callTransmission;
    HwClosureContextPtr _callIndirect;
    HwClosureContextPtr _callEmission;
};

} // namespace MaterialX

#endif
