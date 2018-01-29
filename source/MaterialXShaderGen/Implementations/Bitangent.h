#ifndef MATERIALX_BITANGENT_H
#define MATERIALX_BITANGENT_H

#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

/// Implementation of 'bitangent' node for OgsFx
class BitangentOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
