#ifndef MATERIALX_GEOMCOLORGLSL_H
#define MATERIALX_GEOMCOLORGLSL_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'geomcolor' node for GLSL
class GeomColorGlsl : public GlslImplementation
{
public:
    static SgImplementationPtr creator();

    void createVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
