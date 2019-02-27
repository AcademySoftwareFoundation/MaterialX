#ifndef MATERIALX_COMPARENODE_H
#define MATERIALX_COMPARENODE_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of compare node
class CompareNode : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
