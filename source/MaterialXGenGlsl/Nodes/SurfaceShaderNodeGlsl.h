#ifndef MATERIALX_SURFACESHADERNODEGLSL_H
#define MATERIALX_SURFACESHADERNODEGLSL_H

#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

namespace MaterialX
{

/// Implementation of surface shaders for GLSL.
/// Used for all surface shaders implemented in source code.
class SurfaceShaderNodeGlsl : public SourceCodeNode
{
  public:
    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;
};

} // namespace MaterialX

#endif
