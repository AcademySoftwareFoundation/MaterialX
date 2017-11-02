#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public NodeImplementation
{
public:
    static NodeImplementationPtr creator();

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;

public:
    static const vector<string> kInputNames;
};

} // namespace MaterialX

#endif
