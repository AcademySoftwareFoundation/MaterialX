#ifndef MATERIALX_VIEWDIRECTIONGLSL_H
#define MATERIALX_VIEWDIRECTIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of view direction for GLSL
class ViewDirectionGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
