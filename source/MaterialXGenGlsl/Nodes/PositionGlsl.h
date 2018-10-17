#ifndef MATERIALX_POSITIONGLSL_H
#define MATERIALX_POSITIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'position' node for GLSL
class PositionGlsl : public GlslImplementation
{
public:
    static GenImplementationPtr create();

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
