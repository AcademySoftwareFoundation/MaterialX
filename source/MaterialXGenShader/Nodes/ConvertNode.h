#ifndef MATERIALX_CONVERTNODE_H
#define MATERIALX_CONVERTNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of convert node
class ConvertNode : public ShaderNodeImpl
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen) const override;
};

} // namespace MaterialX

#endif
