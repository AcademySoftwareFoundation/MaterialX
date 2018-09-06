#ifndef MATERIALX_FRAMEGLSL_H
#define MATERIALX_FRAMEGLSL_H

#include <MaterialXGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of frame node for GLSL
class FrameGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
