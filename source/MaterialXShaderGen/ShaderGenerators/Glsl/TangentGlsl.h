#ifndef MATERIALX_TANGENTGLSL_H
#define MATERIALX_TANGENTGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'tangent' node for GLSL
class TangentGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr creator();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
