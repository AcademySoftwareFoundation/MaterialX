#ifndef MATERIALX_VIEWDIRECTIONGLSL_H
#define MATERIALX_VIEWDIRECTIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of view direction for GLSL
class ViewDirectionGlsl : public GlslImplementation
{
public:
    static ShaderImplementationPtr create();

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
