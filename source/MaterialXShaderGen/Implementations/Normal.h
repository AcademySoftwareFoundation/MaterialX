#ifndef MATERIALX_NORMAL_H
#define MATERIALX_NORMAL_H

#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

/// Implementation of 'normal' node for OgsFx
class NormalOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
