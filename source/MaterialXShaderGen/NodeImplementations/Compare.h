#ifndef MATERIALX_COMPARE_H
#define MATERIALX_COMPARE_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of compare node
class Compare : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(Compare)
public:
    void emitCode(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
