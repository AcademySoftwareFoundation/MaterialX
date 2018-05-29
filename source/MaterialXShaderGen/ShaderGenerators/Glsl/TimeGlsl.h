#ifndef MATERIALX_TIMEGLSL_H
#define MATERIALX_TIMEGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of time node for GLSL
class TimeGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr create();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, const SgNodeContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
