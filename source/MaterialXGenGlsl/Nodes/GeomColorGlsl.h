#ifndef MATERIALX_GEOMCOLORGLSL_H
#define MATERIALX_GEOMCOLORGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'geomcolor' node for GLSL
class GeomColorGlsl : public GlslImplementation
{
public:
    static GenImplementationPtr create();

    void createVariables(const DagNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const DagNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
