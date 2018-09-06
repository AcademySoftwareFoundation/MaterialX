#ifndef MATERIALX_NORMALGLSL_H
#define MATERIALX_NORMALGLSL_H

#include <MaterialXGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'normal' node for GLSL
class NormalGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
