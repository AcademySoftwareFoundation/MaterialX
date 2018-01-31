#ifndef MATERIALX_TEXCOORD_H
#define MATERIALX_TEXCOORD_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>

namespace MaterialX
{

/// Implementation of 'texcoord' node for OgsFx
class TexCoordOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
