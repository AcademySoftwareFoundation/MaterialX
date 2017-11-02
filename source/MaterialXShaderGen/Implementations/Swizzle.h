#ifndef MATERIALX_SWIZZLE_H
#define MATERIALX_SWIZZLE_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of swizzle node
class Swizzle : public SgImplementation
{
public:
    static SgImplementationPtr creator();

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader, int numArgs = 0, ...) override;
};

} // namespace MaterialX

#endif
