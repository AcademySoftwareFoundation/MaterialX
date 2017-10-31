#ifndef MATERIALX_SWIZZLE_H
#define MATERIALX_SWIZZLE_H

#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

/// Implementation of constant node
class Constant : public NodeImplementation
{
    DECLARE_NODE_IMPLEMENTATION(Constant)
public:
    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
