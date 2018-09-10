#ifndef MATERIALX_SWIZZLE_H
#define MATERIALX_SWIZZLE_H

#include <MaterialXGenShader/SgImplementation.h>

namespace MaterialX
{

/// Implementation of swizzle node
class Swizzle : public SgImplementation
{
public:
    static SgImplementationPtr create();

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
