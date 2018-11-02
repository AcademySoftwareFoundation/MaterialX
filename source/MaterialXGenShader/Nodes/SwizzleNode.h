#ifndef MATERIALX_SWIZZLENODE_H
#define MATERIALX_SWIZZLENODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of swizzle node
class SwizzleNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

    /// Returns true if the input is editable by users.
    /// Editable inputs are allowed to be published as shader uniforms
    /// and hence must be presentable in a user interface.
    bool isEditable(const ShaderInput& input) const override;
};

} // namespace MaterialX

#endif
