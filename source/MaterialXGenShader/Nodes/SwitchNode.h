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

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
