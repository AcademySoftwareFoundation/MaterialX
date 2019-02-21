#ifndef MATERIALX_SWITCHNODE_H
#define MATERIALX_SWITCHNODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of switch node
class SwitchNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context) const override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
