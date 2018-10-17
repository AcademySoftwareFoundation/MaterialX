#ifndef MATERIALX_SWIZZLE_H
#define MATERIALX_SWIZZLE_H

#include <MaterialXGenShader/GenImplementation.h>

namespace MaterialX
{

/// Implementation of swizzle node
class Swizzle : public GenImplementation
{
public:
    static GenImplementationPtr create();

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
