#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public SgImplementation
{
public:
    static SgImplementationPtr creator();

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

public:
    static const vector<string> kInputNames;
};

} // namespace MaterialX

#endif
