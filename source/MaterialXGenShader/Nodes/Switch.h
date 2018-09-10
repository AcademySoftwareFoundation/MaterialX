#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXGenShader/SgImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public SgImplementation
{
public:
    static SgImplementationPtr create();

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> INPUT_NAMES;
};

} // namespace MaterialX

#endif
