#ifndef MATERIALX_SURFACEGLSL_H
#define MATERIALX_SURFACEGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'surface' node for GLSL
class SurfaceGlsl : public GlslImplementation
{
  public:
    static SgImplementationPtr creator();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
