#ifndef MATERIALX_COMPARENODEIMPL_H
#define MATERIALX_COMPARENODEIMPL_H

#include <MaterialXGenShader/ShaderNodeImpl.h>

namespace MaterialX
{

/// Implementation of compare node
class CompareNodeImpl : public ShaderNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
