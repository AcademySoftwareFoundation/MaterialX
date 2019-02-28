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

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

} // namespace MaterialX

#endif
