#ifndef MATERIALX_VIEWDIRECTIONGLSL_H
#define MATERIALX_VIEWDIRECTIONGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of view direction for GLSL
class ViewDirectionGlsl : public GlslImplementation
{
public:
    static GenImplementationPtr create();

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
