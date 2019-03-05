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

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
