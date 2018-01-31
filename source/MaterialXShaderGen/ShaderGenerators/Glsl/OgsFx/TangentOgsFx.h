#ifndef MATERIALX_TANGENTOGSFX_H
#define MATERIALX_TANGENTOGSFX_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'tangent' node for OgsFx
class TangentOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
