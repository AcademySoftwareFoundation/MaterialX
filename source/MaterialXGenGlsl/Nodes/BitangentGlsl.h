#ifndef MATERIALX_BITANGENTGLSL_H
#define MATERIALX_BITANGENTGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'bitangent' node for GLSL
class BitangentGlsl : public GlslImplementation
{
public:
    static GenImplementationPtr create();

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
