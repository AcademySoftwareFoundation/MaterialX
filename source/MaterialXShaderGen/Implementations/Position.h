#ifndef MATERIALX_POSITION_H
#define MATERIALX_POSITION_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of 'position' node for OgsFx
class PositionOgsFx : public SgImplementation
{
public:
    static SgImplementationPtr creator();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void registerInputs(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
    void emitFunctionCall(const SgNode& node, ShaderGenerator& shadergen, Shader& shader) override;
};

} // namespace MaterialX

#endif
