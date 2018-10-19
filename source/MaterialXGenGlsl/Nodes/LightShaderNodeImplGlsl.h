#ifndef MATERIALX_LIGHTSHADERGLSL_H
#define MATERIALX_LIGHTSHADERGLSL_H

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNodeImpl.h>

namespace MaterialX
{

/// Implementation of light shaders for GLSL.
/// Used for all light shaders implemented in source code.
class LightShaderNodeImplGlsl : public SourceCodeNodeImpl
{
public:
    static ShaderNodeImplPtr create();

    const string& getLanguage() const override;
    const string& getTarget() const override;

    void initialize(ElementPtr implementation, ShaderGenerator& shadergen) override;

    void createVariables(const ShaderNode& node, ShaderGenerator& shadergen, Shader& shader) override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderGenerator& shadergen, Shader& shader) override;

protected:
    vector<Shader::Variable> _lightUniforms;
};

} // namespace MaterialX

#endif
