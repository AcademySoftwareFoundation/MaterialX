#ifndef MATERIALX_POSITION_H
#define MATERIALX_POSITION_H

#include <MaterialXShaderGen/Implementations/OgsFxImplementation.h>

namespace MaterialX
{

/// Implementation of 'position' node for OgsFx
class PositionOgsFx : public OgsFxImplementation
{
public:
    static SgImplementationPtr creator();

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
