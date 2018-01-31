#ifndef MATERIALX_NORMALOGSFX_H
#define MATERIALX_NORMALOGSFX_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'normal' node for OgsFx
class NormalOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerVariables(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
