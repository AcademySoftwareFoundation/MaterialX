#ifndef MATERIALX_GEOMATTRVALUEGLSL_H
#define MATERIALX_GEOMATTRVALUEGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'geomattrvalue' node for GLSL
class GeomAttrValueGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr creator();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
