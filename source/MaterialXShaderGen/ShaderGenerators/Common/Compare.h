#ifndef MATERIALX_COMPARE_H
#define MATERIALX_COMPARE_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of compare node
class Compare : public SgImplementation
{
public:
    static SgImplementationPtr create();

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
