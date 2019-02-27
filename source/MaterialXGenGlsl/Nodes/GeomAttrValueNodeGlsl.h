#ifndef MATERIALX_GEOMATTRVALUENODEGLSL_H
#define MATERIALX_GEOMATTRVALUENODEGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'geomattrvalue' node for GLSL
class GeomAttrValueNodeGlsl : public GlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(Shader& shader, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;

    void emitFunctionCall(ShaderStage& stage, GenContext& context, const ShaderGenerator& shadergen, const ShaderNode& node) const override;
};

} // namespace MaterialX

#endif
