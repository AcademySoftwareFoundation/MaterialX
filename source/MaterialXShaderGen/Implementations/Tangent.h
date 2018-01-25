#ifndef MATERIALX_TANGENT_H
#define MATERIALX_TANGENT_H

#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

/// Implementation of 'tangent' node for OgsFx
class TangentOgsFx : public SgImplementation
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
