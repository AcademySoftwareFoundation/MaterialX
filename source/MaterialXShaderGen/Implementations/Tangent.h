#ifndef MATERIALX_TANGENT_H
#define MATERIALX_TANGENT_H

#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

/// Implementation of 'tangent' node for OgsFx
class TangentOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
