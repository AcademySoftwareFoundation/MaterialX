#ifndef MATERIALX_TANGENTGLSL_H
#define MATERIALX_TANGENTGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'tangent' node for GLSL
class TangentGlsl : public GlslImplementation
{
public:
    static ShaderImplementationPtr create();

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
