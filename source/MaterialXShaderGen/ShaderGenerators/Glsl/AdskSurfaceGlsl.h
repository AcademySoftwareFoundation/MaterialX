#ifndef MATERIALX_ADSKSURFACEGLSL_H
#define MATERIALX_ADSKSURFACEGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Common/SourceCode.h>

namespace MaterialX
{

/// Implementation of 'adskSurface' node for GLSL
class AdskSurfaceGlsl : public SourceCode
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
