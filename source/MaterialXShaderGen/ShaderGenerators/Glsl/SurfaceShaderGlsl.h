#ifndef MATERIALX_SURFACESHADERGLSL_H
#define MATERIALX_SURFACESHADERGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Common/SourceCode.h>

namespace MaterialX
{

/// Implementation of surface shaders for GLSL.
/// Used for all surface shaders implemented in source code.
class SurfaceShaderGlsl : public SourceCode
{
  public:
    static SgImplementationPtr creator();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    bool isTransparent(const SgNode& node) const override;
};

} // namespace MaterialX

#endif
