#ifndef MATERIALX_SWITCH_H
#define MATERIALX_SWITCH_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of switch node
class Switch : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(Switch)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
