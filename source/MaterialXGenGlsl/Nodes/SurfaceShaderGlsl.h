#ifndef MATERIALX_SURFACESHADERGLSL_H
#define MATERIALX_SURFACESHADERGLSL_H

#include <MaterialXGenShader/Nodes/SourceCode.h>

namespace MaterialX
{

/// Implementation of surface shaders for GLSL.
/// Used for all surface shaders implemented in source code.
class SurfaceShaderGlsl : public SourceCode
{
  public:
    static ShaderImplementationPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
