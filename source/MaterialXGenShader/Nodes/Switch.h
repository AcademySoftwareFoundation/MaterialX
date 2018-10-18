#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXGenShader/ShaderImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public ShaderImplementation
{
public:
    static ShaderImplementationPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
