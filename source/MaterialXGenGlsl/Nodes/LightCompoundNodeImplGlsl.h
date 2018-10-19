#ifndef MATERIALX_LIGHTCOMPOUNDGLSL_H
#define MATERIALX_LIGHTCOMPOUNDGLSL_H

#include <MaterialXGenShader/Nodes/CompoundNodeImpl.h>
#include <MaterialXGenShader/Shader.h>

namespace MaterialX
{

/// Implementation of 'light' node for GLSL
class LightCompoundNodeImplGlsl : public CompoundNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionDefinition(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    vector<Shader::Variable> _lightUniforms;
};

} // namespace MaterialX

#endif
