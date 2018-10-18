#ifndef MATERIALX_SWIZZLE_H
#define MATERIALX_SWIZZLE_H

#include <MaterialXGenShader/ShaderImplementation.h>

namespace MaterialX
{

/// Implementation of swizzle node
class Swizzle : public ShaderImplementation
{
public:
    static ShaderImplementationPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
